#include "gui/ede_window.h"
#include "gui/blink_control.h"

#include "ica/ica.h"

#include "xltek/xltek.h"

#include "blink/params.h"
#include "blink/remove.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GtkEdeWindow *ede_window_new( EdeModel *model )
{
  //////////////////////////////////////////////////////////////////////////////
  // All the variables we'll use.
  //////////////////////////////////////////////////////////////////////////////
  GtkEdeWindow *myself;

  // The containers for the main window.
  GtkWidget *window;
  GtkWidget *main_vbox;
  GtkWidget *plot_hbox;
  GtkWidget *file_hbox;

  // The widgets that will be packed into the main window.
  GtkWidget *blink_controls;
  GtkWidget *v_separator;
  GtkWidget *file_label;
  GtkWidget *file_button;
  GtkWidget *openfile_label;
  GtkWidget *filename_label;
  GtkWidget *play_image;
  GtkWidget *pause_image;
  GtkWidget *but_play_pause;

  GtkEegPlot *eeg_plot;

  //////////////////////////////////////////////////////////////////////////////
  // Create all the widgets that we need.
  //////////////////////////////////////////////////////////////////////////////
  myself = (GtkEdeWindow*) malloc( sizeof(GtkEdeWindow) );

  // Main window.
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  gtk_window_set_title( GTK_WINDOW( window ),
                        "Eye-blink Detector & Eliminator");
  gtk_container_set_border_width( GTK_CONTAINER(window), 10 );

  // Main containers.
  main_vbox = gtk_vbox_new( FALSE, 10 );
  plot_hbox = gtk_hbox_new( FALSE, 5 );
  file_hbox = gtk_hbox_new( FALSE, 5 );

  // Channel plotter.
  eeg_plot = eeg_plot_new( model );

  // Separator placed between plotter and ICA controls.
  v_separator = gtk_vseparator_new();

  // ICA control widget.
  blink_controls = blink_control_new( model );

  // File selector widgets.
  file_label  = gtk_label_new( "Open ERD File: " );
  file_button = gtk_file_chooser_button_new( "Select ERD File",
                                             GTK_FILE_CHOOSER_ACTION_OPEN );
  openfile_label = gtk_label_new( "Current file: " );
  filename_label = gtk_label_new( "--" );

  // EEG play/pause button and an empty filler element.
  play_image  = gtk_image_new_from_stock( GTK_STOCK_MEDIA_PLAY,
                                          GTK_ICON_SIZE_SMALL_TOOLBAR );
  pause_image = gtk_image_new_from_stock( GTK_STOCK_MEDIA_PAUSE,
                                          GTK_ICON_SIZE_SMALL_TOOLBAR );
  but_play_pause = gtk_button_new();
  gtk_container_add( GTK_CONTAINER(but_play_pause), play_image );

  //////////////////////////////////////////////////////////////////////////////
  // Setup signal handlers.
  //////////////////////////////////////////////////////////////////////////////
  g_signal_connect( file_button, "file-set", G_CALLBACK(ede_window_setErdFile),
                    myself );
  g_signal_connect( but_play_pause,"clicked", G_CALLBACK(ede_window_play_pause),
                    myself );

  //////////////////////////////////////////////////////////////////////////////
  // Pack all the widgets into the main window.
  //////////////////////////////////////////////////////////////////////////////

  // Plot stuff gets packed into the plot HBOX, which then gets placed at the
  // top of the main VBOX.
  gtk_box_pack_start( GTK_BOX(plot_hbox), blink_controls, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(plot_hbox), v_separator,    FALSE, FALSE, 10 );
  gtk_box_pack_start( GTK_BOX(plot_hbox), eeg_plot->eeg_plot, TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), plot_hbox,          TRUE, TRUE, 0 );

  // File selector widgets go in the bottom of the main VBOX.
  gtk_box_pack_start( GTK_BOX(file_hbox), file_label,  FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(file_hbox), file_button, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(file_hbox), openfile_label, FALSE, FALSE, 10 );
  gtk_box_pack_start( GTK_BOX(file_hbox), filename_label, FALSE, FALSE, 0 );
  gtk_box_pack_end(   GTK_BOX(file_hbox), but_play_pause, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), file_hbox,   FALSE, FALSE, 0 );

  gtk_container_add( GTK_CONTAINER(window), main_vbox );

  //////////////////////////////////////////////////////////////////////////////
  // Setup the return value.
  //////////////////////////////////////////////////////////////////////////////
  myself->ede_window     = window;
  myself->filename_label = filename_label;
  myself->but_play_pause = but_play_pause;
  myself->eeg_plot       = eeg_plot;

  myself->play_image     = play_image;
  myself->pause_image    = pause_image;

  myself->model          = model;

  myself->playing = 0;

  pthread_mutex_init( &(myself->file_lock), NULL );
  pthread_mutex_init( &(myself->ica_lock), NULL );

  myself->ica_cancel = 0;

  myself->samples = NULL;

  //////////////////////////////////////////////////////////////////////////////
  // Start up a timer that causes replotting of the channel plots at a regular
  // interval.
  //////////////////////////////////////////////////////////////////////////////
  g_timeout_add( 300, ede_redrawTimer, myself );

  return myself;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
