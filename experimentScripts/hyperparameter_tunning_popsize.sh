#!/bin/bash
mkdir -p "/workspace/scratch/jobs/earza/slurm_logs"
source scripts/array_to_string_functions.sh


EXPERIMENT_RESULT_FOLDER_NAME="/workspace/scratch/jobs/earza/${PWD##*/}"

echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"


TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/hyperparameter_tunning/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/hyperparameter_tunning/controllers"
LOG_DIR="$EXPERIMENT_RESULT_FOLDER_NAME/logs"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=$EXPERIMENT_RESULT_FOLDER_NAME/"tmp"/$(dirname ${SCORE_PATH})

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME
mkdir -p $LOG_DIR
mkdir -p $TMP_RES_PATH

COMPILE_JOB_ID=`sbatch --parsable --exclude=n[001-004] --export=LOG_DIR=${LOG_DIR} scripts/make_hip.sh`

SRCDIR=`pwd`




NEAT_POPSIZE=1000
MAX_SOLVER_FE=400
MAX_TRAIN_ITERATIONS=2000
MAX_TRAIN_TIME=345600
FULL_MODEL="false"
DIM=20
INSTANCE_INDEX=1



# Train in one
TRAIN_JOB_ID=""
for SOLVER_POPSIZE in 4 8 16 32; do
    i=-1

    CONTROLLER_NAME_PREFIX_ARRAY=()
    SEED_ARRAY=()
    FULL_MODEL_ARRAY=()
    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=()
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=()
    for train_seed in 2 3 4 5 6 7 8 9 10 11; do
        i=$((i+1))


        CONTROLLER_NAME_PREFIX_ARRAY+=("continuous_popsize_${SOLVER_POPSIZE}_seed${train_seed}")
        SEED_ARRAY+=("${train_seed}")

        COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=("${INSTANCE_INDEX}")
        COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=("${DIM}")
        FULL_MODEL_ARRAY+=("${FULL_MODEL}")
    done
    CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
    SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
    FULL_MODEL_ARRAY=$(to_list "${FULL_MODEL_ARRAY[@]}")
    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY[@]}")
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY[@]}")
    TRAIN_JOB_ID="${TRAIN_JOB_ID}:`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=NEAT_POPSIZE=${NEAT_POPSIZE},SOLVER_POPSIZE=${SOLVER_POPSIZE},MAX_SOLVER_FE=${MAX_SOLVER_FE},MAX_TRAIN_ITERATIONS=${MAX_TRAIN_ITERATIONS},MAX_TRAIN_TIME=${MAX_TRAIN_TIME},SEED_ARRAY=${SEED_ARRAY},FULL_MODEL_ARRAY=${FULL_MODEL_ARRAY},COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY},COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY}, --array=0-$i src/experiments/real/scripts/hip_train_array_16_continuous.sl`"
done

echo "TRAIN_JOB_ID = ${TRAIN_JOB_ID}"








############  TEST ###########

COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000
TESTING_JOB_ID=""
for SOLVER_POPSIZE in 4 8 16 32; do
    i=-1
    CONTROLLER_ARRAY=()
    SEED_ARRAY=()
    PROBLEM_INDEX_ARRAY=()
    PROBLEM_DIM_ARRAY=()

    for train_seed in 2 3 4 5 6 7 8 9 10 11; do
        i=$((i+1))
        CONTROLLER_NAME_PREFIX="continuous_popsize_${SOLVER_POPSIZE}_seed${train_seed}"
        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        SEED_ARRAY+=("2") # the same seed for testing, the seed changes only for the controller name.
        PROBLEM_INDEX_ARRAY+=("${INSTANCE_INDEX}")
        PROBLEM_DIM_ARRAY+=("${DIM}")
    done

    CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
    SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
    PROBLEM_INDEX_ARRAY=$(to_list "${PROBLEM_INDEX_ARRAY[@]}")
    PROBLEM_DIM_ARRAY=$(to_list "${PROBLEM_DIM_ARRAY[@]}")

    TEST_JOB_ID="${TEST_JOB_ID}:`sbatch --parsable --dependency=afterok${TRAIN_JOB_ID} --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},SEED_ARRAY=${SEED_ARRAY},PROBLEM_INDEX_ARRAY=${PROBLEM_INDEX_ARRAY},PROBLEM_DIM_ARRAY=${PROBLEM_DIM_ARRAY},SOLVER_POPSIZE=${SOLVER_POPSIZE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},MAX_SOLVER_FE=${MAX_SOLVER_FE},FULL_MODEL=${FULL_MODEL},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR} --array=0-$i src/experiments/real/scripts/hip_test_array_in_one_of_16_problems.sl`"
done

echo "TEST_JOB_ID = ${TEST_JOB_ID}"


# TEST_JOB_ID=":TEST_JOB_ID1:TEST_JOB_ID2:TEST_JOB_ID3"
sbatch --dependency=afterok${TEST_JOB_ID} --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh



############ PERMUS #############

echo "Permus."
echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"

PROBLEM_TYPE="qap"
PROBLEM_PATH="$EXPERIMENT_RESULT_FOLDER_NAME/demo/instances/tai60a.qap"


TRAIN_JOB_ID=""
TEST_JOB_ID=""




for SOLVER_POPSIZE in 4 8 16 32; do
    i=-1
    PROBLEM_TYPE_ARRAY=()
    PROBLEM_PATH_ARRAY=()
    CONTROLLER_NAME_PREFIX_ARRAY=()
    SEED_ARRAY=()
    for train_seed in 2 3 4 5 6 7 8 9 10 11; do
        i=$((i+1))
        CONTROLLER_NAME_PREFIX="permus_popsize_${SOLVER_POPSIZE}_seed${train_seed}"
        
        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${PROBLEM_PATH}")
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
        SEED_ARRAY+=("${train_seed}")
    done
    PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
    COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
    CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
    SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
    TRAIN_JOB_ID="${TRAIN_JOB_ID}:`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY,SEED_ARRAY=$SEED_ARRAY,NEAT_POPSIZE=$NEAT_POPSIZE,MAX_SOLVER_FE=$MAX_SOLVER_FE,SOLVER_POPSIZE=$SOLVER_POPSIZE,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS,MAX_TRAIN_TIME=$MAX_TRAIN_TIME,EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY} --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array.sl`"
done



echo "TRAIN_JOB_ID = ${TRAIN_JOB_ID}"


############  TEST ###########

COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000

TESTING_JOB_ID=""
for SOLVER_POPSIZE in 4 8 16 32; do
    i=-1
    CONTROLLER_ARRAY=()
    PROBLEM_TYPE_ARRAY=()
    PROBLEM_PATH_ARRAY=()
    for train_seed in 2 3 4 5 6 7 8 9 10 11; do
        i=$((i+1))
        PROBLEM_PATH="demo/instances/tai60a.qap"
        CONTROLLER_NAME_PREFIX="permus_popsize_${SOLVER_POPSIZE}_seed${train_seed}"

        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH}")
    done
    CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
    PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
    PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
    TEST_JOB_ID=$TEST_JOB_ID:`sbatch --dependency=afterok${TRAIN_JOB_ID} --parsable --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_FE=${MAX_SOLVER_FE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`
done

echo "TEST_JOB_ID = ${TEST_JOB_ID}"

sbatch --dependency=afterok$TEST_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh


