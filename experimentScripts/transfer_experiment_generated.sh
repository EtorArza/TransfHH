#!/bin/bash

source scripts/array_to_string_functions.sh


mkdir -p "/workspace/scratch/jobs/earza/slurm_logs"
source scripts/array_to_string_functions.sh


EXPERIMENT_RESULT_FOLDER_NAME="/workspace/scratch/jobs/earza/${PWD##*/}"

echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"

TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_generated_continuous/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_generated_continuous/controllers"
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
SOLVER_POPSIZE=8
MAX_SOLVER_FE=400
MAX_TRAIN_ITERATIONS=2000
MAX_TRAIN_TIME=345600
FULL_MODEL="false"
DIM=20





CONTROLLER_NAME_PREFIX_ARRAY=()
SEED_ARRAY=()
NLO_ARRAY=()


i=-1
for train_seed in 2 3 4 5 6 7 8 9 10 11; do
for NLO_train in 1 2 4 8 16 32 64; do
    i=$((i+1))

    CONTROLLER_NAME_PREFIX_ARRAY+=("NLO_${NLO_train}_seed${train_seed}")
    SEED_ARRAY+=("${train_seed}")
    NLO_ARRAY+=("${NLO_train}")
done
done



CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
NLO_ARRAY=$(to_list "${NLO_ARRAY[@]}")


# launch training and get JOB_ID to queue test to launch only after training is complete
TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=MAX_TRAIN_TIME=$MAX_TRAIN_TIME,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS,NEAT_POPSIZE=$NEAT_POPSIZE,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_CONTROLLER_FOLDER_NAME=$EXPERIMENT_CONTROLLER_FOLDER_NAME,SEED_ARRAY=$SEED_ARRAY,NLO_ARRAY=$NLO_ARRAY,DIM=$DIM,SOLVER_POPSIZE=$SOLVER_POPSIZE,MAX_SOLVER_FE=$MAX_SOLVER_FE,LOG_DIR=$LOG_DIR,FULL_MODEL=$FULL_MODEL --array=0-$i src/experiments/real/scripts/hip_train_array_random_problem.sl`


COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000
TESTING_JOB_ID=""


CONTROLLER_ARRAY=()
SEED_ARRAY=()
NLO_ARRAY=()
i=-1
for train_seed in 2 3 4 5 6 7 8 9 10 11; do
for NLO_train in 1 2 4 8 16 32 64; do
    for NLO_test in 1 2 4 8 16 32 64; do
        i=$((i+1))
        CONTROLLER_NAME_PREFIX="NLO_${NLO_train}_seed${train_seed}"
        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        SEED_ARRAY+=("2")
        NLO_ARRAY+=("${NLO_test}")
    done
done
done

CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
NLO_ARRAY=$(to_list "${NLO_ARRAY[@]}")

TESTING_JOB_ID=`sbatch --parsable --dependency=afterok:${TRAINING_JOB_ID} --export=COMPUTE_RESPONSE=${COMPUTE_RESPONSE},SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},CONTROLLER_ARRAY=$CONTROLLER_ARRAY,SEED_ARRAY=$SEED_ARRAY,NLO_ARRAY=$NLO_ARRAY,DIM=$DIM,SOLVER_POPSIZE=$SOLVER_POPSIZE,MAX_SOLVER_FE=$MAX_SOLVER_FE,FULL_MODEL=$FULL_MODEL --array=0-$i src/experiments/real/scripts/hip_test_array_randomly_generated.sl`


sbatch --dependency=afterok:$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh



