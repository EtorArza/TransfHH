#include "std.h"
#include "evaluatorexperiment.h"
#include "genomemanager.h"
#include "permuevaluator.h" 
#include "neat.h"
#include "rng.h"
 
using namespace NEAT;

static struct PermuInit {
    PermuInit() {
        auto create_evaluator = [] () {
            return create_permu_evaluator();
        };
 
        auto create_seeds = [] (rng_t rng_exp) {
            return env->genome_manager->create_seed_generation(env->pop_size,
                                                        rng_exp,
                                                        1,
                                                        __sensor_N,
                                                        __output_N,
                                                        __sensor_N);
        };

        //todo: This is wonky. Should maybe make an explicit static registry func?
        new EvaluatorExperiment("permu", create_evaluator, create_seeds);
    }
} init;