gboolean ede_redrawTimer( void *ede_window )
{
  gdk_threads_enter();
  GtkEdeWindow *myself = (GtkEdeWindow*) ede_window;
  eeg_plot_replot( myself->eeg_plot );
  gdk_threads_leave();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_window_newSamples( GtkEdeWindow *ede_window, int num_new )
{
  eeg_plot_newSamples( ede_window->eeg_plot, num_new );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_window_reinitialize( GtkEdeWindow *ede_window )
{
  eeg_plot_reinitialize( ede_window->eeg_plot );
  ede_window_setPlay( ede_window, 0 );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_window_setErdFile( GtkWidget *widget, gpointer data )
{
  int i, k_l, k_r, row, col, num_channels, elems;
  int sec_8, sec_10, sec_12, sec_16;
  const char **channel_labels;
  Matrix mat_R;

  NUMTYPE *channels;

  GtkEdeWindow *myself = (GtkEdeWindow*) data;
  EdeModel *model = myself->model;

  char *filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(widget) );

  // Open the file.
  xltek_erd_file_t *file = xltek_openErdFile( filename );
  if (file == NULL) {
    gtk_label_set_text( GTK_LABEL(myself->filename_label),
                        "Failed to open file!" );
    return;
  }

  // Wait for ICA thread to stop if one's already started.
  myself->ica_cancel = 1;
  if (pthread_mutex_trylock( &(myself->ica_lock) ) == 0) {
    pthread_mutex_unlock( &(myself->ica_lock) );
  } else {
    pthread_join( myself->ica_thread, NULL );
  }

  //////////////////////////////////////////////////////////////////////////////
  // Lock all the mutices, since we'll be modifying almost the entire model.
  //////////////////////////////////////////////////////////////////////////////
  pthread_mutex_lock( &(myself->file_lock) );
  pthread_mutex_lock( &(model->erd_lock) );
  pthread_mutex_lock( &(model->channel_lock) );
  pthread_mutex_lock( &(model->eeg_lock) );
  pthread_mutex_lock( &(model->mat_lock) );
  pthread_mutex_lock( &(model->b_lock) );

  //////////////////////////////////////////////////////////////////////////////
  // Allocate memory.
  //////////////////////////////////////////////////////////////////////////////

  // Close any open ERD file and remember the new file.
  if (model->erd_file) {
    xltek_closeErdFile( model->erd_file );
  }
  model->erd_file = file;

  // Recompute the blink detection parameters.
  model->b_params.f_s   = (NUMTYPE) xltek_getErdSampleFreq( file );
  model->b_params.t_1   = BLINK_DEF_T_1;
  model->b_params.t_cor = BLINK_DEF_T_COR;

  // Get the new channel information.
  if (model->channel_labels) {
    free(model->channel_labels);
  }
  channel_labels        = xltek_getErdLabels( file );
  num_channels          = xltek_getErdNumChannels( file );
  model->channel_labels = channel_labels;
  model->num_channels   = num_channels;

  // Figure out the size of the `channels' and observation matrices. ICA will
  // only ever be performed on 10 seconds worth of data, but we provide a
  // buffer for 16 seconds worth, allowing the ICA algorithm 3 seconds to
  // compute.
  sec_16       = (int) (16.0 * round(model->b_params.f_s));
  sec_10       = (int) (10.0 * round(model->b_params.f_s));
  elems        = num_channels * sec_16;

  // Free any memory that was already allocated.
  if (myself->samples)   { free( myself->samples ); }
  if (model->mat_X.elem) { free( model->mat_X.elem ); }

  myself->samples  = (double*)  malloc( sizeof(double) * num_channels );
  channels         = (NUMTYPE*) malloc( sizeof(NUMTYPE) * 4 * sec_10 ); 

  model->mat_X.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * elems );
  mat_R.elem        = (NUMTYPE*) malloc( sizeof(NUMTYPE) * elems );
  model->mat_head   = sec_10;

  mat_R.rows = model->mat_X.rows = num_channels;
  mat_R.cols = model->mat_X.cols = sec_10;
  mat_R.ld   = model->mat_X.ld   = num_channels;
  mat_R.lag  = model->mat_X.lag  = sec_16;

  // The model plots 12 seconds of raw data, and 8 seconds of processed data.
  sec_12 = (int) (12.0 * round(model->b_params.f_s));
  sec_8  = (int) ( 8.0 * round(model->b_params.f_s));
  elems  = num_channels * sec_12;

  model->eeg_raw  = (NUMTYPE*) realloc(model->eeg_raw,  sizeof(NUMTYPE)*elems);
  model->eeg_proc = (NUMTYPE*) realloc(model->eeg_proc, sizeof(NUMTYPE)*elems);
  model->eeg_samples  = sec_12;
  model->proc_samples = sec_8;

  //////////////////////////////////////////////////////////////////////////////
  // Initialize the observation/results matrices for first 10 seconds of file.
  //////////////////////////////////////////////////////////////////////////////

  // Zero out the raw/processed EEG.
  for (i = 0; i < elems; i++) {
    model->eeg_proc[i] = 0.0;
    model->eeg_raw[i]  = 0.0;
  }

  // Read in ten seconds of data to start everything off.
for(col = 0; col < 500 * 10; col++) {
xltek_getNextSamples( model->erd_file, myself->samples );
}
  for (col = 0; col < model->mat_X.cols; col++) {

    xltek_getNextSamples( model->erd_file, myself->samples );
    for (row = 0; row < num_channels; row++) {
      model->mat_X.elem[ col * model->mat_X.ld + row ] = myself->samples[row];
      model->eeg_raw[ col * num_channels + row ]       = myself->samples[row];
    }
  }

  // Setup the head index to point to the last sample value.
  model->eeg_head = model->mat_X.cols - 1;
  model->mat_eeg  = model->mat_X.cols - 1;

  //////////////////////////////////////////////////////////////////////////////
  // Perform blink removal on the first ten seconds.
  //////////////////////////////////////////////////////////////////////////////

  // Figure out the channel indices for the FP1, FP2, F3, F7, F4, and F8
  // channels, since those are what we use to perform blink detection. We're
  // also looking for the LEOG and REOG channels (XXX: I think these are the
  // AUX1 and AUX2 channels).
  for (i = 0; i < num_channels; i++) {
    if      (strcmp("FP1",channel_labels[i]) == 0) {myself->channel_ids[0] = i;
                                                    myself->channel_ids[2] = i;}
    else if (strcmp("FP2",channel_labels[i]) == 0) {myself->channel_ids[4] = i;
                                                    myself->channel_ids[6] = i;}
    else if (strcmp("F3", channel_labels[i]) == 0) {myself->channel_ids[1] = i;}
    else if (strcmp("F4", channel_labels[i]) == 0) {myself->channel_ids[5] = i;}
    else if (strcmp("F7", channel_labels[i]) == 0) {myself->channel_ids[3] = i;}
    else if (strcmp("F8", channel_labels[i]) == 0) {myself->channel_ids[7] = i;}
    else if (strcmp("AUX1", channel_labels[i] ) == 0) {myself->eog_ids[0] = i;}
    else if (strcmp("AUX2", channel_labels[i] ) == 0) {myself->eog_ids[1] = i;}
  }

  // Calculate the channels (stored in row-major order).
  for (row = 0; row < 4; row++) {
    for (col = 0; col < model->mat_X.cols; col++) {
      k_l = col * model->mat_X.ld + myself->channel_ids[row * 2];
      k_r = col * model->mat_X.ld + myself->channel_ids[row * 2 + 1];

      channels[row * model->mat_X.cols + col] = model->mat_X.elem[k_l] -
                                                model->mat_X.elem[k_r];
    }
  }

  // Call the magic function.
  blinkRemove( &(mat_R), &(model->mat_X),
               channels,        4,
               myself->eog_ids, 2,
               &(model->ica_params), &(model->b_params) );
  
  // Save the processed EEG data.
  for (col = 0; col < model->proc_samples; col++) {
    for (row = 0; row < num_channels; row++) {
      i = col * num_channels + row;
      model->eeg_proc[i] = (NUMTYPE) mat_R.elem[i];
    }
  }

  // Start up the ICA thread.
  myself->ica_cancel = 0;
  pthread_create( &(myself->ica_thread), NULL, ede_processEeg, (void*) myself );

  //////////////////////////////////////////////////////////////////////////////
  // Unlock mutices in the reverse of the order in which they were locked.
  //////////////////////////////////////////////////////////////////////////////
  pthread_mutex_unlock( &(model->b_lock) );
  pthread_mutex_unlock( &(model->mat_lock) );
  pthread_mutex_unlock( &(model->eeg_lock) );
  pthread_mutex_unlock( &(model->channel_lock) );
  pthread_mutex_unlock( &(model->erd_lock) );
  pthread_mutex_unlock( &(myself->file_lock) );

  gtk_label_set_text( GTK_LABEL(myself->filename_label), filename );
  ede_window_reinitialize( myself );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_window_play_pause( GtkWidget *widget, gpointer data )
{
  GtkEdeWindow *myself = (GtkEdeWindow*) data;
  ede_window_setPlay( myself, !myself->playing );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ede_window_setPlay( GtkEdeWindow *window, int playing )
{
  if (window->playing && !playing) {
    g_object_ref( window->pause_image );
    gtk_container_remove( GTK_CONTAINER(window->but_play_pause),
                          window->pause_image );
    gtk_container_add( GTK_CONTAINER( window->but_play_pause ),
                       window->play_image );
    gtk_widget_show_all( window->but_play_pause );

    window->playing = 0;
  } else if (!window->playing && playing && window->model->erd_file) {
    g_object_ref( window->play_image );
    gtk_container_remove( GTK_CONTAINER(window->but_play_pause),
                          window->play_image );
    gtk_container_add( GTK_CONTAINER( window->but_play_pause ),
                       window->pause_image );
    gtk_widget_show_all( window->but_play_pause );

    g_timeout_add( (guint) (1000.0 / window->model->b_params.f_s),
                   ede_fileReadTimer, window );

    window->playing = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
gboolean ede_fileReadTimer( void *data )
{
  gdk_threads_enter();

  int row, col, k;
  GtkEdeWindow *myself = (GtkEdeWindow*) data;
  EdeModel *model = myself->model;
  gboolean retval = TRUE;

  // Make sure stuff accessed by this timer cannot be modified while it's being
  // used.
  pthread_mutex_lock( &(myself->file_lock) );
  pthread_mutex_lock( &(model->eeg_lock) );
  pthread_mutex_lock( &(model->mat_lock) );
    // Read in a new set of samples.
    if (!xltek_getNextSamples( model->erd_file, myself->samples )) {
      retval = FALSE;
      ede_window_setPlay( myself, 0 );
    } else if (!myself->playing) {
      retval = FALSE;
    } else {
      // Store the samples in the model's observation matrix.
      for (row = 0; row < model->mat_X.rows; row++) {
        model->mat_X.elem[ model->mat_head * model->mat_X.ld + row ] =
                                                          myself->samples[row];
      }
      model->mat_head++;  // Should never overflow (see: ede_processEeg).

      // Copy the samples over to the EDE model.
      col  = (model->eeg_head + 1) % model->eeg_samples;

      for (row = 0; row < model->num_channels; row++) {
        k = col * model->num_channels + row;
        model->eeg_raw[k] = myself->samples[row];
      }
      model->eeg_head = col;

      ede_window_newSamples( myself, 1 );
    }

  pthread_mutex_unlock( &(model->mat_lock) );
  pthread_mutex_unlock( &(model->eeg_lock) );
  pthread_mutex_unlock( &(myself->file_lock) );

  gdk_threads_leave();

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void *ede_processEeg( void *data )
{
  int k_l, k_r, row, col, mat_col, eeg_col, col_shift, cpy_cols;
  NUMTYPE *channels;

  GtkEdeWindow *myself = (GtkEdeWindow*) data;
  EdeModel *model = myself->model;

  // Local copies of model parameters.
  Matrix      mat_X, mat_R;
  ICAParams   ica_params;
  BlinkParams b_params;

  // Lock the mutex that lets us know the ica thread is running.
  pthread_mutex_lock( &(myself->ica_lock) );

  // Initialize the local observation matrix.
  pthread_mutex_lock( &(model->mat_lock) );
    mat_R.rows = mat_X.rows = model->mat_X.rows;
    mat_R.cols = mat_X.cols = model->mat_X.cols;
    mat_R.ld   = mat_X.ld   = model->mat_X.ld;
    mat_R.lag  = mat_X.lag  = model->mat_X.cols;
  pthread_mutex_unlock( &(model->mat_lock) );

  mat_X.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * mat_X.ld * mat_X.cols );
  mat_R.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * mat_R.ld * mat_R.cols );

  channels = (NUMTYPE*) malloc( sizeof(NUMTYPE) * 4 * mat_X.cols );

  // Keep on processing until we're told to cancel.
  while (!(myself->ica_cancel)) {

    ////////////////////////////////////////////////////////////////////////////
    // Copy model values to local variables so that locks can be released.
    ////////////////////////////////////////////////////////////////////////////
    pthread_mutex_lock( &(model->eeg_lock) );
    pthread_mutex_lock( &(model->mat_lock) );
    pthread_mutex_lock( &(model->b_lock) );
    pthread_mutex_lock( &(model->ica_lock) );

      memcpy( &ica_params, &(model->ica_params), sizeof(ICAParams) );
      memcpy( &b_params,   &(model->b_params),   sizeof(BlinkParams) );
      memcpy( mat_X.elem, model->mat_X.elem,
              sizeof(NUMTYPE) * mat_X.ld * mat_X.cols );

    pthread_mutex_unlock( &(model->ica_lock) );
    pthread_mutex_unlock( &(model->b_lock) );
    pthread_mutex_unlock( &(model->mat_lock) );
    pthread_mutex_unlock( &(model->eeg_lock) );

    ////////////////////////////////////////////////////////////////////////////
    // Calculate the channels (stored in row-major order) for blink detection.
    ////////////////////////////////////////////////////////////////////////////
    for (row = 0; row < 4; row++) {
      for (col = 0; col < mat_X.cols; col++) {
        k_l = col * mat_X.ld + myself->channel_ids[row * 2];
        k_r = col * mat_X.ld + myself->channel_ids[row * 2 + 1];

        channels[row * mat_X.cols + col] = mat_X.elem[k_l] - mat_X.elem[k_r];
      }
    }

    // Call the magic function.
    blinkRemove( &(mat_R), &(mat_X), channels, 4, myself->eog_ids, 2,
                 &ica_params, &b_params );

    // Save the processed EEG data and shift the observation matrix.
    pthread_mutex_lock( &(model->eeg_lock) );
    pthread_mutex_lock( &(model->mat_lock) );
      // If we're currently scrolling EEG, then only save the last 2 seconds,
      // otherwise, save the whole EEG.
      if (myself->playing) {
        cpy_cols = (int) (2.0 * round(b_params.f_s));
      } else {
        cpy_cols = mat_X.cols;
      }

      // The two seconds that we save are the two seconds directly before where
      // the observations were when we started processing.
      for (col = 0; col < cpy_cols; col++) {

        // Figure out the column indices of the result matrix we will be copying
        // from, and the sample matrix that we will be copying to.
        mat_col = mat_X.cols -     cpy_cols + col;
        eeg_col = model->mat_eeg - cpy_cols + col;

        if (eeg_col < 0) { eeg_col += model->eeg_samples; }

        // Perform the actual copy.
        memcpy( model->eeg_proc + eeg_col * mat_R.rows,
                mat_R.elem      + mat_col * mat_R.ld,
                sizeof(NUMTYPE) * mat_R.rows );
      }

      // Shift the observations.
      col_shift = model->mat_head - mat_X.cols;
      memmove( model->mat_X.elem, model->mat_X.elem + col_shift * mat_X.ld,
               sizeof(NUMTYPE) * mat_X.cols * mat_X.ld );

      model->mat_head = mat_X.cols;
      model->mat_eeg  = model->eeg_head;

    pthread_mutex_unlock( &(model->mat_lock) );
    pthread_mutex_unlock( &(model->eeg_lock) );
  }

  // Unlock the thread's mutex to signal that the thread is exiting.
  pthread_mutex_unlock( &(myself->ica_lock) );

  return NULL;
}
