#include "wavelet/wavelets.h"
#include "test/wavelets.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define SIGNAL_LENGTH   5000
#define IDWT_LENGTH     508

#define NUM_TRIALS      1000

/**
 * Name: main
 *
 * Description:
 * This program tests the wavelet functions, first verifying that the functions
 * generate expected output, then determining the average runtime of the
 * functions.
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
  NUMTYPE *x, *c;        // Arrays for input/output of wavelet functions.
  NUMTYPE *cA, *cD;
  unsigned int *l;

  clock_t start, stop;  // Values used for timing.
  double avg_time;

  unsigned int i;       // General indexing variable.

  // Variables to store the length of the output coefficient vector and the
  // maximum deconstruction level we can use.
  unsigned int len_coef;
  unsigned int level;

  // Functions to test and the strings to print while testing them.
  int (*test_funcs[])() = { test_wavedecMaxLevel,
                            test_wavedecResultLength,
                            test_wavedec,
                            test_idwtResultLength,
                            test_idwt,
                            test_wrcoef };
  char const *test_strs[] = { "    wavedecMaxLevel...      ",
                              "    wavedecResultLength...  ",
                              "    wavedec...              ",
                              "    idwtResultLength...     ",
                              "    idwt...                 ",
                              "    wrcoef...               " };
  unsigned int num_tests = 6;

  // Verify functions provide expected output.
  printf( "Testing correctness of wavelet functions...\n" );

  for (i = 0; i < num_tests; i++) {
    printf( test_strs[i] );
    if (test_funcs[i]() == 0) {
      printf( "FAILED\n" );
      return 1;
    }
    printf( "PASSED\n" );
  }

  if (argc > 1) {
    // We were given a command line argument. Don't perform the runtime tests.
    return 0;
  }

  // Figure out the maximum level we can use and the resulting coef. length.
  level    = wavedecMaxLevel( SIGNAL_LENGTH, COIF3.len_filter );
  len_coef = wavedecResultLength( SIGNAL_LENGTH, COIF3.len_filter, level );

  // Allocate memory for the signal and output variables.
  x = (NUMTYPE*) malloc( sizeof(NUMTYPE) * SIGNAL_LENGTH );
  c = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_coef );
  l = (unsigned int*) malloc( sizeof(int) * (level + 1) );

  cA = (NUMTYPE*) malloc( sizeof(NUMTYPE) * IDWT_LENGTH );
  cD = (NUMTYPE*) malloc( sizeof(NUMTYPE) * IDWT_LENGTH );

  // Generate some test data.
  for (i = 0; i < SIGNAL_LENGTH; i++) {
    x[i] = sin( (NUMTYPE) i * 0.01 * M_PI );
  }

  for (i = 0; i < IDWT_LENGTH; i++) {
    cA[i] = sin( (NUMTYPE) i * 0.01 * M_PI );
    cD[i] = cos( (NUMTYPE) i * 0.01 * M_PI );
  }

  // Time the wavedec function.
  printf( "\nFinding average execution time of wavedec()...\n" );

  start = clock();
  for (i = 0; i < NUM_TRIALS; i++) {
    wavedec( c, l, x, SIGNAL_LENGTH, COIF3, level );
  }
  stop = clock();

  // Find the average length of wavedec.
  avg_time = (double) (stop - start) / (double) CLOCKS_PER_SEC;
  avg_time /= (double) NUM_TRIALS;

  printf( "Signal length: %d\n", SIGNAL_LENGTH );
  printf( "Filter length: %d\n", COIF3.len_filter );
  printf( "Deconstruction level: %d\n", level );
  printf( "Average execution time: %g seconds.\n", avg_time );

  // Time the idwt function.
  printf( "\n" );
  printf( "\nFinding average execution time of idwt()...\n" );

  start = clock();
  for (i = 0; i < NUM_TRIALS; i++) {
    idwt( x, cA, cD, IDWT_LENGTH, COIF3 );
  }
  stop = clock();

  // Find the average length of wavedec.
  avg_time = (double) (stop - start) / (double) CLOCKS_PER_SEC;
  avg_time /= (double) NUM_TRIALS;

  printf( "Signal length: %d\n", IDWT_LENGTH );
  printf( "Filter length: %d\n", COIF3.len_filter );
  printf( "Average execution time: %g seconds.\n", avg_time );

  // Free allocated memory.
  free( x ); free( c ); free( l );
  free( cA ); free( cD );

  return 0;
}
