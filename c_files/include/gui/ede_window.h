#ifndef EB_WINDOW_H
#define EB_WINDOW_H

#include "gui/eeg_plot.h"
#include "gui/ede_model.h"

#include <pthread.h>
#include <gtk/gtk.h>

/**
 * Struct containing EDE window widget stuff.
 *
 * TODO: the correct thing to do would be to create a new widget, but I don't
 *       know how to do that, and this hack will work for now.
 */
typedef struct GtkEdeWindow {
  GtkWidget *ede_window;        // Widget stuff.
  GtkWidget *filename_label;
  GtkWidget *but_play_pause;
  GtkEegPlot *eeg_plot;

  GtkWidget *play_image;
  GtkWidget *pause_image;

  EdeModel *model;              // The EDE model.

  int playing;                  // Whether or not data is currently 'playing'.

  int channel_ids[8];           // The channel IDs used for blink detection.
  int eog_ids[2];               // The channel IDs unmodified by blink removal.

  pthread_mutex_t file_lock;    // Mutex for reading data from file.
  pthread_mutex_t ica_lock;     // Locked while ICA is running.
  pthread_t       ica_thread;   // Thread performing ICA.
  int             ica_cancel;   // Used to signal ICA thread to exit.

  double *samples;              // Array used to read in sample values.
} GtkEdeWindow;

/**
 * Name: ede_window_new
 *
 * Description:
 * Creates a new top-level window for the eyeblink detection/removal program.
 * The created window does not 'show' any of its contained widgets, so it is up
 * to the caller to use gtk_widget_show_all() on the returned top-level window.
 *
 * Parameters:
 * @param model               the EDE model data, fresh from ede_model_new()
 *
 * Returns:
 * @return GtkEdeWindow*      the top-level window for the eyeblink program
 */
GtkEdeWindow *ede_window_new( EdeModel *model );

/**
 * Name: ede_redrawTimer
 *
 * Description:
 * Causes the EDE window to replot its channels at a regular interval.
 *
 * Parameters:
 * @param ede_window          the EDE window to replot
 *
 * Returns:
 * @return gboolean           always TRUE
 */
gboolean ede_redrawTimer( void *ede_window );

/**
 * Name: ede_window_newSamples
 *
 * Description:
 * Updates the state of the EDE window with new sample values.
 *
 * Parameters:
 * @param ede_window          the ede window to update
 * @param num_new             the number of new samples
 */
void ede_window_newSamples( GtkEdeWindow *ede_window, int num_new );

/**
 * Name: ede_window_reinitialize
 *
 * Description:
 * Signals to the EDE window that it needs to reinitialize itself because the
 * EDE model has changed. This function needs to be called when a new ERD file
 * is loaded.
 *
 * Parameters:
 * @param ede_window          the channel plot to reinitialize
 */
void ede_window_reinitialize( GtkEdeWindow *ede_window );

/**
 * Name: ede_window_setErdFile
 *
 * Description:
 * Signal handler for the file chooser dialog. Changes the ERD file that is
 * being read.
 *
 * Parameters:
 * @param widget              the file chooser button widget
 * @param data                the GtkEdeWindow object
 */
void ede_window_setErdFile( GtkWidget *widget, gpointer data );

/**
 * Name: ede_window_play_pause
 *
 * Description:
 * Signal handler for the play/pause button. Will either pause or resume the
 * playback of EEG data.
 *
 * Parameters:
 * @param widget              the button that was clicked
 * @param data                the GtkEdeWindow object
 */
void ede_window_play_pause( GtkWidget *widget, gpointer data );

/**
 * Name: ede_window_setPlay
 *
 * Description:
 * Sets the 'playing' state of the window.
 *
 * Parameters:
 * @param window              the EDE window on which to operate
 * @param playing             zero to pause, nonzero to play
 */
void ede_window_setPlay( GtkEdeWindow *window, int playing );

/**
 * Name: ede_fileReadTimer
 *
 * Description:
 * A timer function that handles reading in Xltek ERD file samples at a regular
 * interval.
 *
 * This function is responsible for two things:
 *
 * 1) Keeping filled the observation matrix that gets processed by the ICA
 *    program.
 * 2) Pumping new sample values to the GUI widgets to display on the screen.
 *
 * The timer will stop once it reaches the end of the ERD file.
 *
 * Parameters:
 * @param ede_window        the EDE window on which to operate
 *
 * Returns:
 * @return gboolean         TRUE to keep timer going, FALSE to stop timer
 */
gboolean ede_fileReadTimer( void *ede_window );

/**
 * Name: ede_processEeg
 *
 * Description:
 * Run in a separate thread from the main GUI, this function serves as an entry
 * point for a thread that will continually perform ICA on an observation
 * matrix.
 *
 * This function (thread) is responsible for three things:
 *
 * 1) Perform blink removal on the model's observation matrix.
 * 2) Copy the last two seconds of the resulting data to the `processed eeg'
 *    field in the EDE model.
 * 3) Shift the model's observation matrix to remove data more than 10 seconds
 *    old.
 *
 * Parameters:
 * @param ede_window        the EDE window on which to operate
 *
 * Returns:
 * @return NULL
 */
void *ede_processEeg( void *ede_window );

#endif
