#ifndef WAVELET_COEFFS_H
#define WAVELET_COEFFS_H

#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file contains extern declarations of the available wavelet filters. The
 * Available filters are:
 *
 *    Daubechies:   DB1   - DB20
 *    Symlets:      SYM2  - SYM20
 *    Coiflets:     COIF1 - COIF5
 *    Biorthogonal: BIOR 1.1, 1.3, 1.5,
 *                       2.2, 2.4, 2.6, 2.8,
 *                       3.1, 3.3, 3.5, 3.7, 3.9,
 *                       4.4, 5.5, 6.8
 *    Discrete Meyere wavelet approximation: DMEY
 *
 * Biorthogonal wavelet X.Y is accessed through the variable BIORX_Y, e.g.
 * the filter coefficients for biorthogonal wavelet 1.5 is in the variable
 * BIOR1_5.
 *
 *
 * NOTE: this coefficient file is a derivative work of the wavelet coefficient
 *       file copyright Filip Wasilewski <http://filipwasilewski.pl/> and
 *       provided with the python wavelet transform, pywt, package available
 *       from http://wavelets.pybytes.com/ 
 */

/**
 * The filter field of the Wavelet struct is a 4-element array of const pointers
 * to const NUMTYPEs (all the consts are just to make sure no one accidently
 * overwrites a filter value, which would break all the wavelet transforms using
 * that filter).
 *
 * The elements in the filter array are organized as:
 *    filter[0] -> low-pass  deconstruction filter
 *    filter[1] -> high-pass deconstruction filter
 *    filter[2] -> low-pass  reconstruction filter
 *    filter[3] -> high-pass reconstruction filter
 *
 * The len_filter field describes the length of each filter (each filter is the
 * same length).
 *
 * The filters can also be correctly accessed via the #defines LOW_DEC,
 * HIGH_DEC, LOW_REC, and HIGH_REC.
 */
typedef struct Wavelet {
  NUMTYPE const * const (filter[4]);
  unsigned int const len_filter;
} Wavelet;

#define LOW_DEC   0
#define HIGH_DEC  1
#define LOW_REC   2
#define HIGH_REC  3

extern const Wavelet DB1;
extern const Wavelet DB2;
extern const Wavelet DB3;
extern const Wavelet DB4;
extern const Wavelet DB5;
extern const Wavelet DB6;
extern const Wavelet DB7;
extern const Wavelet DB8;
extern const Wavelet DB9;
extern const Wavelet DB10;
extern const Wavelet DB11;
extern const Wavelet DB12;
extern const Wavelet DB13;
extern const Wavelet DB14;
extern const Wavelet DB15;
extern const Wavelet DB16;
extern const Wavelet DB17;
extern const Wavelet DB18;
extern const Wavelet DB19;
extern const Wavelet DB20;
extern const Wavelet SYM2;
extern const Wavelet SYM3;
extern const Wavelet SYM4;
extern const Wavelet SYM5;
extern const Wavelet SYM6;
extern const Wavelet SYM7;
extern const Wavelet SYM8;
extern const Wavelet SYM9;
extern const Wavelet SYM10;
extern const Wavelet SYM11;
extern const Wavelet SYM12;
extern const Wavelet SYM13;
extern const Wavelet SYM14;
extern const Wavelet SYM15;
extern const Wavelet SYM16;
extern const Wavelet SYM17;
extern const Wavelet SYM18;
extern const Wavelet SYM19;
extern const Wavelet SYM20;
extern const Wavelet COIF1;
extern const Wavelet COIF2;
extern const Wavelet COIF3;
extern const Wavelet COIF4;
extern const Wavelet COIF5;
extern const Wavelet BIOR1_1;
extern const Wavelet BIOR1_3;
extern const Wavelet BIOR1_5;
extern const Wavelet BIOR2_2;
extern const Wavelet BIOR2_4;
extern const Wavelet BIOR2_6;
extern const Wavelet BIOR2_8;
extern const Wavelet BIOR3_1;
extern const Wavelet BIOR3_3;
extern const Wavelet BIOR3_5;
extern const Wavelet BIOR3_7;
extern const Wavelet BIOR3_9;
extern const Wavelet BIOR4_4;
extern const Wavelet BIOR5_5;
extern const Wavelet BIOR6_8;
extern const Wavelet DMEY;

#ifdef __cplusplus
}
#endif

#endif
