#ifndef BLINK_CONTROL_H
#define BLINK_CONTROL_H

#include "gui/ede_model.h"

#include <gtk/gtk.h>

/**
 * Name: blink_control_new
 *
 * Description:
 * Creates a new blink control widget. This widget contains a bunch of controls
 * for configuration of the ICA algorithm and for the blink detection algorithm.
 *
 * Parameters:
 * @param model               the EDE model data
 *
 * Returns:
 * @return GtkWidget*         a widget containing a bunch of controls
 */
GtkWidget *blink_control_new( EdeModel *model );

/**
 * Name: ica_implem_changed
 * Name: ica_gpucpu_changed
 * Name: ica_epsilon_changed
 * Name: ica_maxiter_changed
 * Name: ica_contrast_changed
 *
 * Description:
 * Signal handlers for the ICA control widgets. These are called whenever the
 * control widget state's change and they update the model's ICA configuration.
 *
 * Parameters:
 * @param widget              the control whose state changed
 * @param data                a pointer to the EdeModel
 */
void ica_implem_changed(   GtkWidget *widget, gpointer data );
void ica_gpucpu_changed(   GtkWidget *widget, gpointer data );
void ica_epsilon_changed(  GtkWidget *widget, gpointer data );
void ica_maxiter_changed(  GtkWidget *widget, gpointer data );
void ica_contrast_changed( GtkWidget *widget, gpointer data );

/**
 * Name: blink_t1_changed
 * Name: blink_tcor_changed
 *
 * Description:
 * Signal handlers for the blink control widgets. These are called whenever the
 * blink control widget state's change and they update the blink parameters.
 *
 * Parameters:
 * @param widget              the control whose state changed
 * @param data                a pointer to the EdeModel
 */
void blink_t1_changed(   GtkWidget *widget, gpointer data );
void blink_tcor_changed( GtkWidget *widget, gpointer data );

#endif
