#ifndef TEST_CONVOLUTION_H
#define TEST_CONVOLUTION_H

#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: test_conv_mirrorDown
 *
 * Description:
 * Verifies that the conv_mirrorDown() function gives expected results.
 *
 * Return:
 * @return int    0 if test fails, nonzero otherwise
 */
int test_conv_mirrorDown();

/**
 * Name: test_conv_mirrorUp
 *
 * Description:
 * Verifies that the conv_mirrorUp() function gives expected results.
 *
 * Return:
 * @return int    0 if test fails, nonzero otherwise
 */
int test_conv_mirrorUp();

#ifdef __cplusplus
}
#endif

#endif
