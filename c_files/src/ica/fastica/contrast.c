#include "ica/fastica/contrast.h"

#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef MULTITHREAD_CONTRAST
  typedef struct TanhThreadData {
    NUMTYPE *array;
    int length;
  } TanhThreadData;

  typedef struct CubeThreadData {
    NUMTYPE *wz_sqr;
    NUMTYPE *wz_cube;
    int length;
  } CubeThreadData;

  typedef struct GaussThreadData {
    NUMTYPE *wz;
    NUMTYPE *wz_sqr;
    NUMTYPE *wz_expo;
    int length;
  } GaussThreadData;

  void *thr_tanh( void *data );
  void *thr_tanh_sqr( void *data );
  void *thr_cube( void *data );
  void *thr_gauss( void *data );
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void negent_tanh( Matrix *W_next, Matrix *W, Matrix *Z )
{
  int row, col, i;
  Matrix hyp_tan;

  // Initialize our variables.
  hyp_tan.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  hyp_tan.rows = hyp_tan.ld = Z->rows;
  hyp_tan.cols = hyp_tan.lag = Z->cols;

  // Find the tanh() of W * Z.
  GEMM( hyp_tan, *W, *Z );

#ifdef MULTITHREAD_CONTRAST
  // Threading variables.
  pthread_attr_t attr;
  TanhThreadData tdata[NUM_THREADS - 1];
  pthread_t      thread_ids[NUM_THREADS - 1];
  int            block_length;

  pthread_attr_init( &attr );

  block_length = (hyp_tan.rows * hyp_tan.cols) / NUM_THREADS;

  for (i = 0; i < NUM_THREADS - 1; i++) {
    tdata[i].array  = hyp_tan.elem + i * block_length;
    tdata[i].length = block_length;
  }

  // Multithread the calculation of the tanh().
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_create( &thread_ids[i], &attr, thr_tanh, (void*) &tdata[i] );
  }

  for (i = (NUM_THREADS-1)*block_length; i < hyp_tan.cols * hyp_tan.rows; i++) {
    hyp_tan.elem[i] = tanh( hyp_tan.elem[i] );
  }

  // Wait for threads to complete.
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_join( thread_ids[i], NULL );
  }
#else
  for (i = 0; i < hyp_tan.cols * hyp_tan.rows; i++) {
    hyp_tan.elem[i] = tanh( hyp_tan.elem[i] );
  }
#endif

  GEMM_NT( *W_next, hyp_tan, *Z );

  // Convert the tanh() values into values of the derivative of tanh(). The
  // derivative of tanh() is 1 - tanh^2().
#ifdef MULTITHREAD_CONTRAST
  // We can reuse the threading variables exactly as they were created before.
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_create( &thread_ids[i], &attr, thr_tanh_sqr, (void*) &tdata[i] );
  }

  for (i = (NUM_THREADS-1)*block_length; i < hyp_tan.cols * hyp_tan.rows; i++) {
    hyp_tan.elem[i] = 1.0 - hyp_tan.elem[i] * hyp_tan.elem[i];
  }

  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_join( thread_ids[i], NULL );
  }

  pthread_attr_destroy( &attr );
#else
  for (i = 0; i < hyp_tan.cols * hyp_tan.rows; i++) {
    hyp_tan.elem[i] = 1.0 - hyp_tan.elem[i] * hyp_tan.elem[i];
  }
