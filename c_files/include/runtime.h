#ifndef ICA_RUNTIME_H
#define ICA_RUNTIME_H

#define IMPLEM_TYPES  8

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct for storing a runtime result.
 */
typedef struct RuntimeResult {
  int rows;                       // Number of rows in observation matrix.
  int cols;                       // Number of columns in observation matrix.
  double runtimes[IMPLEM_TYPES];  // The runtimes for each implementation type.
  double std_devs[IMPLEM_TYPES];  // The standard deviations of each type.
} RuntimeResult;

/**
 * An enum for indexing the `runtimes' field in the RuntimeResult struct.
 */
typedef enum ResultIndex {
  res_fastica_tanh = 0,
  res_fastica_cube,
  res_fastica_gauss,
  res_jade,

  res_gpu_fastica_tanh,
  res_gpu_fastica_cube,
  res_gpu_fastica_gauss,
  res_gpu_jade
} ResultIndex;

/**
 * Name: printResults
 *
 * Description:
 * Prints the given results in a pretty format.
 *
 * Parameters:
 * @param results       an array of results to print
 * @param num_results   how many results are in the array
 */
void printResults( const RuntimeResult *results, int num_results );

/**
 * Name: nrand
 *
 * Description:
 * Returns a normally distributed random variable. Implementation of this
 * function comes from:
 *  http://www.mas.ncl.ac.uk/~ndjw1/teaching/sim/transf/norm.html
 *
 * Returns:
 * @return double       normally distributed random number
 */
double nrand();

#ifdef __cplusplus
}
#endif

#endif
