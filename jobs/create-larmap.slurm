#!/bin/bash
#
# Author: Luigi Pertoldi - pertoldi@pd.infn.it
# Created: Sun 24 Mar 2019
#
# USAGE: qsub create-larmap.slurm <runid>
#
#$ -o /dev/null
#$ -e /dev/null

../bin/create-larmap ../settings/$2 \
    "../gen/jobs/out/$1/phmap-*.root" \
    out/gerda-larmap-$1.root &> log/create-larmap-$1.log
