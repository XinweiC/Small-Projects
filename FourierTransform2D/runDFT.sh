#!/bin/tcsh
#PBS -q class
#PBS -l nodes=4:sixcore
#PBS -l walltime=00:50:00
#PBS -N xinwei
# The below chnages the working directory to the location of
# your testmpi program
cd /nethome/xchen318/FourierTransform2D
# The below tells MPI to run your jobs on 16 processors
/usr/lib64/openmpi/bin/mpirun -np 16 --hostfile $PBS_NODEFILE ./fft2d


