#include "xltek/erd.h"
#include "gui/ede_model.h"

#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EdeModel *ede_model_new()
{
  EdeModel *model;

  model = (EdeModel*) malloc(sizeof(EdeModel));

  // Initialize ICA parameters to their default values.
  model->ica_params.implem = DEF_IMPLEM;
  model->ica_params.epsilon = DEF_EPSILON;
  model->ica_params.contrast = DEF_CONTRAST;
  model->ica_params.max_iter = DEF_MAX_ITER;
  model->ica_params.num_var = 0;
  model->ica_params.num_obs = 0;
  model->ica_params.use_gpu = 0;
  model->ica_params.gpu_device = DEF_GPU_DEVICE;

  // Initialize the blink parameters to their default values.
  model->b_params.f_s   = 0.0;
  model->b_params.t_1   = BLINK_DEF_T_1;
  model->b_params.t_cor = BLINK_DEF_T_COR;

  // Initialize the matrices.
  model->mat_X.elem = NULL;
  model->mat_X.rows = 0;
  model->mat_X.cols = 0;
  model->mat_X.ld   = 0;
  model->mat_X.lag  = 0;

  // Initialize the samples matrix to a 'NULL' state.
  model->eeg_raw  = NULL;
  model->eeg_proc = NULL;
  model->eeg_head = 0;
  model->eeg_samples = 0;
  model->proc_samples = 0;

  // Initialize the channel labels to a 'NULL' state.
  model->num_channels = 0;
  model->channel_labels = NULL;

  // Initialize the xltek stuff.
  model->erd_file = NULL;

  // Initialize all the mutexs (mutices?).
  pthread_mutex_init( &(model->ica_lock),     NULL );
  pthread_mutex_init( &(model->b_lock),       NULL );
  pthread_mutex_init( &(model->mat_lock),     NULL );
  pthread_mutex_init( &(model->eeg_lock),     NULL );
  pthread_mutex_init( &(model->channel_lock), NULL );
  pthread_mutex_init( &(model->erd_lock),     NULL );

  return model;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_model_destroy( EdeModel *model )
{
  if (model == NULL) {
    fprintf(stderr, "Attempted to free NULL model!\n");
    return;
  }

  // Free memory used for the samples and labels.
  if (model->eeg_raw)  { free(model->eeg_raw);  }
  if (model->eeg_proc) { free(model->eeg_proc); }
  if (model->channel_labels) { free(model->channel_labels); }

  // Free the sample matrices.
  if (model->mat_X.elem) { free(model->mat_X.elem); }

  // Close the ERD file.
  if (model->erd_file) { xltek_closeErdFile( model->erd_file ); }

  // Destroy the mutex objects.
  pthread_mutex_destroy( &(model->ica_lock) );
  pthread_mutex_destroy( &(model->b_lock) );
  pthread_mutex_destroy( &(model->mat_lock) );
  pthread_mutex_destroy( &(model->eeg_lock ) );
  pthread_mutex_destroy( &(model->channel_lock) );
  pthread_mutex_destroy( &(model->erd_lock) );

  // Free the model itself.
  free(model);
}
