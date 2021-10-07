#include "Tools.h"
#include <math.h>
#include <assert.h>
#include "MultidimBenchmarkFF.h"
#include "generator.h"
#include <omp.h>
#include <unistd.h>
MultidimBenchmarkFF::MultidimBenchmarkFF(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    this->dim = dim;
    rng = new RandomNumberGenerator();
    temp_vect_1 = new double[this->dim];
    this->x_lower_lim = x_lower_lim;
    this->x_upper_lim = x_upper_lim;

    if (ROTATE)
    {
        this->getRandomRotationMatrix(SEED);
        this->rotate = true;
    }
    else
    {
        this->rotate = false;
    }
    if (x_lower_lim >= x_upper_lim)
    {
        cout << "\n Error in MultidimBenchmarkFF, x_lims = (" << x_lower_lim << ", " << x_upper_lim << ") not correct.\n" <<endl;
        exit(1);
    }




}


MultidimBenchmarkFF::~MultidimBenchmarkFF()
{
    if (R != nullptr)
    {
        delete_matrix(R, this->dim);
        R = nullptr;
    }
    
    if (!this->rng_deleted)
    {
        delete this->rng;
        this->rng_deleted = true;
    }
    delete[] temp_vect_1;
}






void MultidimBenchmarkFF::load_rng(RandomNumberGenerator *rng)
{
    if (!this->rng_deleted)
    {
        delete this->rng;
        this->rng_deleted = true;
    }
    this->rng = rng;
}

int MultidimBenchmarkFF::GetProblemSize() { return this->dim; }


double MultidimBenchmarkFF::Fitness_Func_0_1(double* x_vec_0_1){

    double local_tmp_vec[this->dim]; 

    if (rotate)
    {
        rotate_x_given_R(local_tmp_vec, x_vec_0_1);
    }else
    {
        copy_array(local_tmp_vec, x_vec_0_1, this->dim);
    }
    



    for (int i = 0; i < this->dim; i++)
    {   
        // assert(x_vec_0_1[i] > -0.0000001);
        // assert(x_vec_0_1[i] < 1.0000001);
        local_tmp_vec[i] = this->get_x_lim_lower() + min(1.0,max(0.0,x_vec_0_1[i])) * (this->get_x_lim_upper() - this->get_x_lim_lower());
    }







    
    // Maximization is assumed in MultidimBenchmarkFF problems.
    return this->FitnessFunc(local_tmp_vec); 
}



double MultidimBenchmarkFF::get_x_lim_lower()
{
    return this->x_lower_lim;
}


double MultidimBenchmarkFF::get_x_lim_upper()
{
    return this->x_upper_lim;
}


void MultidimBenchmarkFF::Evaluate(CIndividual *indiv)
{
	double fitness = 0;
	fitness = Fitness_Func_0_1(indiv->genome);
	indiv->f_value = fitness;
    indiv->bk_was_improved = false;
    if (fitness >indiv->f_best)
    {
        indiv->bk_was_improved = true;
        indiv->f_best = fitness;
        copy_array(indiv->genome_best, indiv->genome, indiv->n);
    }
}



// Sphere (bowl-shaped) [-5.12,5.12]
double F1::FitnessFunc(double* x_vec){
    double res = 0;
    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i];
    }
    return -res;
}


// ROTATED HYPER-ELLIPSOID (bowl-shaped) [-65.536, 65.536]
double F2::FitnessFunc(double* x_vec){
    double res = 0;
    double sum_of_x_j = 0;
    for (int i = 0; i < dim; i++)
    {   
        sum_of_x_j += x_vec[i] * x_vec[i];
        res += x_vec[i] * x_vec[i];
    }
    return -res;
}


// TRID FUNCTION (bowl-shaped) [-4, 4]
double F3::FitnessFunc(double* x_vec){
    double res = 0;
    double sum_of_x_j = 0;

	
    double sum1 = 0;
    double sum2 = 0;
 
    for (int i = 0; i < dim; i++)
    {   
        sum1 += pow(x_vec[i]-1,2);
    }

    for (int i = 1; i < dim; i++)
    {   
        sum2 += x_vec[i]*x_vec[i-1];
    }
    res = sum1 - sum2;
    return -2 - res ; // make sure it is negative
}


// PERM FUNCTION 0, D, BETA (bowl-shaped) [-2, 2]
double F4::FitnessFunc(double* x_vec){
    double res = 0;
    double sum_of_x_j = 0;

	double b = 10;
    double outer = 0;

    for (int i=1; i<= dim;i++)
    {
       double inner = 0;
        for (int j=1; j<= dim;j++)
        {
            double xj = x_vec[j - 1];
            inner = inner + (j+b)*(pow(xj,i)-pow(1/j,i));
        }
        outer = outer + pow(inner,2);
    }
    res = outer;
    return -res;    
}
 


