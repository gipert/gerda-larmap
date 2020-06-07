#!/bin/bash
#
# Author: Luigi Pertoldi - pertoldi@pd.infn.it
# Created: Sun 24 Mar 2019

dryrun=false
sim_dir="sim"

print_log() {
    case "$1" in
        "info")
            shift
            \echo -e "\033[97;1mINFO:\033[0m $@"
            ;;
        "warn")
            shift
            >&2 \echo -e "\033[33mWARNING:\033[0m $@"
            ;;
        "err")
            shift
            >&2 \echo -e "\033[91mERROR:\033[0m $@"
            ;;
        *)
            print_log err "unknown logging level $1"
    esac
}

which_manager() {

    if `which qsub &> /dev/null`; then
        \echo "qsub"
    elif `which sbatch &> /dev/null`; then
        \echo "slurm"
        # add your cluster manager here...
    else
        if $dryrun; then
            print_log warn "No supported cluster manager found"
            \echo "none"
        else
            print_log err "No supported cluster manager found, aborting..."
            exit 1
        fi
    fi
}

manager=$(which_manager)

fill_template_macro() {

    local template_file="$1"
    local rootfile="$2"
    \sed "s|\${OUTNAME}|$rootfile|g" "$template_file"
}

submit_mage_job() {

    local job_name="$1"; shift

    \cd "$sim_dir"

    if is_job_running "$sim_id"; then
        print_log warn "'$sim_id' jobs look already running, won't submit"
    else
        if [[ $manager == "qsub" ]]; then
            \qsub -P short -N "$job_name" ../MaGe.qsub "$@"
        elif [[ $manager == "slurm" ]]; then
            \sbatch -J "$job_name" ../MaGe.qsub "$@"
        # add your cluster manager here...
        fi
    fi

    \cd - > /dev/null
}

submit_mage_job_array() {

    local job_name="$1";
    local start_idx=$2;
    local stop_idx=$3;
    shift 3

    \cd "$sim_dir"

    if [[ $manager == "qsub" ]]; then
        if $dryrun; then
            print_log info "qsub -P short -N $job_name -t ${start_idx}-${stop_idx} ../MaGe.qsub $@"
        else
            \qsub -P short -N "$job_name" -t ${start_idx}-${stop_idx} ../MaGe.qsub "$@"
        fi
    # add your cluster manager here...
    fi

    \cd - > /dev/null
}

is_job_running() {

    # cache job list
    if [[ "$joblist" == "" ]]; then
        print_log info "getting list of running jobs"
        if [[ $manager == "qsub" ]]; then
            joblist=`\qstat -r | grep 'Full jobname:'`
            # add your cluster manager here...
        fi
    fi

    local job_name="$1";
    echo "$joblist" | grep "$job_name" > /dev/null
    [[ $? == 1 ]] && return 1 || return 0
}

submit_mage_runid_jobs() {

    local sim_id="$1"
    local start_idx=$2
    local stop_idx=$3
    local job_name="${sim_id}.${start_idx}-${stop_idx}"

    local do_rerun=false
    for i in `seq -f "%05g" $start_idx $stop_idx`; do
        if [[ ! -f "$sim_dir/$sim_id/output/${sim_id}-${i}.root" ]]; then
            do_rerun=true
        fi
    done

    if $do_rerun; then
        print_log info "submitting '$job_name' jobs"

        if is_job_running "$job_name"; then
            print_log warn "'$job_name' jobs look already running, won't submit"
        else
            submit_mage_job_array "$job_name" $start_idx $stop_idx "$sim_id/macros/$sim_id"
        fi
    else
        print_log warn "'$job_name' jobs look up to date, won't submit"
        return
    fi
}

process_simulation_run() {

    joblist=""

    $dryrun && print_log warn "running in dry-run mode, no jobs will be actually sent"

    local template="$1"
    local copy_num=$(printf "%05d" $2)
    local n_macros=$3
    local start_id=${4:-1}

    if [[ ! -f "../${template}.mac.template" ]]; then
        print_log err "../${template}.mac.template: no such file or directory"
        return
    fi

    local name_id="${template}-${copy_num}"
    print_log info "creating '$name_id' macros"

    \mkdir -p "$sim_dir/$name_id"/{macros,output}

    for i in `seq -f "%05g" $start_id $(expr $start_id + $n_macros - 1)`; do
        fill_template_macro "../${template}.mac.template" "${name_id}/output/${name_id}-${i}.root" \
            > "$sim_dir/${name_id}/macros/${name_id}-${i}.mac"
    done

    submit_mage_runid_jobs $name_id $start_id $(expr $start_id + $n_macros - 1)
}
