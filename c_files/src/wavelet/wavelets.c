#include "wavelet/wavelets.h"
#include "convolution.h"

#include <string.h>
#include <stdlib.h>

#include <stdio.h>

/**
 * Name: floorLog2
 *
 * Description:
 * Computes the log base 2 of a given unsigned int, rounding down. Algorithm
 * taken from wikipedia: http://en.wikipedia.org/wiki/Binary_logarithm
 *
 * Parameters:
 * @param num
 *
 * Returns:
 * @return int   if num is zero, returns -1, otherwise floor( log2( num ) )
 */
int floorLog2( unsigned int num )
{
  unsigned int log = 0;
  if (num >= 1 << 16) { num >>= 16; log += 16; }
  if (num >= 1 <<  8) { num >>=  8; log +=  8; }
  if (num >= 1 <<  4) { num >>=  4; log +=  4; }
  if (num >= 1 <<  2) { num >>=  2; log +=  2; }
  if (num >= 1 <<  1) { num >>=  1; log +=  1; }
  return ((num == 0) ? (-1) : log);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline unsigned int idwtResultLength( unsigned int len_coef,
                                      unsigned int len_filter )
{
  return 2 * len_coef - len_filter + 2;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int wavedecMaxLevel( unsigned int len_signal,
                              unsigned int len_filter )
{
  // The maximum deconstruction level is given by how many times the filter can
  // fit within the input signal. The maximum deconstruction sample increase by
  // one for every power of two the filter can fit into the signal. If the
  // filter only fits twice, the maximum level is 1. If the filter fits four
  // times, the max level is 2, eight times -> 3, etc.
  return floorLog2( len_signal / len_filter );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int wavedecResultLength( unsigned int len_signal,
                                  unsigned int len_filter,
                                  unsigned int max_level )
{
  unsigned int i, g, k;

  // Verify that level is less than or equal to the maximum allowable level.
  i = wavedecMaxLevel( len_signal, len_filter );
  if (max_level > i) {
    max_level = i;
  }

  // Each deconstruction level results in two setes of coefficients, each of
  // length:
  //    g(n) = floor((n-1) / 2) + M / 2
  // where:
  //    n -> length of input to filters
  //    M -> length of filters (always even)
  // For every deconstruction level except for the first and last level, we send
  // one set of those coefficients back through the filters and store the other
  // set by appending it to the coefficient vector. For the final deconstruction
  // level we append both sets of coefficients.
  //
  // Thus, the total length of the coefficient vector, k, is equal to:
  //    k = g(n_0) + sum( g(n_i), i = 1:N-1 ) + g(n_(N-1))
  // where:
  //    N     -> maximum deconstruction level
  //    g(.)  -> as defined above
  //    n_0   -> length of signal vector
  //    n_i   -> g( n_(i-1) )

  g = (len_signal + len_filter - 1) / 2;
  k = g;

  for (i = 1; i < max_level; i++) {
    g = (g + len_filter - 1) / 2;
    k += g;
  }
  k += g;

  return k;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void wavedec( NUMTYPE *coefs,  unsigned int *lengths,
              NUMTYPE const *signal, unsigned int len_signal,
              Wavelet wavelet, unsigned int level )
{
  unsigned int i;               // Indexing variable.
  unsigned int len_coef;        // Length of the full coefficient vector.
  unsigned int i_detail;        // The offset into the coefficient vector where
                                // next detail coefficients should be stored.

  NUMTYPE *(scratch[2]);        // Scratch space used in calculations.

  // Verify that level is less than or equal to the maximum allowable level.
  i = wavedecMaxLevel( len_signal, wavelet.len_filter );
  if (level > i) {
    level = i;
  }

  // Figure out the length of the full coefficient vector and figure out how
  // much space the first level deconstruction coefficients need.
  len_coef       = wavedecResultLength( len_signal, wavelet.len_filter, level );
  lengths[level] = (len_signal + wavelet.len_filter - 1) / 2;

  // Figure out the index into the coefficient vector where we should put the
  // first details.
  i_detail  = len_coef - lengths[level];

  // Initialize scratch space.
  scratch[0] = (NUMTYPE*) malloc( sizeof(NUMTYPE) * lengths[level] );
  scratch[1] = (NUMTYPE*) malloc( sizeof(NUMTYPE) * lengths[level] );

  // Calculate the approximation coefficients, storing the result in scratch
  // space.
  conv_mirrorDown( scratch[0],
                   signal, len_signal,
                   wavelet.filter[ LOW_DEC ], wavelet.len_filter );

  // Calculate the detail coefficients.
  conv_mirrorDown( coefs + i_detail,
                   signal, len_signal,
                   wavelet.filter[ HIGH_DEC ], wavelet.len_filter );

  // Remember the length of these details.
  lengths[ level - 1 ] = lengths[level];

  // Within this loop we alternate which scratch space contains the previous
  // level's approximation and which will be used to store the current level's
  // approxmation.
  for (i = 1; i < level; i++) {
    // Compute the length of the current level.
    lengths[level - i] = (lengths[level-i+1] + wavelet.len_filter - 1) / 2;

    // Update the index into the coefficient vector.
    i_detail -= lengths[level-i];

    // Calculate the current level's detail coefficients.
    conv_mirrorDown( coefs + i_detail,
                     scratch[ (i-1)&0x01 ], lengths[level-i+1],
                     wavelet.filter[ HIGH_DEC ], wavelet.len_filter );

    // Calculate the current level's approximation coefficients.
    conv_mirrorDown( scratch[ i & 0x01 ],
                     scratch[ (i-1)&0x01 ], lengths[level-i+1],
                     wavelet.filter[ LOW_DEC ], wavelet.len_filter );
  }

  // We've now got all the details into the coefficient vector, we just need
  // to copy over the final approximation coefficients, record the final length
  // value, and then we're done.
  lengths[0] = lengths[1];
  memcpy( coefs, scratch[ (i-1)&0x01 ], sizeof(NUMTYPE) * lengths[1] );

  // Free up the scratch space.
  free( scratch[0] ); free( scratch[1] );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void wrcoef( NUMTYPE *result,
             NUMTYPE const *coefs, unsigned int const *lengths,
             unsigned int len_lengths, ReconType type, Wavelet wavelet,
             unsigned int level )
{
  unsigned int i, i_coefs;
  unsigned int max_level = len_lengths - 1;
  size_t coef_mem_size;
  NUMTYPE *cA, *cD, *scratch;

  // Make sure the level value is valid.
  if (level > max_level) {
    level = max_level;
  } else if (level == 0 && type == RECON_DETAIL) {
    level = 1;
  }

  // Allocate space for the coefficient vectors.
  coef_mem_size = sizeof(NUMTYPE) * lengths[len_lengths - 1];
  cA      = (NUMTYPE*) malloc( coef_mem_size );
  scratch = (NUMTYPE*) malloc( coef_mem_size );

  if (type == RECON_APPROX) {
    // If we're reconstructing from approximation coefficients, then we either
    // need to fetch them directly from the coefficient vector or reconstruct
    // using both approximation and detail coefficients until we get to the
    // level requested.

    // We need to setup cA for the first round of idwt() and we need to find
    // the location of the detail coefficients to use.
    memcpy( cA, coefs, sizeof(NUMTYPE) * lengths[0] );
    i_coefs = lengths[0];

    // Reconstruct using both detail and approximation coefficients until we
    // have the approximation coefficients we need.
    for (i = max_level; i > level; i--) {
      // The cast is needed to suppress a warning about the 'const'ness of the
      // coefs vector.
      cD = (NUMTYPE*) coefs + i_coefs;
      i_coefs += lengths[ len_lengths - i ];

      // Use the 'result' parameter as temporary storage space.
      idwt( result, cA, cD, lengths[ len_lengths - i ], wavelet );

      if (i != 1) {
        memcpy( cA, result, sizeof(NUMTYPE) * lengths[ len_lengths - i + 1 ] );
      }
    }
  } else {
    // If we're reconstructing from detail coefficients, we can fetch them
    // directly from the coefficient vector and reconstruct.
    i_coefs = lengths[0];
    for (i = max_level; i > level; i--) {
      i_coefs += lengths[ len_lengths - i ];
    }

    // Casting done once again to suppress a compiler warning about 'const'.
    cD = (NUMTYPE*) coefs + i_coefs;

    // When rebuilding from detail coefficients, we ignore the approximation
    // values.
    memset( cA, 0x00, coef_mem_size );

    idwt( result, cA, cD, lengths[ len_lengths - level ], wavelet );

    if (i != 1) {
      memcpy( cA, result, sizeof(NUMTYPE) * lengths[ len_lengths - i + 1 ] );
    }

    // We just did one round of idwt(), so update the 'i' value, which we use
    // a little later.
    i--;
  }

  // We now need to finish out the reconstruction so that the signal is its
  // original length. We use only the coefficients we've extracted so far,
  // setting the details to zero from here on.
  cD = scratch;
  memset( cD, 0x00, coef_mem_size );

  // 'i' has already be setup by the previous loops. We may already have the
  // result we need. If not, this last loop will get us there.
  for (; i > 0; i--) {
    idwt( result, cA, cD, lengths[ len_lengths - i ], wavelet );

    // Don't copy on the last run or we'll pull an unknown value from the
    // 'lengths' array.
    if (i != 1) {
      memcpy( cA, result, sizeof(NUMTYPE) * lengths[ len_lengths - i + 1 ] );
    }
  }

  // Free up allocated memory.
  free( cA ); free( cD );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void idwt( NUMTYPE *result, NUMTYPE const *coef_approx,
           NUMTYPE const *coef_detail, unsigned int coef_length,
           Wavelet wavelet )
{
  unsigned int i, len_conv, len_result;

  len_conv = 2 * coef_length + wavelet.len_filter - 1;
  len_result = idwtResultLength( coef_length, wavelet.len_filter );

  NUMTYPE *(scratch[2]);        // Scratch space used in calculations.

  // Initialize scratch space.
  scratch[0] = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_conv );
  scratch[1] = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_conv );

  // Convolve the approxmimation coefficients with the low pass recon. filter.
  conv_mirrorUp( scratch[0], coef_approx, coef_length,
                 wavelet.filter[ LOW_REC ], wavelet.len_filter );

  // Convolve the detail coefficients with the high pass recon. filter.
  conv_mirrorUp( scratch[1], coef_detail, coef_length,
                 wavelet.filter[ HIGH_REC ], wavelet.len_filter );

  // Sum the two convolutions.
  for (i = 0; i < len_conv; i++) {
    scratch[1][i] += scratch[0][i];
  }

  // The result is the sum minus a few extraneous values on the edges that
  // result from the convolution.
  memcpy( result, scratch[1] + wavelet.len_filter - 1,
          sizeof(NUMTYPE) * len_result );

  // Free allocated memory.
  free( scratch[0] ); free( scratch[1] );
}