// Dixon & price (valley-shaped) [-10,10]
double F5::FitnessFunc(double* x_vec){
    double res = (x_vec[0] - 1.0) * (x_vec[0] - 1.0);
    for (int i = 2; i <= dim; i++)
    {
        res += (double) i * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]) * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]);
    }
    return -  res;
}


// Rosenbrock (valley-shaped) [-5, 10]
double F6::FitnessFunc(double* x_vec){
    double res = 0.0;

    for (int i = 0; i < dim-1; i++)
    {
        res += 100.0 * 
        (x_vec[i]*x_vec[i] - x_vec[i+1]) * 
        (x_vec[i]*x_vec[i] - x_vec[i+1]) + 
        (x_vec[i] - 1.0) * 
        (x_vec[i] - 1.0);
    }

    return -res;
}


// six hump cammel funcion (valley-shaped) [-3,3]
double F7::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    if(dim != 2)
    {
        cout << "\nERROR: cammel function is only available when dim = 2." << endl;
        exit(1);
    }

    double x1 = x_vec[0];
    double x2 = x_vec[1];
	
  res += (4-2.1*pow(x1,2)+pow(x1,4)/3) * pow(x1,2);
  res+= x1*x2;
  res+= (-4+4*pow(x2,2)) * pow(x2,2);
	

   return -1.0317 - res;
}


// three hump cammel funcion (valley-shaped) [-5,5]
double F8::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    if(dim != 2)
    {
        cout << "\nERROR: cammel function is only available when dim = 2." << endl;
        exit(1);
    }

    double x1 = x_vec[0];
    double x2 = x_vec[1];
	
    res+= 2*pow(x1,2);
    res+= -1.05*pow(x1,4);
    res+= pow(x1,6) / 6;
    res+= x1*x2;
    res+= pow(x2,2);
	

   return - res;
}




// Zakharov (plate-shaped) [-5, 10]
double F9::FitnessFunc(double* x_vec){
    double res = 0;
    double val_1 = 0.0;
    double val_2 = 0.0;
    
    for (int i = 0; i < dim; i++)
    {
        val_1 += x_vec[i] * x_vec[i];
    }
    
    for (int i = 1; i <= dim; i++)
    {
        val_2 += (0.5 * i * x_vec[i-1]); 
    }

    res = val_1 + (val_2 * val_2) + (val_2 * val_2 * val_2 * val_2);
    return -res;
}




// booth function (plate-shaped) [-10, 10]
double F10::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    if(dim != 2)
    {
        cout << "\nERROR: cammel function is only available when dim = 2." << endl;
        exit(1);
    }

    double x1 = x_vec[0];
    double x2 = x_vec[1];
	


    res += pow(x1 + 2*x2 - 7,2);
    res += pow(2*x1 + x2 - 5,2);


   return -res;
}


// matyas function (plate-shaped) [-10, 10]
double F11::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    if(dim != 2)
    {
        cout << "\nERROR: cammel function is only available when dim = 2." << endl;
        exit(1);
    }

    double x1 = x_vec[0];
    double x2 = x_vec[1];
	
    res = 0.26*(x1*x1 + x2*x2) - 0.48*x1*x2;

   return -res;
}


// MCCORMICK function (plate-shaped) [-3, 4]
double F12::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    if(dim != 2)
    {
        cout << "\nERROR: cammel function is only available when dim = 2." << endl;
        exit(1);
    }

    double x1 = x_vec[0];
    double x2 = x_vec[1];
	
    res += sin(x1+x2);
    res += pow(x1 -x2,2);
    res += -1.5*x1 +2.5*x2 +1;
    return -1.9134 - res;
}







// Rastrigin (many local optima) [-5.12, 5.12]
double F13::FitnessFunc(double* x_vec){
    double res = (double) 10 * dim;

    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i] - 10.0 * cos(2.0 * M_PI * x_vec[i]);
    }

    return -res;
}


