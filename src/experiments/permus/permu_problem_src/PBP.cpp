/*
 *  PBP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#include "PBP.h"
#include "Tools.h"
#include "Individual.h"


PBP::PBP()
{
}

PBP::~PBP()
{
    delete[] _random_permu1;
    delete[] _random_permu2;
}

// This function needs to be called when the read procedure is called
void PBP::initialize_variables_PBP(int problem_size)
{   
    if (problem_size < 2)
    {
        std::cout << "error, problem size < 2. Instance file not read correctly.";
        exit(1);
    }
    
    _random_permu1 = new int[problem_size];
    GenerateRandomPermutation(_random_permu1, problem_size);
    _random_permu2 = new int[problem_size];
    GenerateRandomPermutation(_random_permu2, problem_size);
    problem_size_PBP = problem_size;
}

void PBP::apply_operator_with_fitness_update(CIndividual *indiv, int i, int j, int operator_id)
{   
    if(i==j && j==-1){
        return;
    }
    switch (operator_id)
    {
    case SWAP_OPERATOR:
        indiv->f_value = this->fitness_delta_swap(indiv, i, j) + indiv->f_value;
        Swap(indiv->genome, i, j);
        break;
    case INTERCHANGE_OPERATOR:
        indiv->f_value = this->fitness_delta_interchange(indiv, i, j) + indiv->f_value;
        Swap(indiv->genome, i, j);
        break;
    case INSERT_OPERATOR:
        indiv->f_value = this->fitness_delta_insert(indiv, i, j) + indiv->f_value;
        InsertAt(indiv->genome, i, j, this->problem_size_PBP);
        break;

    default:

        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }
    for (int i = 0; i < 4; i++)
    {
        indiv->is_local_optimum[i] = false;
    }
}


void PBP::local_search_iteration(CIndividual *indiv, int operator_id)
{
    if (indiv->is_local_optimum[operator_id])
    {
        return;
    }

    switch (operator_id)
    {
    case SWAP_OPERATOR:
        shuffle_vector(_random_permu1, problem_size_PBP);
        for (int i = 0; i < problem_size_PBP; i++)
        {
            int r = _random_permu1[i];
            if (r == problem_size_PBP - 1)
            {
                continue;
            }
            if (fitness_delta_swap(indiv, r, r+1) > 0)
            {
                apply_operator_with_fitness_update(indiv, r, r+1, operator_id);
                return;
            }
        }
        break;

    case INTERCHANGE_OPERATOR:
        shuffle_vector(_random_permu1, this->problem_size_PBP);
        shuffle_vector(_random_permu2, this->problem_size_PBP);
        for (int i = 0; i < this->problem_size_PBP; i++)
        {
            for (int j = 0; j < this->problem_size_PBP; j++)
            {
                if (i < j)
                {
                    if (fitness_delta_interchange(indiv, _random_permu1[i], _random_permu2[j]) > 0)
                    {
                        apply_operator_with_fitness_update(indiv, _random_permu1[i], _random_permu2[j], operator_id);
                        return;
                    }
                }
            }
        }
        break;

    case INSERT_OPERATOR:
        shuffle_vector(_random_permu1, this->problem_size_PBP);
        shuffle_vector(_random_permu2, this->problem_size_PBP);
        for (int i = 0; i < this->problem_size_PBP; i++)
        {
            for (int j = 0; j < this->problem_size_PBP; j++)
            {
                if (i != j || _random_permu1[j]==_random_permu1[(i-1)])
                {
                    if (fitness_delta_insert(indiv, _random_permu1[i], _random_permu2[j]) > 0)
                    {
                        apply_operator_with_fitness_update(indiv, _random_permu1[i], _random_permu2[j], operator_id);
                        return;
                    }
                }
            }
        }
        break;

    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }

    indiv->is_local_optimum[operator_id] = true;

}

// obtain item at pos idx assuming operator operator_id where applied.
int PBP::item_i_after_operator(int *permu, int idx, int operator_id, int i, int j){
    switch (operator_id)
    {
    case SWAP_OPERATOR:
        if (idx != i && idx != j)
        {
            return permu[idx];
        }else if(idx == i)
        {
            return permu[j];
        }else
        {
            return permu[i];
        }
        std::cout <<  "idx not valid";
        exit(1);
        break;
    case INTERCHANGE_OPERATOR:
        if (idx != i && idx != j)
        {
            return permu[idx];
        }else if(idx == i)
        {
            return permu[j];
        }else
        {
            return permu[i];
        }
        std::cout << "idx not valid";
        exit(1);
        break;

    case INSERT_OPERATOR:
        if(i < j){
            if (idx == j)
            {
                return permu[i];
            }else if(idx >= i && idx < j){
                return permu[idx + 1];
            }else
            {
                return permu[idx];
            }
        }else if( i > j){
            if (idx == j)
            {
                return permu[i];
            }else if(idx >= j+1 && idx < i+1){
                return permu[idx - 1];
            }else
            {
                return permu[idx];
            }
        }else{ // i == j
            return permu[idx];
        }
        std::cout << "error in function item_i_after_operator, return not reached";
        exit(1);
        break;

    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }
}


void PBP::obtain_indexes_step_towards(int *permu, int *ref_permu, int* i, int* j, int operator_id)
{
    switch (operator_id)
    {
    case SWAP_OPERATOR:  // SCHIZVINOTTO 2007
    {
        shuffle_vector(_random_permu1, problem_size_PBP);
        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu1[idx];
            if (r < problem_size_PBP)
            {
                if (
                    (permu[r] < permu[r+1] && ref_permu[r] > ref_permu[r+1]) || 
                    (permu[r] > permu[r+1] && ref_permu[r] < ref_permu[r+1])
                    )
                {
                    *i = r;
                    *j = r+1;
                    return;
                }
            }
        }
        *i = -1;
        *j = -1;
        return;

        break;
    }
    case INTERCHANGE_OPERATOR:  // SCHIZVINOTTO 2007
    {
        for (int idx = 0; idx < problem_size_PBP; idx++) // compute ref_permu \circ permu^-1
        {   
            _random_permu1[idx] = Find(permu, problem_size_PBP, ref_permu[idx]);
        }
        shuffle_vector(_random_permu2, problem_size_PBP);
        
        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu2[idx];
            if (_random_permu1[r] != r){
                *i = r;
                *j = _random_permu1[r];
                return;
            }
        }
        
        // permu == ref_permu

        *i = -1;
        *j = -1;
        return;

    }
    case INSERT_OPERATOR:
        // get indices to insert considering the item that makes the biggest "jump"    
    {
        for (int idx = 0; idx < problem_size_PBP; idx++) // compute ref_permu \circ permu^-1
        {   
            _random_permu1[idx] = Find(permu, problem_size_PBP, ref_permu[idx]);
        }
        shuffle_vector(_random_permu2, problem_size_PBP);
        
        int max = 0;
        int arg_max = 0;

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute ref_permu \circ permu^-1
        {   
            int r = _random_permu2[idx];
            if (abs(_random_permu1[r] - r) > max)
            {
                max = abs(_random_permu1[r] - r);
                arg_max = r;
            }
        }
        if (max == 0)
        {
            *i = -1;
            *j = -1;
        }else{
            *i = arg_max;
            *j = _random_permu1[*i];
        }
        break;
    }
    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }

}


void PBP::move_indiv_towards_reference(CIndividual* indiv, int* ref_permu, int operator_id){
    int i,j;
    obtain_indexes_step_towards(indiv->genome, ref_permu, &i, &j, operator_id);
    // std::cout << "(" << i << "," << j << ")" << endl;
    apply_operator_with_fitness_update(indiv, i, j, operator_id);
}
