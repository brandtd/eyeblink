#include "matrix.h"
#include "ica/ica.h"
#include "numtype.h"

#include "cmd_args/ica.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/**
 * Name: main
 *
 * Description:
 * This program generates runtime data for the C implementation of the FastICA
 * algorithm and verifies that it is returning expected results. See the usage
 * statement for details (run with the '-h' option).
 */
int main( int argc, char **argv )
{
  CmdLineArgs cmd_args;
  ICAParams ica_params;
  NUMTYPE *mu_Sa;
  int i, j;

  unsigned int num_iter[2];
#ifdef ENABLE_GPU
  double gpu_init;
  double gpu_exec;
#endif

  double cpu_init;
  double cpu_exec;

  struct timeval start, stop, diff;

  // Provide names for each matrix. It makes the code easier to read. Having
  // the array makes initializing and freeing the matrices easier.
  Matrix X, Wa, Aa, Sa;
  Matrix *my_matrices[] = { &X, &Wa, &Aa, &Sa };

  // Parse the command line arguments.
  if (!parseArgs( &argc, &argv, &cmd_args)) {
    // Either an invalid parameter was given or the help flag was specified.
    // Either way we should exit.
    return 1;
  }

  ica_params.contrast = cmd_args.contrast;
  ica_params.epsilon  = cmd_args.epsilon;
  ica_params.max_iter = cmd_args.max_iter;
  ica_params.implem   = cmd_args.implem;

  // Open up each matrix file that we were given. If we're checking output,
  // increment the argv[] index by 5 every iteration, otherwise, only increment
  // by one.
  for (i = 1; i < argc; i++) {
    mat_newFromFile( &X, argv[i] );

    // Initialize the other matrices.
    Wa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * X.rows * X.rows );
    Aa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * X.rows * X.rows );
    Sa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * X.rows * X.cols );
    mu_Sa   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * X.rows );

    Wa.rows = Wa.cols = Wa.ld = Wa.lag = X.rows;
    Aa.rows = Aa.cols = Aa.ld = Aa.lag = X.rows;

    Sa.rows = Sa.ld  = X.rows;
    Sa.cols = Sa.lag = X.cols;

    ica_params.num_var = X.rows;
    ica_params.num_obs = X.cols;

#ifdef ENABLE_GPU
    // With everything initialized, run and time the ICA algorithm.
    if (cmd_args.gpu_only || cmd_args.compare) {
      // Make sure the GPU implementation is enabled.
      ica_params.use_gpu = 1;

      // Set the correct GPU device.
      ica_params.gpu_device = 1;

      // Initialize the ICA library.
      gettimeofday( &start, NULL );
        ica_init( &ica_params );
      gettimeofday( &stop, NULL );
      timersub( &stop, &start, &diff );

      gpu_init = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;
      printf("GPU setup time:     %g seconds.\n", gpu_init);

      gettimeofday( &start, NULL );
        num_iter[1] = ica( &Wa, &Aa, &Sa, mu_Sa, &X );
      gettimeofday( &stop, NULL );
      timersub( &stop, &start, &diff );

      gpu_exec = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;
      printf("GPU execution time: %g seconds.\n", gpu_exec);
      printf("GPU iterations/sweeps: %d\n", num_iter[1] );

      if (isnan(Sa.elem[0])) {
        printf("GPU NaN\n");
      }

      if (cmd_args.print) {
        mat_printToFile( "Wgpu.csv", &Wa, ROW_MAJOR );
        mat_printToFile( "Agpu.csv", &Aa, ROW_MAJOR );
        mat_printToFile( "Sgpu.csv", &Sa, ROW_MAJOR );
      }
    }
#endif

    if (!cmd_args.gpu_only) {
      // Make sure the CPU implementation is enabled.
      ica_params.use_gpu = 0;

      // Initialize the ICA library.
      gettimeofday( &start, NULL );
        ica_init( &ica_params );
      gettimeofday( &stop, NULL );
      timersub( &stop, &start, &diff );

      cpu_init = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;
      printf("CPU setup time:     %g seconds.\n", cpu_init);

      gettimeofday( &start, NULL );
        num_iter[0] = ica( &Wa, &Aa, &Sa, mu_Sa, &X );
      gettimeofday( &stop, NULL );
      timersub( &stop, &start, &diff );

      cpu_exec = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;
      printf("CPU execution time: %g seconds.\n", cpu_exec);
      printf("CPU iterations/sweeps: %d\n", num_iter[0] );

      if (isnan(Sa.elem[0])) {
        printf("CPU NaN\n");
      }

      if (cmd_args.print) {
        mat_printToFile( "Wcpu.csv", &Wa, ROW_MAJOR );
        mat_printToFile( "Acpu.csv", &Aa, ROW_MAJOR );
        mat_printToFile( "Scpu.csv", &Sa, ROW_MAJOR );
      }
    }

    // Free allocated memory.
    for (j = 0; j < sizeof(my_matrices) / sizeof(Matrix*); j++) {
      mat_freeMatrix( my_matrices[j] );
    }
    free( mu_Sa );
    mu_Sa = NULL;

    // Shutdown ICA library.
    ica_shutdown();
  }

  return 0;
}