// Levy (many local optima) [-10, 10]
double F14::FitnessFunc(double* x_vec){
    double w_1 = 1.0 + (x_vec[0] - 1.0)/4.0;
    double res = sin(M_PI * w_1) * sin(M_PI * w_1);

    for (int i = 0; i < dim - 1; i++)
    {
        double w_i = 1.0 + (x_vec[i] - 1.0)/4.0;
        res += (w_i - 1) *(w_i - 1) * (1.0 + 10.0*sin(M_PI * w_i + 1.0)*sin(M_PI * w_i + 1.0));
    }

    double w_d = 1.0 + (x_vec[dim - 1] - 1.0)/4.0;

    res += (w_d - 1.0) * (w_d - 1.0) * (1.0 + pow(sin(2.0 * M_PI * w_d), 2.0));

    return -res;
}



// Griewank (many local optima) [-600,600]
double F15::FitnessFunc(double* x_vec){
    double val_1 = 0.0;
    for (int i = 0; i < dim; i++)
    {
        val_1 += x_vec[i] * x_vec[i] / 4000.0;
    }

    double val_2 = 1.0;

    for (int i = 1; i <= dim; i++)
    {
        val_2 *= cos(x_vec[i-1] / sqrt((double) i));
    }

    double res = 1.0 + val_1 - val_2;

    return -res;
}




// Ackley (many local optima) [-32.768, 32.768]
double F16::FitnessFunc(double* x_vec){
    double res = 0;
    double sum_1 = 0.0;
    double sum_2 = 0.0;

    for (int i = 0; i < dim; i++)
    {
        sum_1 += x_vec[i] * x_vec[i];
    }

    sum_1 = -20 * exp(-0.2 * sqrt(sum_1 / (double) dim));

    for (int i = 0; i < dim; i++)
    {
        sum_2 += cos(2.0 * M_PI * x_vec[i]);
    }

    sum_2 = -exp(sum_2 / (double) dim);

    res = 20.0 + exp(1) + sum_1 + sum_2;
    return -res;
}



static int current_jobs_with_this_seed = 0;
FRandomlyGenerated::FRandomlyGenerated(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)  : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE)
{
    char config_path[] = "src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat";


    bool repeat = true;
    while (repeat)
    {
        #pragma omp critical(adshfafoiadfoak)
        {
            //cout << "(" << seed << "," << dim;
            static int current_dim = -1;
            static int current_seed = -1;
            if (current_jobs_with_this_seed == 0)
            {
                g_seeded_initialize(config_path, SEED, dim);
                current_seed = SEED;
                current_dim = dim;
                current_jobs_with_this_seed++;
                repeat = false;
            }
            else if(current_seed == SEED && current_dim == dim)
            {
                current_jobs_with_this_seed++;
                repeat = false;
            }
            else
            {
                //cout << "waiting";
                repeat = true;
            }
            //cout << endl;
        }
        if (repeat == true)
        {
            usleep((useconds_t) 1000 );
        }
        
    }

};


FRandomlyGenerated::~FRandomlyGenerated()
{
    #pragma omp critical(adshfafoiadfoak)
    {
        //cout << current_seed << "," << dim << ")" << endl;
        current_jobs_with_this_seed--;
    }
};


double FRandomlyGenerated::FitnessFunc(double* x_vec)
{
    return g_calculate(x_vec);
}





void MultidimBenchmarkFF::getRandomRotationMatrix(int SEED)
{

    if (this->R==nullptr)
    {
        zero_initialize_matrix(R, this->dim, this->dim);
    }


  double scalar=0, line[this->dim], norm=0;
  int i,j,k;
  RandomNumberGenerator local_rng = RandomNumberGenerator();
  local_rng.seed(SEED);
  
  for (i=0;i<this->dim;i++){		
    /*random values for matrix R:s i:th row, first step*/
    for (j=0;j<this->dim;j++)
      R[i][j]=local_rng.random_0_1_double();		 
    
    /*second step*/
    /*if j<=i, the line must be 0*/
    for(k=0;k<this->dim;k++)		
      line[k]=0.0;
    
    for(j=0;j<i;j++){
      scalar=0;/*scalar product*/
      for(k=0;k<this->dim;k++)				
	scalar=scalar+R[i][k]*R[j][k];			
      for(k=0;k<this->dim;k++)			
	line[k]=line[k]+scalar*R[j][k];			
    }
    for(k=0;k<this->dim;k++)		
      R[i][k]=R[i][k]-line[k];
    
    /*third step*/
    norm=0;
    for(k=0;k<this->dim;k++)			
      norm=norm+R[i][k]*R[i][k];
    
    norm=sqrt(norm);
    for(k=0;k<this->dim;k++)		
      R[i][k]=R[i][k]/norm;					
  }
//   /*We generate the transpose of R*/
//   for(i=0;i<this->dim;i++) 
//     for(j=0;j<this->dim;j++)
//       RT[i][j]=R[j][i]; 
}

