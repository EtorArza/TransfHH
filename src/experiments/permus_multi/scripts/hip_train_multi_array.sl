#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_%x_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_%x_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=16G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=5-0:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004]



echo "Train ${SLURM_ARRAY_TASK_ID} start."


mkdir ${SCRATCH_JOB} -p

source scripts/array_to_string_functions.sh

SRCDIR=`pwd`

list_to_array $PROBLEM_TYPE_ARRAY
PROBLEM_TYPE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_TYPE=${PROBLEM_TYPE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $CONTROLLER_NAME_PREFIX_ARRAY
CONTROLLER_NAME_PREFIX_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER_NAME_PREFIX=${CONTROLLER_NAME_PREFIX_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $SEED_ARRAY
SEED_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SEED=${SEED_ARRAY[$SLURM_ARRAY_TASK_ID]}




replaced_with=","
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY//"s_e_p"/$replaced_with}
list_to_array $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[$SLURM_ARRAY_TASK_ID]}







echo -n "COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS: " 
echo $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS 

echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID


list_to_array_with_comma $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS
ARRAY_OF_INSTANCES=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")


instance_path=`dirname ${ARRAY_OF_INSTANCES[0]}`






echo -n "instance_path: " 
echo $instance_path 

mkdir -p "$SCRATCH_JOB/$instance_path"

for PROBLEM_PATH in "${ARRAY_OF_INSTANCES[@]}"
do
    cp $PROBLEM_PATH "$SCRATCH_JOB/$instance_path" -v
done 



cp main.out -v $SCRATCH_JOB 

cd $SCRATCH_JOB

echo "pwd: `pwd`"


cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = permu_multi


MAX_TRAIN_ITERATIONS = $MAX_TRAIN_ITERATIONS
MAX_TRAIN_TIME = $MAX_TRAIN_TIME
POPSIZE = $NEAT_POPSIZE
THREADS = $SLURM_CPUS_PER_TASK

SEARCH_TYPE = phased
SEED = $SEED

CONTROLLER_NAME_PREFIX = $CONTROLLER_NAME_PREFIX
EXPERIMENT_FOLDER_NAME = $EXPERIMENT_CONTROLLER_FOLDER_NAME

MAX_SOLVER_FE = $MAX_SOLVER_FE
SOLVER_POPSIZE = $SOLVER_POPSIZE


PROBLEM_TYPE = $PROBLEM_TYPE
PROBLEM_PATH = dummy_path
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS = $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS

EOF

echo "---conf file begin---" 

cat tmp.ini

echo "---conf file end---"


date
srun main.out "tmp.ini"
date

rm main.out
cd ..
rm ${SCRATCH_JOB} -r