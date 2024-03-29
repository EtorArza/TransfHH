#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=4 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short
#SBATCH --exclude=n[001-004,017-018]

SCRATCH_JOB="${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}_compile"
mkdir ${SCRATCH_JOB} -p


SRC_DIR=`pwd`

cat > Makefile.conf <<EOF
DEVMODE=false
CFLAGS= -std=c++11 -pthread -Wall
PARALELLIZE=true

HIPATIA=true
PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

cp -r -v src ${SCRATCH_JOB}/ 
cp -v Makefile ${SCRATCH_JOB}/ 
cp -v Makefile.conf ${SCRATCH_JOB}/ 


cd ${SCRATCH_JOB}

echo "ls: " 
ls 

make -j ${SLURM_CPUS_PER_TASK}

cp -v main.out ${SRC_DIR}