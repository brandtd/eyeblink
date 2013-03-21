#ifndef CHAN_PLOT_H
#define CHAN_PLOT_H

#include "numtype.h"
#include "gui/ede_model.h"

#include <gtk/gtk.h>

/**
 * Struct used to store state of a channel plot.
 */
typedef struct chan_plot_state_t {
  EdeModel *model;              // The EDE model.

  int left;                     // The 'left' channel index.
  int right;                    // The 'right' channel index.

  double max;                   // Maximum plotted value.
  double min;                   // Minimum plotted value.

  NUMTYPE *raw_channel;         // The raw channel values to plot.
  NUMTYPE *proc_channel;        // The processed channel values to plot.

  int recalc;                   // If not set, don't recalculate the channel.

  int counter;                  // A counter used to determine if a replot is
                                // necessary.

  pthread_mutex_t channel_lock; // Lock for accessing the samples arrays.
} chan_plot_state_t;

/**
 * Struct containing channel plot widget stuff.
 *
 * TODO: the correct thing to do would be to create a new widget, but I don't
 *       know how to do that, and this hack will work for now.
 */
typedef struct GtkChanPlot {
  GtkWidget *chan_plot;
  GtkWidget *entry_left;
  GtkWidget *entry_right;
  chan_plot_state_t state;
} GtkChanPlot;

/**
 * Name: chan_plot_new
 *
 * Description:
 * Creates a new channel plot widget. This widget provides a means for plotting
 * a single EEG channel.
 *
 * Parameters:
 * @param model               the EDE model data
 * @param left                what to intialize the 'left' plot to
 * @param right               what to intialize the 'right' plot to
 *
 * Returns:
 * @return GtkChanPlot*       the channel plot widget and state
 */
GtkChanPlot *chan_plot_new( EdeModel *model, const char *left,
                            const char *right );

/**
 * Name: chan_plot_newSamples
 *
 * Description:
 * Updates the state of the channel plot with new sample values.
 *
 * Parameters:
 * @param chan_plot           the channel plot to update
 * @param num_new             the number of new samples
 */
void chan_plot_newSamples( GtkChanPlot *chan_plot, int num_new );

/**
 * Name: chan_plot_reinitialize
 *
 * Description:
 * Signals to the channel plot that it needs to reinitialize itself because
 * the EDE model has changed. This function needs to be called when a new ERD
 * file is loaded.
 *
 * Parameters:
 * @param chan_plot           the channel plot to reinitialize
 */
void chan_plot_reinitialize( GtkChanPlot *chan_plot );

/**
 * Name: chan_plot_replot
 *
 * Description:
 * Forces channel plot to recalcuate and replot its values.
 *
 * Parameters:
 * @param chan_plot           the channel plot to replot
 */
void chan_plot_replot( GtkChanPlot *chan_plot );

/*******************************************************************************
 * Implementation functions that only the GtkChanPlot widget should have to
 * know about (consider these 'private' functions).
 ******************************************************************************/

/**
 * Name: chan_plot_recalcChannel
 *
 * Description:
 * Forces recalculation of the channel value arrays
 *
 * Parameters:
 * @param chan_plot           the channel plot to recalculate
 */
void chan_plot_recalcChannel( GtkChanPlot *chan_plot );

/**
 * Name: drawarea_expose_event
 *
 * Description:
 * Signal handler for an 'expose' event used by the plotting area of a channel
 * plot.
 *
 * Parameters:
 * @param widget              the widget that was 'exposed'
 * @param event               unused, required for callback
 * @param data                the channel plot
 *
 * Returns:
 * @return gboolean           returns FALSE
 */
gboolean drawarea_expose_event( GtkWidget *widget, GdkEventExpose *event,
                                gpointer data );

/**
 * Name: chan_plot_clamp
 *
 * Description:
 * Signal handler for the 'clamp' button on a channel plot. Recalculates the
 * maximum and minimum values of a channel plot.
 *
 * Parameters:
 * @param widget              the widget that was clicked
 * @param data                the channel plot
 */
void chan_plot_clamp( GtkWidget *widget, gpointer data );

/**
 * Name: chan_plot_left_changed
 * Name: chan_plot_right_changed
 *
 * Description:
 * Signal handlers for the channel entry fields. These get called whenever the
 * corresponding entry field is changed. These update the chan_plot state.
 *
 * Parameters:
 * @param widget              the widget that was changed
 * @param data                the channel plot
 */
void chan_plot_left_changed( GtkWidget *widget, gpointer data );
void chan_plot_right_changed( GtkWidget *widget, gpointer data );

#endif
