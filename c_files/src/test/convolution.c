#include "numtype.h"
#include "convolution.h"
#include "test/convolution.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_conv_mirrorDown()
{
  unsigned int i, retval = 1;
  NUMTYPE epsilon = 0.00001;

  // Generate some test data and the expected results. The last value in the
  // result and output arrays is set to a known value so that we can verify
  // that the convolution function does not attempt to give us more results
  // than we expect.
  NUMTYPE x1[] = {0.2, 0.4, 3.2, 1.3, -4.2, 8.2, -12.1, 3.4, 0.0, 0.0, 2.3,0.4};
  NUMTYPE h1[] = {1.3, 2.7, 0.3, 2.9,  3.1};
  NUMTYPE r1[] = {14.49, 12.27, 10.23, -33.94, -8.65, 17.27, 13.0, 12.33, 3.14};
  NUMTYPE y1[9] = {0}; y1[8] = 3.14;

  unsigned int l_x1 = sizeof(x1) / sizeof(NUMTYPE);
  unsigned int l_h1 = sizeof(h1) / sizeof(NUMTYPE);
  unsigned int l_y1 = sizeof(y1) / sizeof(NUMTYPE);

  NUMTYPE x2[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 2.0,
                 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0,
                 1.0};
  NUMTYPE h2[] = {0.3, 0.2, 0.1, 0.4, 0.5, 0.3, 0.2};
  NUMTYPE r2[] = {6.9, 5.1, 6.1, 8.1, 8.9, 6.9, 5.1, 6.1, 9.7, 12.1, 12.9, 10.3,
                 6.9, 5.1, 6.1, 3.14};
  NUMTYPE y2[16] = {0}; y2[15] = 3.14;

  unsigned int l_x2 = sizeof(x2) / sizeof(NUMTYPE);
  unsigned int l_h2 = sizeof(h2) / sizeof(NUMTYPE);
  unsigned int l_y2 = sizeof(y2) / sizeof(NUMTYPE);

  conv_mirrorDown( y1, x1, l_x1, h1, l_h1 );
  conv_mirrorDown( y2, x2, l_x2, h2, l_h2 );

  for (i = 0; i < l_y1; i++) {
    if (fabs(r1[i] - y1[i]) > epsilon) {
      retval = 0;
      break;
    }
  }

  for (i = 0; i < l_y2; i++) {
    if (fabs(r2[i] - y2[i]) > epsilon) {
      retval = 0;
      break;
    }
  }

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int test_conv_mirrorUp()
{
  unsigned int i, retval = 1;
  NUMTYPE epsilon = 0.00001;

  // Generate some test data and the expected results. The last value in the
  // result and output arrays is set to a known value so that we can verify
  // that the convolution function does not attempt to give us more results
  // than we expect.

  // Three sets of data are tested: two small filters and inputs, one with an
  // even length filter, one with an odd length, and one less small input and
  // filter using an odd length filter.

  // Results were orginally generated in MATLAB.
  NUMTYPE x1[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  NUMTYPE h1[] = {1.2, 3.4, -2.0, 0.2};
  NUMTYPE r1[] = {3.8, -0.8, 3.6, 0.4, 7.0, -0.4, 10.6, -1.2, 14.2, -2.0, 17.8,
                 -5.2, 14.6, 3.14};
  NUMTYPE y1[14] = {0}; y1[13] = 3.14;

  unsigned int l_x1 = sizeof(x1) / sizeof(NUMTYPE);
  unsigned int l_h1 = sizeof(h1) / sizeof(NUMTYPE);
  unsigned int l_y1 = sizeof(y1) / sizeof(NUMTYPE);

  NUMTYPE x2[] = {1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0};
  NUMTYPE h2[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  NUMTYPE r2[] = {10.0, 14.0, 6.0, 10.0, 8.0, 14.0, 14.0, 23.0, 20.0, 32.0,
                 26.0, 39.0, 28.0, 40.0, 22.0, 31.0, 16.0, 22.0, 10.0, 15.0,
                 8.0, 14.0, 3.14};
  NUMTYPE y2[23] = {0}; y2[22] = 3.14;

  unsigned int l_x2 = sizeof(x2) / sizeof(NUMTYPE);
  unsigned int l_h2 = sizeof(h2) / sizeof(NUMTYPE);
  unsigned int l_y2 = sizeof(y2) / sizeof(NUMTYPE);

  NUMTYPE x3[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, -1.0, -2.0, -3.0, -4.0, -5.0,
                 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
  NUMTYPE h3[] = {1.0, 2.0, 3.0, 4.0, 5.0, 4.0, 3.0, 2.0, 1.0};
  NUMTYPE r3[] = {30.0, 27.0, 20.0, 19.0, 16.0, 19.0, 20.0, 27.0, 30.0, 39.0,
                 42.0, 52.0, 54.0, 57.0, 50.0, 44.0, 26.0, 9.0, -6.0, -20.0,
                 -30.0, -39.0, -42.0, -40.0, -30.0, -17.0, 6.0, 30.0, 42.0,
                 53.0, 54.0, 52.0, 42.0, 39.0, 30.0, 28.0, 22.0, 23.0, 22.0,
                 28.0, 30.0, 39.0, 3.14};
  NUMTYPE y3[43] = {0}; y3[42] = 3.14;

  unsigned int l_x3 = sizeof(x3) / sizeof(NUMTYPE);
  unsigned int l_h3 = sizeof(h3) / sizeof(NUMTYPE);
  unsigned int l_y3 = sizeof(y3) / sizeof(NUMTYPE);


  conv_mirrorUp( y1, x1, l_x1, h1, l_h1 );
  conv_mirrorUp( y2, x2, l_x2, h2, l_h2 );
  conv_mirrorUp( y3, x3, l_x3, h3, l_h3 );

  for (i = 0; i < l_y1; i++) {
    if (fabs(r1[i] - y1[i]) > epsilon) {
      retval = 0;
      break;
    }
  }

  for (i = 0; i < l_y2; i++) {
    if (fabs(r2[i] - y2[i]) > epsilon) {
      retval = 0;
      break;
    }
  }

  for (i = 0; i < l_y3; i++) {
    if (fabs(r3[i] - y3[i]) > epsilon) {
      retval = 0;
      break;
    }
  }

  return retval;
}
