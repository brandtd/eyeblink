#include "gui/ede_model.h"
#include "gui/ede_window.h"

#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>

/**
 * Name: main
 *
 * Description:
 * This program pops up a GUI (called the Eyeblink Detector and Eliminator) that
 * demonstrates the eyeblink detection and elimination computation. This GUI
 * allows the user to open a Xltek ERD file and simulates receiving data in real
 * time by slowing extracting sample points from the Xltek ERD file.
 */
int main( int argc, char **argv )
{
  //////////////////////////////////////////////////////////////////////////////
  // Initialize everything.
  //////////////////////////////////////////////////////////////////////////////
  g_thread_init(NULL);
  gdk_threads_init();
  gdk_threads_enter();
  gtk_init( &argc, &argv );

  EdeModel *ede_model      = ede_model_new();
  GtkEdeWindow *ede_window = ede_window_new( ede_model );

  //////////////////////////////////////////////////////////////////////////////
  // Perform GTK setup.
  //////////////////////////////////////////////////////////////////////////////
  g_signal_connect( ede_window->ede_window, "delete-event",
                    G_CALLBACK( gtk_main_quit ), NULL );

  // Show the GUI.
  gtk_widget_show_all( ede_window->ede_window );

  // Start the main GUI thread.
  gtk_main();

  //////////////////////////////////////////////////////////////////////////////
  // Program is exiting. Perform cleanup.
  //////////////////////////////////////////////////////////////////////////////
  ede_model_destroy( ede_model );
  gtk_widget_destroy( ede_window->ede_window );
  gdk_threads_leave();

  exit(0);
}
