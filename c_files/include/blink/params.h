#ifndef BLINK_PARAMS_H
#define BLINK_PARAMS_H

#define BLINK_DEF_T_1   15
#define BLINK_DEF_T_COR 0.75

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configuration parameters passed to most of the blink functions.
 */
typedef struct BlinkParams {
  NUMTYPE f_s;    // Sample frequency of EEG (Hz).
  NUMTYPE t_1;    // Threshold to use in single channel detection.
  NUMTYPE t_cor;  // Correlation threshold.
} BlinkParams;

#ifdef __cplusplus
}
#endif

#endif
