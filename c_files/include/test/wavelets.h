#ifndef TEST_WAVELETS_H
#define TEST_WAVELETS_H

#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: test_wavedecMaxLevel
 *
 * Description:
 * Verifies that the wavedecMaxLevel() function gives expected output.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_wavedecMaxLevel();

/**
 * Name: test_wavedecResultLength
 *
 * Description:
 * Verifies that the wavedecResultLength() function gives expected output.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_wavedecResultLength();

/**
 * Name: test_wavedec
 *
 * Description:
 * Verifies that the wavedec() function gives expected results.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_wavedec();

/**
 * Name: test_wrcoef
 *
 * Description:
 * Verifies that the wrcoef() function gives expected results.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_wrcoef();

/**
 * Name: test_idwtResultLength
 *
 * Description:
 * Verifies that the idwtResultLength() function gives expected results.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_idwtResultLength();

/**
 * Name: test_idwt
 *
 * Verifies that the idwt() function gives expected results.
 *
 * Returns:
 * @return int  0 if test fails, nonzero otherwise
 */
int test_idwt();

#ifdef __cplusplus
}
#endif

#endif