#endif

  // Sum the rows of this new matrix (in preparation of finding their average).
  for (col = 1; col < hyp_tan.cols; col++) {
    for (row = 0; row < hyp_tan.rows; row++) {
      hyp_tan.elem[row] += hyp_tan.elem[col*hyp_tan.rows + row];
    }
  }

  // Put everything together.
  for (col = 0; col < W_next->cols; col++) {
    for (row = 0; row < W_next->rows; row++) {
      i = col * W_next->rows + row;
      W_next->elem[i] = (W_next->elem[i] -
                         hyp_tan.elem[row] * W->elem[i]) / Z->cols;
    }
  }

  // Free the space we allocated.
  free( hyp_tan.elem );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void negent_cube( Matrix *W_next, Matrix *W, Matrix *Z )
{
  int row, col, i;

  Matrix WZ_cubed;
  Matrix WZ_squared;

  // Initialize our matrices.
  WZ_cubed.elem   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  WZ_squared.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  WZ_squared.rows = WZ_cubed.rows = WZ_squared.ld = WZ_cubed.ld = Z->rows;
  WZ_squared.cols = WZ_cubed.cols = Z->cols;
  WZ_squared.lag  = WZ_cubed.lag  = Z->cols;

  GEMM( WZ_cubed, *W, *Z );

  // Compute the square and cube of each element in W * Z. We do not use the
  // pow() function here because it NUMTYPEs the runtime of the ica() algorithm.
#ifndef MULTITHREAD_CONTRAST
  for (i = 0; i < WZ_cubed.rows * WZ_cubed.cols; i++) {
    WZ_squared.elem[i] = 3.0 * (WZ_cubed.elem[i] * WZ_cubed.elem[i]);
    WZ_cubed.elem[i] = WZ_cubed.elem[i] * WZ_cubed.elem[i] * WZ_cubed.elem[i];
  }
#else
  // Threading variables.
  pthread_attr_t attr;
  CubeThreadData cdata[NUM_THREADS - 1];
  pthread_t      thread_ids[NUM_THREADS - 1];
  int            block_length;

  pthread_attr_init( &attr );

  block_length = (WZ_cubed.rows * WZ_cubed.cols) / NUM_THREADS;

  for (i = 0; i < NUM_THREADS - 1; i++) {
    cdata[i].wz_sqr  = WZ_squared.elem + i * block_length;
    cdata[i].wz_cube = WZ_cubed.elem   + i * block_length;
    cdata[i].length  = block_length;
  }

  // Multithread the calculation of the tanh().
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_create( &thread_ids[i], &attr, thr_cube, (void*) &cdata[i] );
  }

  for (i=(NUM_THREADS-1)*block_length; i < WZ_cubed.rows * WZ_cubed.cols; i++) {
    WZ_squared.elem[i] = 3.0 * (WZ_cubed.elem[i] * WZ_cubed.elem[i]);
    WZ_cubed.elem[i] = WZ_cubed.elem[i] * WZ_cubed.elem[i] * WZ_cubed.elem[i];
  }

  // Wait for threads to complete.
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_join( thread_ids[i], NULL );
  }

  pthread_attr_destroy( &attr );
#endif

  GEMM_NT( *W_next, WZ_cubed, *Z );

  // Find the sum of the squared element rows (in preparation of finding their
  // mean).
  for (col = 1; col < WZ_squared.cols; col++) {
    for (row = 0; row < WZ_squared.rows; row++) {
      WZ_squared.elem[row] += WZ_squared.elem[col * WZ_squared.rows + row];
    }
  }

  // Put everything together.
  for (col = 0; col < W_next->cols; col++) {
    for (row = 0; row < W_next->rows; row++) {
      i = col * W_next->rows + row;
      W_next->elem[i] = (W_next->elem[i] -
                         WZ_squared.elem[row] * W->elem[i]) / Z->cols;
    }
  }

  // Free the memory that we allocated.
  free( WZ_cubed.elem );
  free( WZ_squared.elem );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void negent_gauss( Matrix *W_next, Matrix *W, Matrix *Z )
{
  int row, col, i;
  Matrix WZ, WZ_sqr, WZ_expo;

  // Initialize our matrices.
  WZ_expo.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  WZ_sqr.elem  = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  WZ.elem      = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows * Z->cols );
  WZ_expo.rows = WZ_sqr.rows = WZ.rows = Z->rows;
  WZ_expo.ld   = WZ_sqr.ld   = WZ.ld   = Z->ld;
  WZ_expo.cols = WZ_sqr.cols = WZ.cols = Z->cols;
  WZ_expo.lag  = WZ_sqr.lag  = WZ.lag  = Z->cols;

  GEMM( WZ, *W, *Z );

#ifndef MULTITHREAD_CONTRAST
  // Compute the square and exp( -square / 2 ) of each element in W*Z and then
  // compute the element-wise product of WZ and WZ_expo and the element-wise
  // product of (1 - (W*Z)^2) and WZ_expo.
  for (i = 0; i < WZ.rows * WZ.cols; i++) {
    WZ_sqr.elem[i]  = WZ.elem[i] * WZ.elem[i];
    WZ_expo.elem[i] = exp( -WZ_sqr.elem[i] / 2.0 );
    WZ.elem[i]      = WZ.elem[i] * WZ_expo.elem[i];
    WZ_expo.elem[i] = (1.0 - WZ_sqr.elem[i]) * WZ_expo.elem[i];
  }
