#ifndef BLINK_REMOVE_H
#define BLINK_REMOVE_H

#include "matrix.h"
#include "ica/ica.h"
#include "numtype.h"
#include "blink/params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: blinkRemoveFromEDF
 *
 * Description:
 * Removes eyeblink artifacts from an EEG recording stored in an EDF file. This
 * function will read in the the EEG data contained in the specified EDF file,
 * remove any eyeblinks found, and then store the results in another EDF file.
 *
 * The `ica_params' parameter is not expected to have its `num_var' or `num_obs'
 * fields populated, but all other fields should already be initialized.
 *
 * Parameters:
 * @param input_edf         the EDF file to read for input data
 * @param output_edf        the file in which to store results
 * @param ica_params        configuration parameters for ICA
 */
void blinkRemoveFromEDF( const char *input_edf, const char *output_edf,
                         ICAParams *ica_params );

/**
 * Name: blinkRemove
 *
 * Description:
 * Removes eyeblink artifacts from an EEG recording given as an observation
 * matrix. Each row of the observation matrix should represent a different
 * EEG sensor (e.g., the 'FP1' sensor, the 'F4' sensor), each column should
 * represent an observation of the sensors.
 *
 * The `channels' parameter must be an array containing the channels to use for
 * blink detection. The recommended channels to use are 'FP1 - F3', 'FP1 - F7',
 * 'FP2 - F4', and 'FP2 - F8', but the only requirement is that at least one
 * channel must be given.
 *
 * The `channels' parameter is passed to the blinkDetect() function, so its
 * storage must match that expected by blinkDetect() (i.e., `channels' must be
 * stored in row-major order).
 *
 * The `keep' list specifies which channels in the observation EEG to leave
 * unmodified. This can be used, for example, to prevent the obliteration of
 * blinks from the EOG.
 *
 * Parameters:
 * @param mat_R             where to store the results
 * @param mat_X             the observation matrix
 * @param channels          the channels to use for blink detection
 * @param num_channels      the number of channels
 * @param keep              which channels to keep
 * @param num_keep          the length of the `keep' array
 * @param ica_params        parameters to use for ICA
 * @param b_params          blink detection parameters
 *
 * Returns:
 * @return int              the number of blinks removed
 */
int blinkRemove( Matrix *mat_R, const Matrix *mat_X,
                 const NUMTYPE *channels, int num_channels,
                 const int *keep, int num_keep,
                 ICAParams *ica_params, const BlinkParams *b_params );

#ifdef __cplusplus
}
#endif

#endif