void MultidimBenchmarkFF::rotate_x_given_R(double *res_x_rotated, double *x)
{
    int i, j;
    const int const_dim = this->dim;

    if (x == res_x_rotated) // create temporal array, if result and x are the same
    {
        double tmp[const_dim];
        for (i = 0; i < const_dim; i++)
        {
            tmp[i] = 0;
            for (j = 0; j < const_dim; j++)
                tmp[i] = tmp[i] + R[i][j] * x[j];
        }
        for (i = 0; i < const_dim; i++)
            x[i] = tmp[i];
    }
    else
    {
        for (i = 0; i < const_dim; i++)
        {
            res_x_rotated[i] = 0;
        }
        for (i = 0; i < const_dim; i++)
        {
            for (j = 0; j < const_dim; j++)
                res_x_rotated[i] += R[i][j] * x[j];
        }
    }
}





MultidimBenchmarkFF* load_problem_with_default_lims(int problem_index, int dim,  int SEED, bool ROTATE)
{

    // Sphere (bowl-shaped) [-5.12,5.12]
    // ROTATED HYPER-ELLIPSOID (bowl-shaped) [-65.536, 65.536]
    // TRID FUNCTION (bowl-shaped) [-4.0, 4.0]
    // PERM FUNCTION 0, D, BETA (bowl-shaped) [-2.0, 2.0]
    // Dixon & price (valley-shaped) [-10.0,10.0]
    // Rosenbrock (valley-shaped) [-5.0, 10.0]
    // six hump cammel funcion (valley-shaped) [-3.0,3.0]
    // three hump cammel funcion (valley-shaped) [-5.0,5.0]
    // Zakharov (plate-shaped) [-5.0, 10.0]
    // booth function (plate-shaped) [-10.0, 10.0]
    // matyas function (plate-shaped) [-10.0, 10.0]
    // MCCORMICK function (plate-shaped) [-3.0, 4.0]
    // Rastrigin (many local optima) [-5.12, 5.12]
    // Levy (many local optima) [-10.0, 10.0]
    // Griewank (many local optima) [-600.0,600.0]
    // Ackley (many local optima) [-32.768, 32.768]
    double x_lower_lim;
    double x_upper_lim;

    switch (problem_index)
    {
    case 0:
        x_lower_lim = 0;
        x_upper_lim = 1;
        break;
    case 1:
        x_lower_lim = -5.12;
        x_upper_lim = 5.12;
        break;
    case 2:
        x_lower_lim = -65.536;
        x_upper_lim = 65.536;
        break;
    case 3:
        x_lower_lim = -4.0;
        x_upper_lim = 4.0;
        break;
    case 4:
        x_lower_lim = -2.0;
        x_upper_lim = 2.0;
        break;
    case 5:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 6:
        x_lower_lim = -5.0;
        x_upper_lim = 10.0;
        break;
    case 7:
        x_lower_lim = -3.0;
        x_upper_lim = 3.0;
        break;
    case 8:
        x_lower_lim = -5.0;
        x_upper_lim = 5.0;
        break;
    case 9:
        x_lower_lim = -5.0;
        x_upper_lim = 10.0;
        break;
    case 10:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 11:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 12:
        x_lower_lim = -3.0;
        x_upper_lim = 4.0;
        break;
    case 13:
        x_lower_lim = -5.12;
        x_upper_lim = 5.12;
        break;
    case 14:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 15:
        x_lower_lim = -600.0;
        x_upper_lim = 600.0;
        break;
    case 16:
        x_lower_lim = -32.768;
        x_upper_lim = 32.768;
        break;
    default:
        cout << "Incorrect problem index, only integers between 1 and 16 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }


    return load_problem(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);


}



MultidimBenchmarkFF *load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    MultidimBenchmarkFF *problem;
    switch (problem_index)
    {
    case 0:
        problem = new FRandomlyGenerated(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 1:
        problem = new F1(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 2:
        problem = new F2(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 3:
        problem = new F3(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 4:
        problem = new F4(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 5:
        problem = new F5(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 6:
        problem = new F6(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 7:
        problem = new F7(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 8:
        problem = new F8(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 9:
        problem = new F9(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 10:
        problem = new F10(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 11:
        problem = new F11(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 12:
        problem = new F12(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 13:
        problem = new F13(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 14:
        problem = new F14(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 15:
        problem = new F15(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 16:
        problem = new F16(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;

    default:
        cout << "Incorrect problem index, only integers between 0 and 16 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }
    return problem;
}



