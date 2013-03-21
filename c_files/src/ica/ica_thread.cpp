#include "ica/ica.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void *icaMainThread( void *data )
{
  ICAThreadData *thr_data = (ICAThreadData*) data;

  // Pull out the stuff from the given data so we don't have to type as much.
  Matrix const *X = thr_data->X;
  Matrix *W       = thr_data->W;
  Matrix *A       = thr_data->A;
  Matrix *S       = thr_data->S;

  NUMTYPE *mu_S = thr_data->mu_S;

  ICAParams const *ica_params = thr_data->ica_params;

  // Initialize the ICA library.
  ica_init( ica_params );

  // Run the ica algorithm.
  ica( W, A, S, mu_S, X );

  // That's it!
  return NULL;
}
