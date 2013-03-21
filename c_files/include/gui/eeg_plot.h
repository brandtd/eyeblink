#ifndef EEG_PLOT_H
#define EEG_PLOT_H

#include "gui/ede_model.h"
#include "gui/chan_plot.h"

#include <gtk/gtk.h>

/**
 * Struct containing eeg plot widget stuff.
 *
 * TODO: the correct thing to do would be to create a new widget, but I don't
 *       know how to do that, and this hack will work for now.
 */
typedef struct GtkEegPlot {
  GtkWidget     *eeg_plot;
  GtkChanPlot **chan_plots;
  chan_plot_state_t state;
} GtkEegPlot;

/**
 * Name: eeg_plot_new
 *
 * Description:
 * Creates a new EEG plotting widget. This widget creates a bunch of controls
 * for selecting specific channels to plot (e.g., 'FP1 - F3', 'F7', etc), and
 * also creates a large window where plot information will actually be put.
 *
 * Parameters:
 * @param model               the EDE model data
 *
 * Returns:
 * @return GtkEegPlot*        the EEG plot widget and state
 */
GtkEegPlot *eeg_plot_new( EdeModel *model );

/**
 * Name: eeg_plot_newSamples
 *
 * Description:
 * Updates the state of the EEG plot with new sample values.
 *
 * Parameters:
 * @param eeg_plot            the eeg plot to update
 * @param num_new             the number of new samples
 */
void eeg_plot_newSamples( GtkEegPlot *eeg_plot, int num_new );

/**
 * Name: eeg_plot_reinitialize
 *
 * Description:
 * Signals to the eeg plot that it needs to reinitialize itself because the EDE
 * model has changed. This function needs to be called when a new ERD file is
 * loaded.
 *
 * Parameters:
 * @param eeg_plot           the eeg plot to reinitialize
 */
void eeg_plot_reinitialize( GtkEegPlot *eeg_plot );

/**
 * Name: eeg_plot_replot
 *
 * Description:
 * Forces EEG plot to recalcuate and replot its values.
 *
 * Parameters:
 * @param eeg_plot          the plot to replot
 */
void eeg_plot_replot( GtkEegPlot *eeg_plot );

#endif