#else
  // Do the same thing as the above couple blocks, but use threads for all those
  // independent calculations.
  pthread_attr_t  attr;
  GaussThreadData gdata[NUM_THREADS - 1];
  pthread_t       thread_ids[NUM_THREADS - 1];
  int             block_length;

  pthread_attr_init( &attr );

  block_length = (WZ.rows * WZ.cols) / NUM_THREADS;

  for (i = 0; i < NUM_THREADS - 1; i++) {
    gdata[i].wz      = WZ.elem      + i * block_length;
    gdata[i].wz_sqr  = WZ_sqr.elem  + i * block_length;
    gdata[i].wz_expo = WZ_expo.elem + i * block_length;
    gdata[i].length  = block_length;
  }

  // Multithread the calculation of the tanh().
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_create( &thread_ids[i], &attr, thr_gauss, (void*) &gdata[i] );
  }

  for (i = (NUM_THREADS-1)*block_length; i < WZ.rows * WZ.cols; i++) {
    WZ_sqr.elem[i]  = WZ.elem[i] * WZ.elem[i];
    WZ_expo.elem[i] = exp( -WZ_sqr.elem[i] / 2.0 );
    WZ.elem[i]      = WZ.elem[i] * WZ_expo.elem[i];
    WZ_expo.elem[i] = (1.0 - WZ_sqr.elem[i]) * WZ_expo.elem[i];
  }

  // Wait for threads to complete.
  for (i = 0; i < NUM_THREADS - 1; i++) {
    pthread_join( thread_ids[i], NULL );
  }

  pthread_attr_destroy( &attr );
#endif

  GEMM_NT( *W_next, WZ, *Z );

  // Sum the rows of the WZ_expo matrix in preparation for finding their mean.
  for (col = 1; col < WZ_expo.cols; col++) {
    for (row = 0; row < WZ_expo.rows; row++) {
      WZ_expo.elem[row] += WZ_expo.elem[col*WZ_expo.rows + row];
    }
  }

  // Put everything together.
  for (col = 0; col < W_next->cols; col++) {
    for (row = 0; row < W_next->rows; row++) {
      i = col * W_next->rows + row;
      W_next->elem[i] = (W_next->elem[i] -
                         WZ_expo.elem[row] * W->elem[i]) / WZ.cols;
    }
  }

  // Free allocated memory.
  free( WZ_expo.elem );
  free( WZ_sqr.elem );
  free( WZ.elem );
}

#ifdef MULTITHREAD_CONTRAST
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void *thr_tanh( void *data )
  {
    // Extract the thread data from the given void*.
    TanhThreadData *d  = (TanhThreadData*) data;
    NUMTYPE *array      = d->array;
    int i, length      = d->length;

    // Compute the tanh() of each element.
    for (i = 0; i < length; i++) {
      array[i] = tanh( array[i] );
    }

    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void *thr_tanh_sqr( void *data )
  {
    // Extract the thread data from the given void*.
    TanhThreadData *d  = (TanhThreadData*) data;
    NUMTYPE *array      = d->array;
    int i, length      = d->length;

    // Compute the tanh() of each element.
    for (i = 0; i < length; i++) {
      array[i] = 1.0f - array[i] * array[i];
    }

    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void *thr_cube( void *data )
  {
    // Extract thread data from the given void*.
    CubeThreadData *d = (CubeThreadData*) data;
    NUMTYPE *wz_sqr    = d->wz_sqr;
    NUMTYPE *wz_cube   = d->wz_cube;
    int i, length     = d->length;

    // Do the same thing as the unthreaded version.
    for (i = 0; i < length; i++) {
      wz_sqr[i]  = 3.0 * (wz_cube[i] * wz_cube[i]);
      wz_cube[i] = wz_cube[i] * wz_cube[i] * wz_cube[i];
    }

    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void *thr_gauss( void *data )
  {
    GaussThreadData *d = (GaussThreadData*) data;
    NUMTYPE *wz_expo    = d->wz_expo;
    NUMTYPE *wz_sqr     = d->wz_sqr;
    NUMTYPE *wz         = d->wz;
    int i, length      = d->length;

    // Do the same things as the unthreaded version.
    for (i = 0; i < length; i++) {
      wz_sqr[i]  = wz[i] * wz[i];
      wz_expo[i] = exp( -wz_sqr[i] / 2.0 );

      wz[i]      = wz[i] * wz_expo[i];
      wz_expo[i] = (1.0 - wz_sqr[i]) * wz_expo[i];
    }

    return NULL;
  }
#endif
