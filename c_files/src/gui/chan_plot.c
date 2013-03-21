#include "gui/chan_plot.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define SAMPLES_PER_POINT 10

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GtkChanPlot *chan_plot_new( EdeModel *model, const char *left,
                            const char *right )
{
  GtkChanPlot *chan_plot;       // Struct for our widget.

  GtkWidget *main_hbox;         // Our widget's container.

  GtkWidget *entry_left;        // The two text entry widget's used to select
  GtkWidget *entry_right;       // EEG channels.

  GtkWidget *label;             // A simple label used for decoration.

  GtkWidget *drawarea;          // The area where we plot EEG.

  GtkWidget *but_clamp;         // Button signalling to reset max/min of plot.

  //////////////////////////////////////////////////////////////////////////////
  // Initialize the state of the widget.
  //////////////////////////////////////////////////////////////////////////////
  chan_plot = (GtkChanPlot*) malloc( sizeof(GtkChanPlot) );
  chan_plot->state.model        = model;
  chan_plot->state.left         = -1;
  chan_plot->state.right        = -1;
  chan_plot->state.min          = -1.0;
  chan_plot->state.max          = -1.0;
  chan_plot->state.raw_channel  = NULL;
  chan_plot->state.proc_channel = NULL;
  chan_plot->state.recalc       = 0;
  chan_plot->state.counter      = 0;
  pthread_mutex_init( &(chan_plot->state.channel_lock), NULL );

  //////////////////////////////////////////////////////////////////////////////
  // Create all the widgets.
  //////////////////////////////////////////////////////////////////////////////
  main_hbox = gtk_hbox_new( FALSE, 0 );

  // Setup the EEG channel entry widgets.
  entry_left = gtk_entry_new(); entry_right = gtk_entry_new();
  gtk_entry_set_max_length( GTK_ENTRY(entry_left),  4 );
  gtk_entry_set_max_length( GTK_ENTRY(entry_right), 4 );

  gtk_entry_set_width_chars( GTK_ENTRY(entry_left),  5 );
  gtk_entry_set_width_chars( GTK_ENTRY(entry_right), 5 );

  label = gtk_label_new("-");

  // Setup the drawing/plotting area widget.
  drawarea = gtk_drawing_area_new();
  gtk_widget_set_size_request( drawarea, 1200, 89 );

  but_clamp = gtk_button_new_with_label( "Clamp" );

  // Finish setting up the main widget state container.
  chan_plot->chan_plot   = main_hbox;
  chan_plot->entry_left  = entry_left;
  chan_plot->entry_right = entry_right;

  //////////////////////////////////////////////////////////////////////////////
  // Setup signal handlers.
  //////////////////////////////////////////////////////////////////////////////
  g_signal_connect( drawarea, "expose-event", G_CALLBACK(drawarea_expose_event),
                    chan_plot );
  g_signal_connect( entry_left,  "changed", G_CALLBACK(chan_plot_left_changed),
                    chan_plot );
  g_signal_connect( entry_right, "changed", G_CALLBACK(chan_plot_right_changed),
                    chan_plot );
  g_signal_connect( but_clamp, "clicked", G_CALLBACK(chan_plot_clamp),
                    chan_plot );

  gtk_entry_set_text( GTK_ENTRY(entry_left),  left );
  gtk_entry_set_text( GTK_ENTRY(entry_right), right );

  //////////////////////////////////////////////////////////////////////////////
  // Pack all the widgets into our container.
  //////////////////////////////////////////////////////////////////////////////
  gtk_box_pack_start( GTK_BOX(main_hbox), entry_left,  FALSE, FALSE, 2 );
  gtk_box_pack_start( GTK_BOX(main_hbox), label,       FALSE, FALSE, 2 );
  gtk_box_pack_start( GTK_BOX(main_hbox), entry_right, FALSE, FALSE, 2 );
  gtk_box_pack_start( GTK_BOX(main_hbox), drawarea,    TRUE,  TRUE,  0 );
  gtk_box_pack_start( GTK_BOX(main_hbox), but_clamp,   FALSE, FALSE, 2 );

  return chan_plot;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
gboolean drawarea_expose_event( GtkWidget *widget, GdkEventExpose *event,
                                gpointer data )
{
  int i, k;
  int eeg_head, num_samples, num_raw_points, num_proc_points;

  double block_width, sample_length, scale, offset, lag_x;
  double dashes[] = { 0.0, 4.0 };

  char str[40];

  GtkAllocation alloc;

  cairo_t           *cr        = gdk_cairo_create( widget->window );
  GtkChanPlot       *chan_plot = (GtkChanPlot*) data;
  chan_plot_state_t *state     = &(chan_plot->state);

  gtk_widget_get_allocation( widget, &alloc );

  //////////////////////////////////////////////////////////////////////////////
  // Pull values from state that we will use a lot, and calculate plot sizes.
  //////////////////////////////////////////////////////////////////////////////

  // Lock the channel so that it doesn't get modified while we're plotting it.
  pthread_mutex_lock( &(state->channel_lock) );

  num_proc_points = state->model->proc_samples / SAMPLES_PER_POINT;
  num_raw_points  = state->model->eeg_samples / SAMPLES_PER_POINT;
  eeg_head        = state->model->eeg_head;
  num_samples     = state->model->eeg_samples;

  // The plot will be divided horizontally into 12 blocks--one per second.
  block_width   = ((double) alloc.width) / 12.0;
  sample_length = (double) alloc.width / (double) (num_raw_points - 1);

  // Figure out the scale and offset values we need to plot the channels.
  scale  = (double) alloc.height / (state->min - state->max);
  offset = -scale * state->max;

  //////////////////////////////////////////////////////////////////////////////
  // Setup background of plot.
  //////////////////////////////////////////////////////////////////////////////

  // Draw a white rectangle with a black border--this is the plot's background.
  cairo_set_source_rgb( cr, 1.0, 1.0, 1.0 );
  cairo_rectangle( cr, 0, 0, alloc.width, alloc.height );
  cairo_fill( cr );

  // Signify the time segment where processed EEG is lagging behind the raw
  // EEG by coloring the background differently.
  cairo_set_source_rgb( cr, 0.9, 0.9, 0.9 );
  lag_x = (8.0 / 12.0) * (double) alloc.width;
  cairo_rectangle( cr, lag_x, 0, alloc.width, alloc.height );
  cairo_fill( cr );

  cairo_set_source_rgb( cr, 0.0, 0.0, 0.0 );
  cairo_set_line_width( cr, 1 );
  cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
  cairo_rectangle( cr, 0, 0, alloc.width, alloc.height );
  cairo_stroke( cr );

  cairo_set_line_width( cr, 1 );
  cairo_set_dash( cr, dashes, 2, 0 );
  cairo_set_line_cap( cr, CAIRO_LINE_CAP_ROUND );
  for (i = 1; i <= 11; i++) {
    cairo_move_to( cr, block_width * i, 0.0 );
    cairo_line_to( cr, block_width * i, (double) alloc.height );
    cairo_stroke( cr );
  }

  //////////////////////////////////////////////////////////////////////////////
  // Plot the channels.
  //////////////////////////////////////////////////////////////////////////////

  if (num_raw_points > 0) {
    // Plot the raw samples first.
    cairo_set_line_width( cr, 1 );
    cairo_set_source_rgb( cr, 1.0, 0.0, 0.0 );
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_dash( cr, NULL, 0, 0 );

    k = ((eeg_head - (eeg_head % SAMPLES_PER_POINT)) +
         SAMPLES_PER_POINT) % num_samples;
    cairo_move_to( cr, 0, scale * state->raw_channel[k] + offset );

    for (i = 1; i < num_raw_points; i++) {
      k += SAMPLES_PER_POINT;
      if (k >= num_samples) { k = 0; }

      cairo_line_to( cr, i * sample_length,
                         scale * state->raw_channel[k] + offset );
    }
    cairo_stroke(cr);

    // Now, plot the processed samples.
    cairo_set_line_width( cr, 1 );
    cairo_set_source_rgb( cr, 0.0, 0.0, 1.0 );
    cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT );
    cairo_set_dash( cr, NULL, 0, 0 );

    k = ((eeg_head - (eeg_head % SAMPLES_PER_POINT)) +
         SAMPLES_PER_POINT) % num_samples;
    cairo_move_to( cr, 0, scale * state->proc_channel[k] + offset );

    for (i = 1; i < num_proc_points; i++) {
      k += SAMPLES_PER_POINT;
      if (k >= num_samples) { k = 0; }

      cairo_line_to( cr, i * sample_length,
                         scale * state->proc_channel[k] + offset );
    }
    cairo_stroke(cr);
  }

  // Now that we've plotted, unlock the channel.
  pthread_mutex_unlock( &(state->channel_lock) );

  //////////////////////////////////////////////////////////////////////////////
  // Print the axis limits.
  //////////////////////////////////////////////////////////////////////////////
  cairo_set_font_size( cr, 12 );
  cairo_set_source_rgb( cr, 0.0, 0.0, 0.0 );

  sprintf(str, "%.0f uV", state->max);
  cairo_move_to( cr, 0, 0 + 13.0 );
  cairo_show_text( cr, str );

  sprintf(str, "%.0f uV", state->min);
  cairo_move_to( cr, 0, (double) alloc.height - 2.0 );
  cairo_show_text( cr, str );

  cairo_destroy( cr );

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_left_changed( GtkWidget *widget, gpointer data )
{
  int i, channel = -1;
  GtkChanPlot *chan_plot = (GtkChanPlot*) data;
  chan_plot_state_t *state = &(chan_plot->state);
  const char *label = gtk_entry_get_text( GTK_ENTRY(widget) );
  GdkColor color;

  pthread_mutex_lock( &(state->channel_lock) );
    // Find which channel the label corresponds to.
    for (i = 0; i < state->model->num_channels; i++) {
      if (strcmp(label, state->model->channel_labels[i]) == 0) {
        channel = i;
        break;
      }
    }

    if (channel == -1) {
      // Channel was not found. Set background color to 'not found' color, and
      // set the left index in the state to reflect the 'not found' state.
      state->left = -1;
      color.red   = 0xFFFF; color.green = 0xB000; color.blue  = 0xB000;
    } else {
      // Channel was found.
      state->left = channel;
      color.red   = 0xFFFF; color.green = 0xFFFF; color.blue  = 0xFFFF;
    }
  pthread_mutex_unlock( &(state->channel_lock) );

  gtk_widget_modify_base( widget, GTK_STATE_NORMAL, &color );
  gtk_widget_modify_base( widget, GTK_STATE_ACTIVE, &color );

  // Recalulate the channel values.
  chan_plot_recalcChannel( chan_plot );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_right_changed( GtkWidget *widget, gpointer data )
{
  int i, channel = -1;
  GtkChanPlot *chan_plot = (GtkChanPlot*) data;
  chan_plot_state_t *state = &(chan_plot->state);
  const char *label = gtk_entry_get_text( GTK_ENTRY(widget) );
  GdkColor color;

  pthread_mutex_lock( &(state->channel_lock) );
    // Find which channel the label corresponds to.
    for (i = 0; i < state->model->num_channels; i++) {
      if (strcmp(label, state->model->channel_labels[i]) == 0) {
        channel = i;
        break;
      }
    }

    if (channel == -1) {
      // Channel was not found. Set background color to 'not found' color, and
      // set the right index in the state to reflect the 'not found' state.
      state->right = -1;
      color.red   = 0xFFFF; color.green = 0xB000; color.blue  = 0xB000;
    } else {
      // Channel was found.
      state->right = channel;
      color.red   = 0xFFFF; color.green = 0xFFFF; color.blue  = 0xFFFF;
    }
  pthread_mutex_unlock( &(state->channel_lock) );

  gtk_widget_modify_base( widget, GTK_STATE_NORMAL, &color );
  gtk_widget_modify_base( widget, GTK_STATE_ACTIVE, &color );

  // Recalulate the channel values.
  chan_plot_recalcChannel( chan_plot );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_newSamples( GtkChanPlot *chan_plot, int num_new )
{
  int i, i_eeg, row_l, row_r, rows, cols, replot;
  NUMTYPE val, left, right;
  NUMTYPE *raw_eeg;
  NUMTYPE *proc_eeg;

  // We need to update the raw and processed channel plots.
  pthread_mutex_lock( &(chan_plot->state.channel_lock) );
    raw_eeg  = chan_plot->state.model->eeg_raw;
    proc_eeg = chan_plot->state.model->eeg_proc;

    row_l = chan_plot->state.left;
    row_r = chan_plot->state.right;

    rows = chan_plot->state.model->num_channels;
    cols = chan_plot->state.model->eeg_samples;

    // Calculate the new raw values first.
    for (i = 0; i < num_new; i++) {
      i_eeg = chan_plot->state.model->eeg_head - num_new + i + 1;

      // Make sure the EEG index is within the bounds of the sample array.
      if      (i_eeg < 0)     { i_eeg += cols; }
      else if (i_eeg >= cols) { i_eeg -= cols; }

      if (row_l == -1) { left  = 0.0; }
      else             { left  = raw_eeg[ i_eeg * rows + row_l ]; }

      if (row_r == -1) { right = 0.0; }
      else             { right = raw_eeg[ i_eeg * rows + row_r ]; }

      // Calculate the new value, see if it exceeds any previous bounds, and
      // then record it.
      val = left - right;

      if (val > chan_plot->state.max) { chan_plot->state.max = val; }
      if (val < chan_plot->state.min) { chan_plot->state.min = val; }

      chan_plot->state.raw_channel[i_eeg] = val;
    }

    // Calculate the new processed values.
    for (i = 0; i < num_new; i++) {
      i_eeg = chan_plot->state.model->eeg_head -
              (chan_plot->state.model->eeg_samples -
               chan_plot->state.model->proc_samples) - num_new + i + 1;

      // Make sure the EEG index is within the bounds of the sample array.
      if      (i_eeg < 0)     { i_eeg += cols; }
      else if (i_eeg >= cols) { i_eeg -= cols; }

      if (row_l == -1) { left  = 0.0; }
      else             { left  = proc_eeg[ i_eeg * rows + row_l ]; }

      if (row_r == -1) { right = 0.0; }
      else             { right = proc_eeg[ i_eeg * rows + row_r ]; }

      // Calculate the new value, see if it exceeds any previous bounds, and
      // then record it.
      val = left - right;

      if (val > chan_plot->state.max) { chan_plot->state.max = val; }
      if (val < chan_plot->state.min) { chan_plot->state.min = val; }

      chan_plot->state.proc_channel[i_eeg] = val;
    }

    // If enough new samples have been gathered, replot the channel.
    if (chan_plot->state.counter++ >= SAMPLES_PER_POINT) {
      chan_plot->state.counter = 0;
      replot = 1;
    } else {
      replot = 0;
    }
  pthread_mutex_unlock( &(chan_plot->state.channel_lock) );

  if (replot) {
    gtk_widget_queue_draw( chan_plot->chan_plot );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_reinitialize( GtkChanPlot *chan_plot )
{
  chan_plot_state_t *state = &(chan_plot->state);
  EdeModel *model = state->model;

  // Lock the channel before modifying it.
  pthread_mutex_lock( &(state->channel_lock) );
    // Set the state to 'zero' state.
    if (state->raw_channel)  { free( state->raw_channel );  }
    if (state->proc_channel) { free( state->proc_channel ); }

    state->max =  1.0;
    state->min = -1.0;

    state->raw_channel  = (NUMTYPE*)malloc(sizeof(NUMTYPE)* model->eeg_samples);
    state->proc_channel = (NUMTYPE*)malloc(sizeof(NUMTYPE)* model->eeg_samples);

    state->left  = -1;
    state->right = -1;

  pthread_mutex_unlock( &(state->channel_lock) );

  // Get the left and right channel indices to figure themselves out.
  state->recalc = 0; // Turn off channel recalculation for the moment.
  chan_plot_right_changed( chan_plot->entry_right, chan_plot );
  chan_plot_left_changed(  chan_plot->entry_left,  chan_plot );
  state->recalc = 1; // Reenable channel recalculation.

  // Recalculate the channel.
  chan_plot_recalcChannel( chan_plot );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_replot( GtkChanPlot *chan_plot )
{
  chan_plot_recalcChannel( chan_plot );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_recalcChannel( GtkChanPlot *chan_plot )
{
  int i, row_l, row_r, rows, cols;
  NUMTYPE val, left, right;

  chan_plot_state_t *state = &(chan_plot->state);
  EdeModel *model = state->model;

  // Only recalculate if we're supposed to.
  if (!state->recalc) {
    return;
  }

  // Lock the channel again while we calculate the channel values.
  pthread_mutex_lock( &(state->channel_lock) );
    row_l = state->left; row_r = state->right;
    rows  = model->num_channels;
    cols  = model->eeg_samples;

    for (i = 0; i < cols; i++) {
      // First, calculate the raw channel values.
      if (row_l == -1) { left  = 0.0; }
      else             { left  = model->eeg_raw[ i * rows + row_l ]; }

      if (row_r == -1) { right = 0.0; }
      else             { right = model->eeg_raw[ i * rows + row_r ]; }

      val = left - right;

      if (val > state->max) { state->max = val; }
      if (val < state->min) { state->min = val; }

      state->raw_channel[i] = val;

      // Next, calculate the processed channel values.
      if (row_l == -1) { left  = 0.0; }
      else             { left  = model->eeg_proc[ i * rows + row_l ]; }

      if (row_r == -1) { right = 0.0; }
      else             { right = model->eeg_proc[ i * rows + row_r ]; }

      state->proc_channel[i] = left - right;
    }
  pthread_mutex_unlock( &(state->channel_lock) );

  // Make the channel plot redraw itself.
 gtk_widget_queue_draw( chan_plot->chan_plot );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void chan_plot_clamp( GtkWidget *widget, gpointer data )
{
  int i;
  NUMTYPE max = -INFINITY, min = INFINITY;
  GtkChanPlot *chan_plot = (GtkChanPlot*) data;
  chan_plot_state_t *state = &(chan_plot->state);

  // Lock the channel lock before attempting to read/write channel values.
  pthread_mutex_lock( &(state->channel_lock) );
    // Check for max/mins in the raw data only.
    for (i = 0; i < state->model->eeg_samples; i++) {
      if (max < state->raw_channel[i])  { max = state->raw_channel[i]; }
      if (min > state->raw_channel[i])  { min = state->raw_channel[i]; }
    }

    // Set the max/min values.
    state->max = max; state->min = min;
  pthread_mutex_unlock( &(state->channel_lock) );

  // Make the channel plot redraw itself.
  gtk_widget_queue_draw( chan_plot->chan_plot );
}
