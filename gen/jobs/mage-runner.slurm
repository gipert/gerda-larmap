#!/bin/bash
#
# Author: Luigi Pertoldi - pertoldi@pd.infn.it
# Created: Sun 24 Mar 2019
#
# USAGE: qsub mage-runner.slurm <mage-macro-file>
#
#$ -o /dev/null
#$ -e /dev/null

dirn=`dirname $1`
fname=`basename $1 .mac`.log

/usr/bin/time -v MaGe --no-colors $1 &> $dirn/$fname
