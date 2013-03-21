#include "matrix.h"
#include "ica/ica.h"
#include "numtype.h"
#include "runtime.h"
#include "cmd_args/runtime.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// The size of an observation matrix will be equal to N x N * COL_STEP, where N
// is the test index, which falls in the range [MIN_TEST, MAX_TEST].

#define MIN_TEST  2
#define MAX_TEST  64
#define COL_STEP  500

/**
 * Name: main
 *
 * Description:
 * Gathers data relating the size of an observation matrix with the runtime of
 * each implementation of ICA. Only worst case runtimes are explored. See the
 * usage statement for details.
 */
int main( int argc, char **argv )
{
  int i, j, test, num_results, rows, cols;
  CmdLineArgs cmd_args;
  RuntimeResult *results;

  Matrix mat_X;

  Matrix mat_W;
  Matrix mat_A;
  Matrix mat_S;
  NUMTYPE *mu_S;

  struct timeval start, stop, diff;
  double *runtimes;
  double average;
  double std_dev;

  // Statically allocate the ICA parameters that we'll be using. Only the number
  // of variables and number of columns will vary throughout the program.
  ICAParams ica_params[IMPLEM_TYPES] = {
    {ICA_FASTICA, 0.0001, NONLIN_TANH,  400, 0, 0, 1, 0},
    {ICA_FASTICA, 0.0001, NONLIN_CUBE,  400, 0, 0, 1, 0},
    {ICA_FASTICA, 0.0001, NONLIN_GAUSS, 400, 0, 0, 1, 0},
    {ICA_JADE,    0.0001, NONLIN_TANH,  400, 0, 0, 1, 0},

    {ICA_FASTICA, 0.0001, NONLIN_TANH,  400, 0, 0, 1, 1},
    {ICA_FASTICA, 0.0001, NONLIN_CUBE,  400, 0, 0, 1, 1},
    {ICA_FASTICA, 0.0001, NONLIN_GAUSS, 400, 0, 0, 1, 1},
    {ICA_JADE,    0.0001, NONLIN_TANH,  400, 0, 0, 1, 1},
  };

  if (!parseArgs( &argc, &argv, &cmd_args )) {
    // Something went wrong.
    return 0;
  }

  // Initialize the results array.
  num_results = MAX_TEST - MIN_TEST + 1;

  results     = (RuntimeResult*) malloc( sizeof(RuntimeResult) * num_results );
  runtimes    = (double*) malloc( sizeof(double) * cmd_args.samples );

  if (cmd_args.demo) {
    // Fill the results with some dummy variables and print that to give a
    // demo of what results will look like.
    for (test = MIN_TEST; test <= MAX_TEST; test++) {
      results[test - MIN_TEST].rows = test;
      results[test - MIN_TEST].cols = test * COL_STEP;

      for (i = 0; i < IMPLEM_TYPES; i++) {
        results[test - MIN_TEST].runtimes[i] = 0.0;
        results[test - MIN_TEST].std_devs[i] = 0.0;
      }
    }

    printResults( results, num_results );
  } else {
    // Time to actually gather all the results.
    printf("Gathering timing results (this may take a while)...\n\n");

    for (test = MIN_TEST; test <= MAX_TEST; test++) {
      rows = test;
      cols = test * COL_STEP;

      printf("Testing matrix size %d x %d\n", rows, cols);
      results[test - MIN_TEST].rows = rows;
      results[test - MIN_TEST].cols = cols;

      // Allocate memory for this problem.
      mat_X.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * rows * cols );
      mat_W.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * rows * rows );
      mat_A.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * rows * rows );
      mat_S.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * rows * cols );
      mu_S       = (NUMTYPE*) malloc( sizeof(NUMTYPE) * rows );

      // Setup matrices.
      mat_X.ld  = mat_X.rows = mat_S.ld  = mat_S.rows = rows;
      mat_X.lag = mat_X.cols = mat_S.lag = mat_S.cols = cols;
      mat_W.ld  = mat_W.rows = mat_A.ld  = mat_A.rows = rows;
      mat_W.lag = mat_W.cols = mat_A.lag = mat_A.cols = rows;

      // Setup ICA parameters.
      for (i = 0; i < IMPLEM_TYPES; i++) {
        ica_params[i].num_var = rows;
        ica_params[i].num_obs = cols;
      }

      // Initialize X with a normally distributed variables.
      for (i = 0; i < mat_X.cols; i++) {
        for (j = 0; j < mat_X.rows; j++) {
          mat_X.elem[ i * mat_X.rows + j ] = nrand();
        }
      }

      // Moment of truth. Gather runtime information for each implementation.
      for (i = 0; i < IMPLEM_TYPES; i++) {
        ica_init( &(ica_params[i]) );

        for (j = 0; j < cmd_args.samples; j++) {
          gettimeofday( &start, NULL );
          ica( &mat_W, &mat_A, &mat_S, mu_S, &mat_X );
          gettimeofday( &stop, NULL );
          timersub( &stop, &start, &diff );
          runtimes[j] = (double) diff.tv_sec +
                        (double) (diff.tv_usec) * 0.000001;
        }

        // Find the average and standard deviation of those runtimes;
        average = 0.0;
        for (j = 0; j < cmd_args.samples; j++) {
          average += runtimes[j];
        }
        average /= cmd_args.samples;

        std_dev = 0.0;
        for (j = 0; j < cmd_args.samples; j++) {
          std_dev += (runtimes[j] - average) * (runtimes[j] - average);
        }
        std_dev /= cmd_args.samples;
        std_dev = sqrt(std_dev);

        results[test - MIN_TEST].runtimes[i] = average;
        results[test - MIN_TEST].std_devs[i] = std_dev;
      }

      // Print each result, one at a time, just in case the program gets
      // cancelled.
      printResults( results + (test - MIN_TEST), 1 );

      // Free memory.
      free( mat_X.elem );
      free( mat_W.elem );
      free( mat_A.elem );
      free( mat_S.elem );
      free( mu_S );
    }

    // Print the results that we obtained.
    printResults( results, num_results );
  }

  free( runtimes );
  free( results );

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
double nrand()
{
  double theta = 2 * M_PI * ((double) rand() / (double) RAND_MAX);
  double rsq   = -20.0 * log( ((double) rand() / (double) RAND_MAX) );
  return sqrt(rsq) * cos(theta);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void printResults( const RuntimeResult *results, int num_results )
{
  int i;
  printf(
"            |                                              CPU Implementation                                           |                                        GPU (CUDA) Implementation                                          |\n"
"Matrix size |                                     FastICA                                    |           JADE           |                                     FastICA                                    |           JADE           |\n"
"            |           tanh           |           cube           |           gauss          |                          |           tanh           |           cube           |           gauss          |                          |\n"
"------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+\n"
);

  for (i = 0; i < num_results; i++) {
    printf(
"%3d x %5d | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s | %9.5f s | %8.4f s |\n"
            , results[i].rows, results[i].cols,
              results[i].runtimes[res_fastica_tanh],
              results[i].std_devs[res_fastica_tanh],
              results[i].runtimes[res_fastica_cube],
              results[i].std_devs[res_fastica_cube],
              results[i].runtimes[res_fastica_gauss], 
              results[i].std_devs[res_fastica_gauss], 
              results[i].runtimes[res_jade],
              results[i].std_devs[res_jade],
              results[i].runtimes[res_gpu_fastica_tanh],
              results[i].std_devs[res_gpu_fastica_tanh],
              results[i].runtimes[res_gpu_fastica_cube],
              results[i].std_devs[res_gpu_fastica_cube],
              results[i].runtimes[res_gpu_fastica_gauss], 
              results[i].std_devs[res_gpu_fastica_gauss], 
              results[i].runtimes[res_gpu_jade],
              results[i].std_devs[res_gpu_jade] );
  }
  printf(
"------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+-------------+------------+\n"
  );
}
