#include "gui/blink_control.h"

#include <string.h>
#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
GtkWidget *blink_control_new( EdeModel *model )
{
  GtkWidget *main_vbox;       // The main container for this widget.

  GtkWidget *ica_table;       // The table used to arrange ICA controls.
  GtkWidget *blink_table;     // Table for blink parameters.

  GtkWidget *ica_frame;       // Frame for ICA table.
  GtkWidget *blink_frame;     // Frame for blink table.

  GtkWidget *but_fica;        // Buttons for selecting between JADE and FastICA.
  GtkWidget *but_jade;

  GtkWidget *lab_contrast;    // Labels for data entry fields.
  GtkWidget *lab_epsilon;
  GtkWidget *lab_max_iter;
  GtkWidget *lab_gpu_dev;

  GtkWidget *combo_contrast;  // The actual data entry fields.
  GtkWidget *spin_epsilon;
  GtkWidget *spin_max_iter;

  GtkWidget *but_gpu;         // Buttons for selecting between CPU and GPU.
  GtkWidget *but_cpu;

  GtkWidget *lab_t1;          // Labels for blink parameter fields.
  GtkWidget *lab_tcor;

  GtkWidget *spin_t1;         // Spinners for blink parameters.
  GtkWidget *spin_tcor;

  GtkWidget *image_logo;      // Logo widgets.
  GtkWidget *label_logo;
  GtkWidget *box_logo;

  GtkWidget *label_power;     // Red Bull label.

  GtkWidget *label_empty;     // Space filler.

  //////////////////////////////////////////////////////////////////////////////
  // Create all the widgets.
  //////////////////////////////////////////////////////////////////////////////
  main_vbox   = gtk_vbox_new( FALSE, 0 );
  ica_table   = gtk_table_new( 6, 2, TRUE );
  blink_table = gtk_table_new( 2, 2, TRUE );

  ica_frame   = gtk_frame_new( "ICA Controls" );
  blink_frame = gtk_frame_new( "Blink Detection" );

  // Radio buttons that select between JADE and FastICA implementations.
  but_fica = gtk_radio_button_new_with_label( NULL, "FastICA" );
  but_jade = gtk_radio_button_new_with_label_from_widget(
                              GTK_RADIO_BUTTON(but_fica), "JADE" );

  // Labels for manual entry.
  lab_contrast = gtk_label_new( "Contrast: " );
  lab_epsilon  = gtk_label_new( "Epsilon: " );
  lab_max_iter = gtk_label_new( "Max iterations: " );
  lab_gpu_dev  = gtk_label_new( "GPU device: " );

  lab_t1       = gtk_label_new( "T1:" );
  lab_tcor     = gtk_label_new( "Tcor:" );

  gtk_label_set_markup( GTK_LABEL(lab_t1),   "T<sub>1</sub>:" );
  gtk_label_set_markup( GTK_LABEL(lab_tcor), "T<sub>cor</sub>:" );

  // Use a drop down menu for selecting the negentropy estimation.
  combo_contrast = gtk_combo_box_new_text();
  gtk_combo_box_append_text( GTK_COMBO_BOX(combo_contrast), "tanh" );
  gtk_combo_box_append_text( GTK_COMBO_BOX(combo_contrast), "gauss" );
  gtk_combo_box_append_text( GTK_COMBO_BOX(combo_contrast), "cubic" );

  // Use a spin button for selecting the epsilon value (floating point value).
  spin_epsilon = gtk_spin_button_new_with_range( 0.0, 1.0, 0.00000001 );

  // Use a spin button for selecting the max iteration value (integer value).
  spin_max_iter = gtk_spin_button_new_with_range( 1.0, 1000.0, 1.0 );
  gtk_spin_button_set_digits( GTK_SPIN_BUTTON(spin_max_iter), 0 );

  // Use spin buttons for blink parameters.
  spin_t1 = gtk_spin_button_new_with_range( 0.0, 100.0, 1.0 );
  gtk_spin_button_set_digits( GTK_SPIN_BUTTON(spin_t1), 0 );

  spin_tcor = gtk_spin_button_new_with_range( -1.0, 1.0, 0.01 );
  gtk_spin_button_set_digits( GTK_SPIN_BUTTON(spin_tcor), 2 );

  // Radio buttons that select between CPU and GPU implementations.
  but_gpu = gtk_radio_button_new_with_label( NULL, "GPU" );
  but_cpu = gtk_radio_button_new_with_label_from_widget(
                              GTK_RADIO_BUTTON(but_gpu), "CPU" );

  // Stuff for the logo.
  box_logo = gtk_hbox_new( TRUE, 0 );
  image_logo = gtk_image_new_from_file( "images/norem.png" );
  label_logo = gtk_label_new("");
  gtk_label_set_markup( GTK_LABEL(label_logo),
                        "<big><b>E</b></big>ye-blink\n"
                        "<big><b>D</b></big>etector &amp;\n"
                        "<big><b>E</b></big>liminator" );

  label_power = gtk_label_new("");
  gtk_label_set_markup( GTK_LABEL(label_power),
                        "<small>Powered by RedBull&#174;</small>" );

  label_empty = gtk_label_new("");

  //////////////////////////////////////////////////////////////////////////////
  // Set widgets to the state given by the model.
  //////////////////////////////////////////////////////////////////////////////
  switch (model->ica_params.implem) {
    case ICA_FASTICA:
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but_fica), TRUE );
      break;

    case ICA_JADE:
    default:
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but_jade), TRUE );
      break;
  }

  if (model->ica_params.use_gpu) {
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but_gpu),  TRUE );
  } else {
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(but_cpu),  TRUE );
  }

  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_epsilon),
                             model->ica_params.epsilon );
  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_max_iter),
                             model->ica_params.max_iter );

  switch (model->ica_params.contrast) {
    case NONLIN_TANH:
      gtk_combo_box_set_active( GTK_COMBO_BOX(combo_contrast), 0 ); break;
    case NONLIN_GAUSS:
      gtk_combo_box_set_active( GTK_COMBO_BOX(combo_contrast), 1 ); break;
    case NONLIN_CUBE:
    default:
      gtk_combo_box_set_active( GTK_COMBO_BOX(combo_contrast), 2 ); break;
  }

  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_t1),   model->b_params.t_1 );
  gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_tcor), model->b_params.t_cor);

  //////////////////////////////////////////////////////////////////////////////
  // Attach signal handlers to the widgets.
  //////////////////////////////////////////////////////////////////////////////
  g_signal_connect(but_fica, "toggled", G_CALLBACK(ica_implem_changed), model);
  g_signal_connect(but_jade, "toggled", G_CALLBACK(ica_implem_changed), model);
  g_signal_connect(but_gpu,  "toggled", G_CALLBACK(ica_gpucpu_changed), model);
  g_signal_connect(but_cpu,  "toggled", G_CALLBACK(ica_gpucpu_changed), model);

  g_signal_connect(combo_contrast, "changed", G_CALLBACK(ica_contrast_changed),
                   model );

  g_signal_connect( spin_max_iter, "value-changed",
                    G_CALLBACK(ica_maxiter_changed), model );
  g_signal_connect( spin_epsilon, "value-changed",
                    G_CALLBACK(ica_epsilon_changed), model );

  g_signal_connect( spin_t1, "value-changed",
                    G_CALLBACK(blink_t1_changed), model );
  g_signal_connect( spin_tcor, "value-changed",
                    G_CALLBACK(blink_tcor_changed), model );

  //////////////////////////////////////////////////////////////////////////////
  // Pack all the widgets into containers.
  //////////////////////////////////////////////////////////////////////////////
  gtk_table_attach_defaults( GTK_TABLE(ica_table), but_fica, 0, 1, 0, 2 );
  gtk_table_attach_defaults( GTK_TABLE(ica_table), but_jade, 1, 2, 0, 2 );

  gtk_table_attach_defaults( GTK_TABLE(ica_table), lab_contrast,   0, 1, 2, 3 );
  gtk_table_attach_defaults( GTK_TABLE(ica_table), combo_contrast, 1, 2, 2, 3 );

  gtk_table_attach_defaults( GTK_TABLE(ica_table), lab_epsilon,  0, 1, 3, 4 );
  gtk_table_attach_defaults( GTK_TABLE(ica_table), spin_epsilon, 1, 2, 3, 4 );

  gtk_table_attach_defaults( GTK_TABLE(ica_table), lab_max_iter,  0, 1, 4, 5 );
  gtk_table_attach_defaults( GTK_TABLE(ica_table), spin_max_iter, 1, 2, 4, 5 );

  gtk_table_attach_defaults( GTK_TABLE(ica_table), but_gpu, 0, 1, 5, 7 );
  gtk_table_attach_defaults( GTK_TABLE(ica_table), but_cpu, 1, 2, 5, 7 );

  gtk_table_attach_defaults( GTK_TABLE(blink_table), lab_t1,  0, 1, 0, 1 );
  gtk_table_attach_defaults( GTK_TABLE(blink_table), spin_t1, 1, 2, 0, 1 );

  gtk_table_attach_defaults( GTK_TABLE(blink_table), lab_tcor,  0, 1, 1, 2 );
  gtk_table_attach_defaults( GTK_TABLE(blink_table), spin_tcor, 1, 2, 1, 2 );

  gtk_box_pack_start( GTK_BOX(box_logo), image_logo, TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX(box_logo), label_logo, TRUE, TRUE, 0 );

  gtk_container_add( GTK_CONTAINER(ica_frame),   ica_table );
  gtk_container_add( GTK_CONTAINER(blink_frame), blink_table );

  // Pack the table into a box so that the table stuff is always at the top of
  // the window and won't get stretched out.
  gtk_box_pack_start( GTK_BOX(main_vbox), ica_frame, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), blink_frame, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), label_empty, TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), box_logo, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(main_vbox), label_power, FALSE, FALSE, 0 );

  return main_vbox;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_implem_changed( GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  // Only change states if this is the active toggle button. This function will
  // be called for both the active and inactive buttons.
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    // Make sure to lock/unlock the mutex when changing configuration details.
    pthread_mutex_lock( &(model->ica_lock) );
      if (strcmp( gtk_button_get_label(GTK_BUTTON(widget)), "JADE" ) == 0) {
        model->ica_params.implem = ICA_JADE;
      } else {
        model->ica_params.implem = ICA_FASTICA;
      }
    pthread_mutex_unlock( &(model->ica_lock) );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_gpucpu_changed( GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  // Only change states if this is the active toggle button. This function will
  // be called for both the active and inactive buttons.
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    // Make sure to lock/unlock the mutex when changing configuration details.
    pthread_mutex_lock( &(model->ica_lock) );
      if (strcmp( gtk_button_get_label(GTK_BUTTON(widget)), "CPU" ) == 0) {
        model->ica_params.use_gpu = 0;
      } else {
        model->ica_params.use_gpu = 1;
      }
    pthread_mutex_unlock( &(model->ica_lock) );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_epsilon_changed(  GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  pthread_mutex_lock( &(model->ica_lock) );
    model->ica_params.epsilon =
      (NUMTYPE) gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(widget));
  pthread_mutex_unlock( &(model->ica_lock) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_maxiter_changed(  GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  pthread_mutex_lock( &(model->ica_lock) );
    model->ica_params.max_iter =
      (unsigned int) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  pthread_mutex_unlock( &(model->ica_lock) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_contrast_changed( GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  pthread_mutex_lock( &(model->ica_lock) );
    // 0 -> tanh, 1 -> gauss, 2 -> cube
    switch (gtk_combo_box_get_active( GTK_COMBO_BOX(widget) )) {
      case 0: model->ica_params.contrast = NONLIN_TANH; break;
      case 1: model->ica_params.contrast = NONLIN_GAUSS; break;
      case 2: model->ica_params.contrast = NONLIN_CUBE; break;
    }
  pthread_mutex_unlock( &(model->ica_lock) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void blink_t1_changed(   GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  pthread_mutex_lock( &(model->b_lock) );
    model->b_params.t_1 =
        (NUMTYPE) gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(widget));
  pthread_mutex_unlock( &(model->b_lock) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void blink_tcor_changed( GtkWidget *widget, gpointer data )
{
  EdeModel *model = (EdeModel*) data;

  pthread_mutex_lock( &(model->b_lock) );
    model->b_params.t_cor =
        (NUMTYPE) gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(widget));
  pthread_mutex_unlock( &(model->b_lock) );
}
