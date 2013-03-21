#include "gui/eeg_plot.h"
#include "gui/chan_plot.h"

#include <stdlib.h>

#define PLOTS 5

static const char *initial_left[10] = {
  "FP1", "FP1", "FP2", "FP2", "AUX1", "F7", "F3", "FZ", "F4", "F8",
};
static const char *initial_right[10] = {
  "F7",  "F3",  "F4",  "F8",  "AUX2", "T3", "C3", "CZ", "C4", "T4"
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GtkEegPlot *eeg_plot_new( EdeModel *model )
{
  int row;

  GtkEegPlot *myself;           // Container for EEG plot stuff.
  GtkWidget *main_vbox;         // The main container for this widget.

  //////////////////////////////////////////////////////////////////////////////
  // Create all the widgets.
  //////////////////////////////////////////////////////////////////////////////
  myself = (GtkEegPlot*) malloc( sizeof(GtkEegPlot) );

  main_vbox = gtk_vbox_new( TRUE, 0 );

  myself->chan_plots = (GtkChanPlot**) malloc( sizeof(GtkChanPlot*) * PLOTS );
  for (row = 0; row < PLOTS; row++) {
    myself->chan_plots[row] = chan_plot_new( model, initial_left[row],
                                                    initial_right[row] );
  }

  //////////////////////////////////////////////////////////////////////////////
  // Pack all the widgets into containers.
  //////////////////////////////////////////////////////////////////////////////
  for (row = 0; row < PLOTS; row++) {
    gtk_box_pack_start( GTK_BOX(main_vbox), myself->chan_plots[row]->chan_plot,
                        TRUE, TRUE, 1 );
  }
  myself->eeg_plot = main_vbox;

  return myself;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void eeg_plot_newSamples( GtkEegPlot *eeg_plot, int num_new )
{
  int i;
  for (i = 0; i < PLOTS; i++) {
    chan_plot_newSamples( eeg_plot->chan_plots[i], num_new );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void eeg_plot_reinitialize( GtkEegPlot *eeg_plot )
{
  int i;
  for (i = 0; i < PLOTS; i++) {
    chan_plot_reinitialize( eeg_plot->chan_plots[i] );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void eeg_plot_replot( GtkEegPlot *eeg_plot )
{
  int i;
  for (i = 0; i < PLOTS; i++) {
    chan_plot_replot( eeg_plot->chan_plots[i] );
  }
}
