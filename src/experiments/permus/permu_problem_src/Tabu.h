#pragma once

class RandomNumberGenerator;

namespace PERMU{


class Tabu
{
public:

	Tabu(RandomNumberGenerator* rng, int n);
	~Tabu();

	void set_tabu(int i, int j);
	bool is_tabu(int i, int j);
	void reset();
	void increase_tabu_size();
	void decrease_tabu_size();
	void resize(int new_size);
	double return_current_relative_tabu_size();

	double tabu_coef_neat = 0.0;

private:
	int MAX_TABU_LENGTH;
	int tabu_length;
	RandomNumberGenerator* rng;
	static int n_indivs_created;
	int* tabu_indices_i;
	int* tabu_indices_j;
	bool** tabu_table;
	int index_pos = 0;
	int n;
	int next_index_pos();


};


}