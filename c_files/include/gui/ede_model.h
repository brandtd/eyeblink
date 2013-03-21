#ifndef EDE_MODEL_H
#define EDE_MODEL_H

#include "matrix.h"
#include "numtype.h"
#include "ica/ica.h"
#include "blink/params.h"
#include "xltek/x_types.h"

#include <pthread.h>

/**
 * Model information displayed/manipulated by the EDE GUI. There should be a
 * single model per EDE GUI.
 *
 * NOTE: when locking mutices contained in the model, lock from the bottom-up,
 *       i.e., if two mutices are needed, lock the mutex that appears closest
 *       to the bottom of this file first. This should reduce the likelihood of
 *       a resource deadlock.
 *
 *       When unlocking mutices, unlock them in an order that is the reverse of
 *       the order in which they were locked.
 *
 * The model contains the following data:
 *
 *******************************************************************************
 * ICA and blink detection parameters.
 *******************************************************************************
 * ica_params:        The configuration parameters for ICA.
 *
 * ica_lock:          The mutex that must be locked before reading/writing the
 *                    ICA configuration data (i.e., `ica_params').
 *
 * b_params:          Configuration parameters for blink detection.
 *
 * b_lock:            The mutex that must be locked before reading/writing
 *                    `b_params'.
 *
 *
 *******************************************************************************
 * Observation matrix.
 *******************************************************************************
 * mat_X:             Observation matrix used by ICA.
 *
 * mat_head:          The column into which the next set of sample data should
 *                    be written.
 *
 * mat_eeg:           The column in the `eeg_proc' matrix into which the latest
 *                    processed data should be written.
 *
 * mat_lock:          The mutex that must be locked before reading/writing any
 *                    matrix values.
 *
 *******************************************************************************
 * Sample values.
 *******************************************************************************
 * eeg_raw:           The raw, unprocessed EEG sample data. This is an array of
 *                    floating point values that should be treated as a matrix
 *                    stored in column-major order, with `num_channels' rows and
 *                    `eeg_samples' columns.
 *
 * eeg_proc:          Processed EEG--similar to `eeg_raw', but storing processed
 *                    EEG data.
 *
 * eeg_head:          The column containing the most recent sample set in the
 *                    `eeg_raw' and `eeg_proc' matrices. Because we don't want
 *                    to shift the data in the `eeg_raw' matrix everytime a new
 *                    sample set comes in, the `eeg_head' value keeps track of
 *                    where the most recent sample set is located, and
 *                    everytime a new sample set is put into the `eeg_raw'
 *                    matrix, the `eeg_head' index must be incremented (modulo
 *                    `eeg_samples').
 *
 * eeg_samples:       The number of columns in `eeg_raw'.
 * proc_samples:      The number of columns in `eeg_proc'. Should be equal to
 *                    `eeg_samples'.
 *
 * eeg_lock:          The mutex that must be locked before reading/writing any
 *                    sample values (i.e., `eeg_raw', `eeg_proc', `eeg_head',
 *                    `eeg_samples', `proc_samples').
 *
 *******************************************************************************
 * Channel values.
 *******************************************************************************
 * num_channels:      The number of channels in the EEG data. This is also the
 *                    number of rows in the `eeg_proc' and `eeg_raw' matrices.
 *
 * channel_labels:    A list of the EEG channel labels contained by the opened
 *                    ERD file. These labels map directly to the rows in the
 *                    `eeg_*' matrices, so channel_labels[i] is the channel
 *                    label for row `i' in the `eeg_raw' matrix.
 *
 * channel_lock:      The mutex that must be locked before reading/writing any
 *                    channel values (i.e., `num_channels', `channel_labels').
 *
 *******************************************************************************
 * ERD file values.
 *******************************************************************************
 * erd_file:          The ERD file that is currently being read from.
 *
 * erd_lock:          The mutex that must be locked before accessing the
 *                    erd_file.
 */
typedef struct EdeModel {
  ICAParams       ica_params;
  pthread_mutex_t ica_lock;

  BlinkParams     b_params;
  pthread_mutex_t b_lock;

  Matrix          mat_X;
  int             mat_head;
  int             mat_eeg;
  pthread_mutex_t mat_lock;

  NUMTYPE        *eeg_raw;
  NUMTYPE        *eeg_proc;
  int             eeg_head;
  int             eeg_samples;
  int             proc_samples;
  pthread_mutex_t eeg_lock;

  int             num_channels;
  const char    **channel_labels;
  pthread_mutex_t channel_lock;

  xltek_erd_file_t *erd_file;
  pthread_mutex_t   erd_lock;
} EdeModel;

/**
 * Name: ede_model_new
 *
 * Description:
 * Creates a new model for the EDE program. The created model will need to later
 * be destroyed with the ede_model_destory() function.
 *
 * Returns:
 * @return EdeModel*            the newly created model
 */
EdeModel *ede_model_new();

/**
 * Name: ede_model_destroy
 *
 * Description:
 * Destroys the given EdeModel, freeing resources used by the model. After a
 * call to this function, the given model is no longer considered valid and
 * should not be used.
 *
 * Parameters:
 * @param model                 the model to destory
 */
void ede_model_destroy( EdeModel *model );

#endif
