/*
 *  PBP.h
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */
#ifndef __PBP_H__
#define __PBP_H__

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "Individual.h"

#define SWAP_OPERATOR 1
#define INTERCHANGE_OPERATOR 2
#define INSERT_OPERATOR 3


using namespace std;
using std::string;
using std::stringstream;

class PBP
{
  public:
	PBP();

	/*
	 * Virtual functions to be defined with each permutation problem.
	 */
	virtual ~PBP();
	virtual int Read(string filename) = 0;
	virtual int GetProblemSize() = 0;
	virtual void Evaluate(CIndividual *indiv) = 0; // update the f_value of the individuals.


	/*
	 * Functions that are valid for all permutation problems. 
	 * The use of this functions requires the fitness value of the individual to be previously computed.
	 */
	void apply_operator_with_fitness_update(CIndividual *indiv, int i, int j, int operator_id);
	void local_search_iteration(CIndividual *indiv, int operator_id);
	void move_indiv_towards_reference(CIndividual* indiv, int* ref_permu, int operator_id);


  protected:
	// This function needs to be executed on problem read.
	int item_i_after_operator(int *permu, int idx, int operator_id, int i, int j);
	void initialize_variables_PBP(int problem_size);
	int *_random_permu1;
	int *_random_permu2;

	// The f_value of the individuals does not change in this functions.
	virtual double fitness_delta_swap(CIndividual *indiv, int i, int j) = 0;
	virtual double fitness_delta_interchange(CIndividual *indiv, int i, int j) = 0;
	virtual double fitness_delta_insert(CIndividual *indiv, int i, int j) = 0;

	
  private:
	void obtain_indexes_step_towards(int *permu, int *ref_permu, int* i, int* j, int operator_id);


	int problem_size_PBP;
};
#endif
