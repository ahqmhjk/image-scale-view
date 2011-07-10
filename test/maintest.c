#include "p_signal.h"

static gint base_w = 120;
static gint base_h = 30;

static void gtk_window_create(GtkWidget **window);
static void gtk_menu_create(GtkWidget *vbox);
static void gtk_frame_create(GtkWidget *vbox);
static void connect();

static void g_global_free();

static GtkWidget *window; 
static GtkWidget *menu_file_new, *menu_file_open, *menu_file_save, *menu_file_save_as, *menu_window_quit; 
static GtkWidget *menu_operation_transform, *menu_operation_grabwindow, *menu_operation_grabwholescr;
static GtkWidget *imageFrame, *hscale, *vscale;
static GScale *g_scale = NULL;
static GtkWidgetList *list = NULL;


int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);
	if (!g_thread_supported()) 	g_thread_init(NULL);
	gdk_threads_init();

	gtk_window_create(&window);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 1);
	gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
	gtk_widget_set_size_request(vbox, 7 * base_w, 20 * base_h);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_menu_create(vbox);
	gtk_frame_create(vbox);

	gdk_threads_enter();
	connect();
	gtk_widget_show_all(window);
	gtk_main();
	gdk_threads_leave();
	g_global_free();
	
	return 0;
}

static void gtk_window_create(GtkWidget **window)
{
	GError *err = NULL;
	*window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(*window), "Image Viewer and Scaler");
	gtk_container_set_border_width(GTK_CONTAINER(*window), 10);
	gtk_window_set_icon_from_file((GtkWindow*)*window, "res/My.ico", &err);
	gtk_window_set_default_size((GtkWindow*)*window, 200, 200);
	gtk_widget_set_size_request(*window, 7 * base_w, 20 * base_h);
	gtk_window_set_resizable(GTK_WINDOW(*window), FALSE);
	gtk_window_set_position(GTK_WINDOW(*window), GTK_WIN_POS_CENTER_ALWAYS);

	g_signal_connect(*window, "delete-event", G_CALLBACK(on_quit_window), NULL);
}

static void gtk_menu_create(GtkWidget *vbox)
{
	// menuBar
	GtkWidget *menu_bar = gtk_menu_bar_new();
	gtk_menu_bar_set_child_pack_direction(GTK_MENU_BAR(menu_bar), GTK_PACK_DIRECTION_TTB);
	gtk_widget_set_size_request(menu_bar, 7 * base_w, base_h);
	
	// File menu
	GtkWidget *menu_file = gtk_menu_item_new_with_mnemonic("File");
	gtk_menu_item_set_use_underline(GTK_MENU_ITEM(menu_file), TRUE);
	gtk_widget_set_size_request(menu_file, base_w, base_h);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_file);
	
	// File's sub menu
	GtkWidget *menu1 = gtk_menu_new();
	menu_file_new = gtk_menu_item_new_with_mnemonic("New");
	menu_file_open = gtk_menu_item_new_with_mnemonic("Open");
	menu_file_save = gtk_menu_item_new_with_mnemonic("Save");
	menu_file_save_as = gtk_menu_item_new_with_mnemonic("Save as");
	menu_window_quit = gtk_menu_item_new_with_mnemonic("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL (menu1), menu_file_new);
	gtk_menu_shell_append(GTK_MENU_SHELL (menu1), menu_file_open);
	gtk_menu_shell_append(GTK_MENU_SHELL (menu1), menu_file_save);
	gtk_menu_shell_append(GTK_MENU_SHELL (menu1), menu_file_save_as);
	gtk_menu_shell_append(GTK_MENU_SHELL (menu1), menu_window_quit);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_file), menu1);	
	
	// operation menu
	GtkWidget *menu_operation = gtk_menu_item_new_with_mnemonic("Operation");
	gtk_menu_item_set_use_underline(GTK_MENU_ITEM(menu_operation), TRUE);
	gtk_widget_set_size_request(menu_operation, base_w, base_h);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_operation);
	
	// operation's sub menu
	GtkWidget *menu2 = gtk_menu_new();
	menu_operation_transform = gtk_menu_item_new_with_mnemonic("Transform");
	menu_operation_grabwholescr = gtk_menu_item_new_with_mnemonic("Grab Whole Screen");
	menu_operation_grabwindow = gtk_menu_item_new_with_mnemonic("Grab Window");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu2), menu_operation_transform);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu2), menu_operation_grabwholescr);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu2), menu_operation_grabwindow);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_operation), menu2);	
	
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_operation);

	GtkWidget *vbox_menu_bar = gtk_vbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vbox_menu_bar), menu_bar, FALSE, FALSE, 2);
	
	gtk_box_pack_start(GTK_BOX(vbox), vbox_menu_bar, TRUE, TRUE, 0);
}


static void gtk_frame_create(GtkWidget *vbox)
{
	GtkWidget *hbox2 = gtk_hbox_new(FALSE, 1);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 10);
	imageFrame = gtk_frame_new("image:");
	gtk_widget_set_size_request(imageFrame, 6 * base_w, 16 * base_h);
	gtk_frame_set_label_align(GTK_FRAME(imageFrame), 0.5, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox2), imageFrame, FALSE, FALSE, 20);
	
	vscale = gtk_vscale_new_with_range(1, 1000, 1);
	gtk_widget_set_size_request(vscale, base_w/4, 16 * base_h);
	gtk_box_pack_start(GTK_BOX(hbox2), vscale, FALSE, FALSE, 0);
	gtk_widget_set_size_request(hbox2, 4 * base_w, 16 * base_h);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, TRUE, TRUE, 0);
	
	hscale = gtk_hscale_new_with_range(1, 1000, 1);
	gtk_widget_set_size_request(hscale, 3 * base_w, base_h);
	gtk_box_pack_start(GTK_BOX(vbox), hscale, TRUE, TRUE, 0);
}

static void g_global_free()
{
	free(g_scale);
	free(list);
}

static void connect()
{
	list = (GtkWidgetList*)malloc(sizeof(GtkWidgetList));
	list->gw1 = window;
	list->gw2 = imageFrame;
	list->hs = (GtkHScale*)(hscale);
	list->vs = (GtkVScale*)(vscale);
	g_signal_connect(menu_file_open, "activate", 
					G_CALLBACK(on_open_filedialog), (gpointer)list);
	
	g_scale = (GScale*)malloc(sizeof(GScale));
	g_scale->frame = imageFrame;
	g_scale->hs = (GtkHScale *)hscale;
	g_scale->vs = (GtkVScale *)vscale;
	g_signal_connect(vscale, "value-changed", G_CALLBACK(on_scale_changed), (gpointer)g_scale);
	g_signal_connect(hscale, "value-changed", G_CALLBACK(on_scale_changed), (gpointer)g_scale);

	g_signal_connect(menu_file_save, "activate", G_CALLBACK(on_save), (gpointer)imageFrame);

	g_signal_connect(menu_file_save_as, "activate", G_CALLBACK(on_save_as), (gpointer)imageFrame);

	g_signal_connect(menu_window_quit, "activate", G_CALLBACK(on_quit_window), NULL);	
	
	g_signal_connect(menu_operation_transform, "activate", G_CALLBACK(on_transform), (gpointer)imageFrame);

	g_signal_connect(menu_operation_grabwholescr, "activate", G_CALLBACK(on_grab_whole_screen), (gpointer)window);
	
	g_signal_connect(menu_operation_grabwindow, "activate", G_CALLBACK(on_grabwindow), (gpointer)window);

}
