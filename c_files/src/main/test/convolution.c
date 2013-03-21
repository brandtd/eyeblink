#include "convolution.h"
#include "test/convolution.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define DOWN_INPUT_LENGTH   5000
#define DOWN_FILTER_LENGTH  30
#define DOWN_RESULT_LENGTH  ((DOWN_INPUT_LENGTH + DOWN_FILTER_LENGTH) / 2)

#define UP_INPUT_LENGTH     2500
#define UP_FILTER_LENGTH    30
#define UP_RESULT_LENGTH    (2*UP_INPUT_LENGTH + UP_FILTER_LENGTH)

#define NUM_TRIALS          1000

/**
 * Name: main
 *
 * Description:
 * This program tests the convolution functions, first verifying that the
 * functions generate the expected output, then determining the average runtime
 * of the functions.
 *
 * Statistics are printed to stdout with the results of this test.
 *
 * If given any command line arguments, the runtime analysis is not performed.
 *
 * Parameters:
 * @param argc    the standard
 * @param argv    the standard
 *
 * Returns:
 * @return int    0 if tests pass, nonzero otherwise
 */
int main( int argc, char **argv )
{
  NUMTYPE *x, *h, *y;   // Arrays for input signal, filter, and result.
  clock_t start, stop;  // Used for timing convolution.
  double avg_time;      // Average amount of time used per convolution call.
  int i;                // General indexing variable.
  int x_len, h_len, y_len;

  // Functions to test and the strings to print while testing them.
  int (*test_funcs[])() = { test_conv_mirrorUp,
                            test_conv_mirrorDown };
  char const *test_strs[] = { "    conv_mirrorUp...     ",
                              "    conv_mirrorDown...   " };
  int num_tests = 2;

  // Verify that the convolution functions are working as expected.
  printf( "Testing correctness of convolution functions...\n" );

  for (i = 0; i < num_tests; i++) {
    printf( test_strs[i] );
    if (test_funcs[i]() == 0) {
      printf( "FAILED\n" );
      return 1;
    }
    printf( "PASSED\n" );
  }

  if (argc > 1) {
    // We were given a command line argument. Don't run the runtime tests.
    return 0;
  }

  x_len = DOWN_INPUT_LENGTH  > UP_INPUT_LENGTH  ? DOWN_INPUT_LENGTH  : UP_INPUT_LENGTH;
  h_len = DOWN_FILTER_LENGTH > UP_FILTER_LENGTH ? DOWN_FILTER_LENGTH : UP_FILTER_LENGTH;
  y_len = DOWN_RESULT_LENGTH > UP_RESULT_LENGTH ? DOWN_RESULT_LENGTH : UP_RESULT_LENGTH;

  // Allocate memory for the convolution variables.
  x = (NUMTYPE*) malloc( sizeof(NUMTYPE) * x_len );
  h = (NUMTYPE*) malloc( sizeof(NUMTYPE) * h_len );
  y = (NUMTYPE*) malloc( sizeof(NUMTYPE) * y_len );

  // Generate some test data to use.
  for (i = 0; i < x_len; i++) {
    x[i] = sin( (NUMTYPE) i * 0.01 * M_PI );
  }

  for (i = 0; i < h_len; i++) {
    h[i] = cos( (NUMTYPE) i * 0.001 * M_PI );
  }

  // Time the down-sampling convolution by performing it NUM_TRIALS times.
  printf("\nFinding average execution time of down-sampling convolution...\n");

  start = clock();
  for (i = 0; i < NUM_TRIALS; i++) {
    conv_mirrorDown( y, x, DOWN_INPUT_LENGTH, h, DOWN_FILTER_LENGTH );
  }
  stop = clock();

  // Find the average length of the convolution.
  avg_time = (double) (stop - start) / (double) CLOCKS_PER_SEC;
  avg_time /= (double) NUM_TRIALS;

  printf( "Input length:  %d\n", DOWN_INPUT_LENGTH );
  printf( "Filter length: %d\n", DOWN_FILTER_LENGTH );
  printf( "Average execution time: %g seconds.\n", avg_time );

  // Do the same steps, this time for the up-sampling convolution.
  printf("\nFinding average execution time of up-sampling convolution...\n");
  start = clock();
  for (i = 0; i < NUM_TRIALS; i++) {
    conv_mirrorUp( y, x, UP_INPUT_LENGTH, h, UP_FILTER_LENGTH );
  }
  stop = clock();

  avg_time = (double) (stop - start) / (double) CLOCKS_PER_SEC;
  avg_time /= (double) NUM_TRIALS;

  printf( "Input length:  %d\n", UP_INPUT_LENGTH );
  printf( "Filter length: %d\n", UP_FILTER_LENGTH );
  printf( "Average execution time: %g seconds.\n", avg_time );

  free( x ); free( y ); free( h );

  return 0;
}
