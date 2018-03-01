#!/bin/tcsh
#PBS -q class
#PBS -l nodes=1:sixcore
#PBS -l walltime=00:10:00
#PBS -N xinwei
# The below chnages the working directory to the location of
# your testmpi program
cd /nethome/xchen318/ThreadsTransform2D
# The below tells MPI to run your jobs on 16 processors
./threadDFT2d


