#include "wavelet/wavelets.h"
#include "test/wavelets.h"

#include <math.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>

#define EPSILON     0.000001

typedef struct MultiArray {
  NUMTYPE *array;
  unsigned int length;
} MultiArray;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_wavedecMaxLevel()
{
  unsigned int signal_lens[] = { 5000, 10000, 2303, 2304, 2559, 2560 };
  unsigned int filter_lens[] = {   18,    18,   18,   18,   40,   40 };
  unsigned int results[]     = {    8,     9,    6,    7,    5,    6 };

  unsigned int i;
  for (i = 0; i < sizeof(signal_lens) / sizeof(unsigned int); i++) {
    if (wavedecMaxLevel( signal_lens[i], filter_lens[i] ) != results[i]) {
      return 0;
    }
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_wavedecResultLength()
{
  unsigned int signal_lens[] = { 5000, 5000, 2303, 2304, 2559, 2560 };
  unsigned int filter_lens[] = {   18,   18,   18,   18,   40,   40 };
  unsigned int levels[]      = {    7,    8,    8,    8,    8,    8 };
  unsigned int results[]     = { 5113, 5130, 2401, 2417, 2752, 2791 };

  unsigned int i;
  for (i = 0; i < sizeof(signal_lens) / sizeof(unsigned int); i++) {
    if (wavedecResultLength( signal_lens[i],
                             filter_lens[i],
                             levels[i] ) != results[i]) {
      return 0;
    }
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_wavedec()
{
  #include "test/wavelet/data_wavedec/wavedec_include.snip"

  unsigned int i, j, level, len_signal, len_output, retval = 1;
  unsigned int *lengths;
  NUMTYPE *output;

  for (j = 0; j < sizeof(signals) / sizeof(MultiArray); j++) {
    len_signal = signals[j].length;

    // Find out the deconstruction level we're supposed to perform.
    level = l_vectors[j].length - 1;

    // Find out how long to make the output.
    len_output = wavedecResultLength( len_signal, COIF3.len_filter, level );
    output  = (NUMTYPE*)      malloc( sizeof(NUMTYPE) * (len_output + 1) );
    lengths = (unsigned int*) malloc( sizeof(unsigned int) * (level + 2) );

    // Set the last values of the output and lengths arrays so we can make sure
    // the wavelet functions don't overwrite them.
    output[len_output] = 3.14;
    lengths[level + 1] = 31415926;

    // Perform the decomposition.
    wavedec( output, lengths, signals[j].array, len_signal, COIF3, level );

    // Verify the results.
    for (i = 0; i < c_vectors[j].length; i++) {
      if (fabs(c_vectors[j].array[i] - output[i]) > EPSILON) {
        retval = 0;
      }
    }

    for (i = 0; i < l_vectors[j].length; i++) {
      if (lengths[i] != (unsigned int) l_vectors[j].array[i]) {
        retval = 0;
      }
    }

    // Verify that the last values of the output and lengths arrays haven't been
    // overwritten.
    if (output[len_output] != 3.14 || lengths[level+1] != 31415926) {
      retval = 0;
    }

    // Free our allocated memory, and return.
    free( output ); free( lengths );
  }

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_idwtResultLength()
{
  unsigned int coef_lens[]   = {  78, 196,  36,  55,  56,  94 };
  unsigned int filter_lens[] = {  40,  40,  18,  18,  18,  18 };
  unsigned int results[]     = { 118, 354,  56,  94,  96, 172 };

  unsigned int i;
  for (i = 0; i < sizeof(coef_lens) / sizeof(unsigned int); i++) {
    if (idwtResultLength( coef_lens[i], filter_lens[i] ) != results[i]) {
      return 0;
    }
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_wrcoef()
{
  #include "test/wavelet/data_wrcoef/wrcoef_include.snip"

  int retval = 1;
  NUMTYPE *output;

  unsigned int len_signal, len_lengths, len_result, *lengths, i, j;

  for (j = 0; j < sizeof(results) / sizeof(MultiArray); j++) {
    len_result  = results[j].length;
    len_lengths = l_vectors[j].length;
    len_signal  = idwtResultLength(
                    (unsigned int) l_vectors[j].array[len_lengths - 1],
                    COIF3.len_filter);

    output  = (NUMTYPE*) malloc( sizeof(NUMTYPE) * (len_signal + 1) );
    lengths = (unsigned int*) malloc( sizeof(int) * len_lengths );
    output[ len_signal ] = 3.14;

    for (i = 0; i < len_lengths; i++) {
      lengths[i] = (unsigned int) l_vectors[j].array[i];
    }

    wrcoef( output, c_vectors[j].array, lengths, len_lengths, types[j],
            COIF3, 5 );

    for (i = 0; i < len_result; i++) {
      if (fabs( results[j].array[i] - output[i] ) > EPSILON) {
        retval = 0;
      }
    }

    if (output[ len_signal ] != 3.14) {
      retval = 0;
    }

    free( output ); free( lengths );
  }

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_idwt()
{
  #include "test/wavelet/data_idwt/idwt_include.snip"
  NUMTYPE *output;

  unsigned int coef_length, output_length, i, j;
  int retval = 1;

  for (j = 0; j < sizeof(results) / sizeof(MultiArray); j++) {
    coef_length   = a_coefs[j].length;
    output_length = idwtResultLength( coef_length, COIF3.len_filter );

    output = (NUMTYPE*) malloc( sizeof(NUMTYPE) * (output_length + 1) );
    output[ output_length ] = 3.14;

    // Perform the inverset DWT.
    idwt( output, a_coefs[j].array, d_coefs[j].array, coef_length, COIF3 );

    // Verify that all values are within an epsilon of expected values.
    for (i = 0; i < output_length; i++) {
      if (fabs(output[i] - results[j].array[i]) > EPSILON) {
        retval = 0;
      }
    }

    // Verify that the final value was not overwritten.
    if (output[output_length] != 3.14) {
      retval = 0;
    }

    // Free our allocated memory.
    free( output );
  }

  return retval;
}
