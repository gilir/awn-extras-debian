/*
 * Copyright (C) 2007, 2008 Rodney Cryderman <rcryderman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
*/
#include <libawn/awn-cairo-utils.h>
#include <glib/gmacros.h>
#include <glib/gerror.h>
#include <gconf/gconf-value.h>

//#include <awn-applet.h>
#include "config_entries.h"
#include <gconf/gconf-client.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#include "render.h"

Cairo_menu_config G_cairo_menu_conf;

static Cairo_menu_config G_cairo_menu_conf_copy;

static GConfClient *gconf_client;

extern AwnApplet *G_applet;

void append_to_launchers(gchar * launcher)
{
  GSList* launcher_list = gconf_client_get_list(gconf_client, "/apps/avant-window-navigator/window_manager/launchers",
                          GCONF_VALUE_STRING, NULL);

  if (launcher_list)
  {
    launcher_list = g_slist_append(launcher_list, launcher);
    gconf_client_set_list(gconf_client, "/apps/avant-window-navigator/window_manager/launchers",
                          GCONF_VALUE_STRING, launcher_list, NULL);
  }


}

void read_config(void)
{
  gchar * svalue;
  gchar  * tmp;
  GConfValue*  value;
  gconf_client = gconf_client_get_default();

  svalue = gconf_client_get_string(gconf_client, GCONF_NORMAL_BG, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_NORMAL_BG, svalue = g_strdup("DDDDDDEE"), NULL);
  }

  awn_cairo_string_to_color(svalue, &G_cairo_menu_conf.normal.bg);

  g_free(svalue);

  svalue = gconf_client_get_string(gconf_client, GCONF_NORMAL_FG, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_NORMAL_FG, svalue = g_strdup("000000FF"), NULL);
  }

  awn_cairo_string_to_color(svalue, &G_cairo_menu_conf.normal.fg);

  g_free(svalue);

  svalue = gconf_client_get_string(gconf_client, GCONF_HOVER_BG, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_HOVER_BG, svalue = g_strdup("0022DDf0"), NULL);
  }

  awn_cairo_string_to_color(svalue, &G_cairo_menu_conf.hover.bg);

  g_free(svalue);

  svalue = gconf_client_get_string(gconf_client, GCONF_HOVER_FG, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_HOVER_FG, svalue = g_strdup("000000FF"), NULL);
  }

  awn_cairo_string_to_color(svalue, &G_cairo_menu_conf.hover.fg);

  g_free(svalue);

  value = gconf_client_get(gconf_client, GCONF_TEXT_SIZE, NULL);

  if (value)
  {
    G_cairo_menu_conf.text_size = gconf_client_get_int(gconf_client, GCONF_TEXT_SIZE, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.text_size = 14;
    gconf_client_set_int(gconf_client, GCONF_TEXT_SIZE, G_cairo_menu_conf.text_size , NULL);
  }

  value = gconf_client_get(gconf_client, GCONF_SHOW_SEARCH, NULL);

  if (value)
  {
    G_cairo_menu_conf.show_search = gconf_client_get_bool(gconf_client, GCONF_SHOW_SEARCH, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.show_search = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_SHOW_SEARCH, G_cairo_menu_conf.show_search, NULL);
  }

  svalue = gconf_client_get_string(gconf_client, GCONF_SEARCH_CMD, NULL);

  if (!svalue)
  {
    svalue = g_find_program_in_path("tracker-search-tool");

    if (!svalue)
    {
      svalue = g_find_program_in_path("beagle-search");
    }

    if (!svalue)
    {
      svalue = g_strdup("terminal -x locate");
      //gconf_client_set_bool(gconf_client,GCONF_SHOW_SEARCH,FALSE,NULL);
    }

    gconf_client_set_string(gconf_client , GCONF_SEARCH_CMD, svalue, NULL);

//    svalue==g_strdup("tracker-search-tool");
  }
  else
  {
    tmp = svalue;
    svalue = g_filename_from_utf8(svalue, -1, NULL, NULL, NULL);
    g_free(tmp);
  }

  G_cairo_menu_conf.search_cmd = g_strdup(svalue);

  g_free(svalue);


  value = gconf_client_get(gconf_client, GCONF_MENU_GRADIENT, NULL);

  if (value)
  {
    G_cairo_menu_conf.menu_item_gradient_factor = gconf_client_get_float(gconf_client, GCONF_MENU_GRADIENT, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.menu_item_gradient_factor = 0.8;
    gconf_client_set_float(gconf_client, GCONF_MENU_GRADIENT, G_cairo_menu_conf.menu_item_gradient_factor, NULL);
  }


  value = gconf_client_get(gconf_client, GCONF_MENU_ITEM_TEXT_LEN, NULL);

  if (value)
  {
    G_cairo_menu_conf.menu_item_text_len = gconf_client_get_int(gconf_client, GCONF_MENU_ITEM_TEXT_LEN, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.menu_item_text_len = 28;
    gconf_client_set_int(gconf_client, GCONF_MENU_ITEM_TEXT_LEN, G_cairo_menu_conf.menu_item_text_len, NULL);
  }


  value = gconf_client_get(gconf_client, GCONF_SHOW_RUN, NULL);

  if (value)
  {
    G_cairo_menu_conf.show_run = gconf_client_get_bool(gconf_client, GCONF_SHOW_RUN, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.show_run = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_SHOW_RUN, G_cairo_menu_conf.show_run, NULL);
  }


  value = gconf_client_get(gconf_client, GCONF_DO_FADE, NULL);

  if (value)
  {
    G_cairo_menu_conf.do_fade = gconf_client_get_bool(gconf_client, GCONF_DO_FADE, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.do_fade = FALSE;
    gconf_client_set_bool(gconf_client, GCONF_DO_FADE, G_cairo_menu_conf.do_fade, NULL);
  }

  value = gconf_client_get(gconf_client, GCONF_SHOW_PLACES, NULL);

  if (value)
  {
    G_cairo_menu_conf.show_places = gconf_client_get_bool(gconf_client, GCONF_SHOW_PLACES, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.show_places = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_SHOW_PLACES, G_cairo_menu_conf.show_places, NULL);
  }

  svalue = gconf_client_get_string(gconf_client, GCONF_FILEMANAGER, NULL);

  if (!svalue)
  {
    svalue = g_find_program_in_path("xdg-open");

    if (!svalue)
    {
      svalue = g_find_program_in_path("nautilus");
    }

    if (!svalue)
    {
      svalue = g_strdup("thunar");
      //gconf_client_set_bool(gconf_client,GCONF_SHOW_SEARCH,FALSE,NULL);
    }
    else
    {
      svalue = g_strdup("xdg-open");
    }

    gconf_client_set_string(gconf_client , GCONF_FILEMANAGER, svalue, NULL);
  }
  else
  {
    tmp = svalue;
    svalue = g_filename_from_utf8(svalue, -1, NULL, NULL, NULL);
    g_free(tmp);
  }

  G_cairo_menu_conf.filemanager = strdup(svalue);

  g_free(svalue);


  svalue = gconf_client_get_string(gconf_client, GCONF_APPLET_ICON, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_APPLET_ICON, svalue = g_strdup("gnome-main-menu"), NULL);
  }

  G_cairo_menu_conf.applet_icon = strdup(svalue);

  g_free(svalue);


  value = gconf_client_get(gconf_client, GCONF_ON_BUTTON_RELEASE, NULL);

  if (value)
  {
    G_cairo_menu_conf.on_button_release = gconf_client_get_bool(gconf_client, GCONF_ON_BUTTON_RELEASE, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.on_button_release = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_ON_BUTTON_RELEASE, G_cairo_menu_conf.on_button_release, NULL);
  }

  value = gconf_client_get(gconf_client, GCONF_SHOW_TOOLTIPS, NULL);

  if (value)
  {
    G_cairo_menu_conf.show_tooltips = gconf_client_get_bool(gconf_client, GCONF_SHOW_TOOLTIPS, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.show_tooltips = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_SHOW_TOOLTIPS, G_cairo_menu_conf.show_tooltips, NULL);
  }

  svalue = gconf_client_get_string(gconf_client, GCONF_LOGOUT, NULL);

  if (!svalue)
  {

    svalue = g_find_program_in_path("closure");

    if (!svalue)
    {

      svalue = g_find_program_in_path("gnome-session-save");

      if (svalue)
      {
        tmp = svalue;
        svalue = g_strdup("gnome-session-save --kill");
        g_free(tmp);
      }

      if (!svalue)
      {
        svalue = g_strdup("closure");
      }
    }

    gconf_client_set_string(gconf_client , GCONF_LOGOUT, svalue, NULL);
  }
  else
  {
    tmp = svalue;
    svalue = g_filename_from_utf8(svalue, -1, NULL, NULL, NULL);
    g_free(tmp);
  }

  G_cairo_menu_conf.logout = g_strdup(svalue);

  g_free(svalue);


  value = gconf_client_get(gconf_client, GCONF_SHOW_LOGOUT, NULL);

  if (value)
  {
    G_cairo_menu_conf.show_logout = gconf_client_get_bool(gconf_client, GCONF_SHOW_LOGOUT, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.show_logout = FALSE;
    gconf_client_set_bool(gconf_client, GCONF_SHOW_LOGOUT, G_cairo_menu_conf.show_logout, NULL);
  }


  value = gconf_client_get(gconf_client, GCONF_BORDER_WIDTH, NULL);

  if (value)
  {
    G_cairo_menu_conf.border_width = gconf_client_get_int(gconf_client, GCONF_BORDER_WIDTH, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.border_width = 1;
    gconf_client_set_int(gconf_client, GCONF_BORDER_WIDTH, G_cairo_menu_conf.border_width, NULL);
  }

  svalue = gconf_client_get_string(gconf_client, GCONF_BORDER_COLOUR, NULL);

  if (!svalue)
  {
    gconf_client_set_string(gconf_client , GCONF_BORDER_COLOUR, svalue = g_strdup("11111133"), NULL);
  }

  awn_cairo_string_to_color(svalue, &G_cairo_menu_conf.border_colour);

  g_free(svalue);


  value = gconf_client_get(gconf_client, GCONF_HONOUR_GTK, NULL);

  if (value)
  {
    G_cairo_menu_conf.honour_gtk = gconf_client_get_bool(gconf_client, GCONF_HONOUR_GTK, NULL) ;
  }
  else
  {
    G_cairo_menu_conf.honour_gtk = TRUE;
    gconf_client_set_bool(gconf_client, GCONF_HONOUR_GTK, G_cairo_menu_conf.honour_gtk, NULL);
  }



  if (G_cairo_menu_conf.honour_gtk)
  {
    GdkColor d;
    GtkWidget *top_win = GTK_WIDGET(G_applet);

    d = top_win->style->base[0];
    G_cairo_menu_conf.normal.bg.red = d.red / 65535.0;
    G_cairo_menu_conf.normal.bg.green = d.green / 65535.0;
    G_cairo_menu_conf.normal.bg.blue = d.blue / 65535.0;
    G_cairo_menu_conf.normal.bg.alpha = 0.9;

    d = top_win->style->fg[0];
    G_cairo_menu_conf.normal.fg.red = d.red / 65535.0;
    G_cairo_menu_conf.normal.fg.green = d.green / 65535.0;
    G_cairo_menu_conf.normal.fg.blue = d.blue / 65535.0;
    G_cairo_menu_conf.normal.fg.alpha = 0.9;


    d = top_win->style->bg[GTK_STATE_ACTIVE];
    G_cairo_menu_conf.hover.bg.red = d.red / 65535.0;
    G_cairo_menu_conf.hover.bg.green = d.green / 65535.0;
    G_cairo_menu_conf.hover.bg.blue = d.blue / 65535.0;
    G_cairo_menu_conf.hover.bg.alpha = 0.9;

    d = top_win->style->fg[GTK_STATE_ACTIVE];
    G_cairo_menu_conf.hover.fg.red = d.red / 65535.0;
    G_cairo_menu_conf.hover.fg.green = d.green / 65535.0;
    G_cairo_menu_conf.hover.fg.blue = d.blue / 65535.0;
    G_cairo_menu_conf.hover.fg.alpha = 0.9;


    d = top_win->style->text_aa[0];
    G_cairo_menu_conf.border_colour.red = d.red / 65535.0;
    G_cairo_menu_conf.border_colour.green = d.green / 65535.0;
    G_cairo_menu_conf.border_colour.blue = d.blue / 65535.0;
    G_cairo_menu_conf.border_colour.alpha = 0.4;

    G_cairo_menu_conf.menu_item_gradient_factor = 1.0;
  }

}


/*



#define GCONF_APPLET_ICON GCONF_MENU "/applet_icon"

#define GCONF_ON_BUTTON_RELEASE GCONF_MENU "/activate_on_release"

*/

char * awncolor_to_string(AwnColor * colour)
{

  return g_strdup_printf("%02x%02x%02x%02x",
                         (unsigned int) round((colour->red*255)),
                         (unsigned int) round((colour->green*255)),
                         (unsigned int) round((colour->blue*255)),
                         (unsigned int) round((colour->alpha*255))
                        );
}


static void _save_config(void)
{
  gchar * svalue;

  gconf_client = gconf_client_get_default();

  svalue = awncolor_to_string(&G_cairo_menu_conf.normal.bg);
  gconf_client_set_string(gconf_client , GCONF_NORMAL_BG, svalue, NULL);
  g_free(svalue);

  svalue = awncolor_to_string(&G_cairo_menu_conf.normal.fg);
  gconf_client_set_string(gconf_client , GCONF_NORMAL_FG, svalue, NULL);
  g_free(svalue);

  svalue = awncolor_to_string(&G_cairo_menu_conf.hover.bg);
  gconf_client_set_string(gconf_client , GCONF_HOVER_BG, svalue, NULL);
  g_free(svalue);

  svalue = awncolor_to_string(&G_cairo_menu_conf.hover.fg);
  gconf_client_set_string(gconf_client, GCONF_HOVER_FG, svalue, NULL);
  g_free(svalue);

  gconf_client_set_int(gconf_client, GCONF_TEXT_SIZE, G_cairo_menu_conf.text_size , NULL);

  gconf_client_set_bool(gconf_client, GCONF_SHOW_SEARCH, G_cairo_menu_conf.show_search, NULL);

  gconf_client_set_string(gconf_client ,  G_cairo_menu_conf.search_cmd, svalue, NULL);

  gconf_client_set_float(gconf_client, GCONF_MENU_GRADIENT, G_cairo_menu_conf.menu_item_gradient_factor, NULL);

  gconf_client_set_int(gconf_client, GCONF_MENU_ITEM_TEXT_LEN, G_cairo_menu_conf.menu_item_text_len, NULL);

  gconf_client_set_bool(gconf_client, GCONF_SHOW_RUN, G_cairo_menu_conf.show_run, NULL);

  gconf_client_set_bool(gconf_client, GCONF_DO_FADE, G_cairo_menu_conf.do_fade, NULL);

  gconf_client_set_bool(gconf_client, GCONF_SHOW_PLACES, G_cairo_menu_conf.show_places, NULL);

  gconf_client_set_string(gconf_client , GCONF_FILEMANAGER, G_cairo_menu_conf.filemanager, NULL);

  gconf_client_set_string(gconf_client , GCONF_APPLET_ICON, G_cairo_menu_conf.applet_icon, NULL);

  gconf_client_set_bool(gconf_client, GCONF_ON_BUTTON_RELEASE, G_cairo_menu_conf.on_button_release, NULL);

  gconf_client_set_bool(gconf_client, GCONF_HONOUR_GTK, G_cairo_menu_conf.honour_gtk, NULL);

  gconf_client_set_bool(gconf_client, GCONF_SHOW_TOOLTIPS, G_cairo_menu_conf.show_tooltips, NULL);

  gconf_client_set_bool(gconf_client, GCONF_SHOW_LOGOUT, G_cairo_menu_conf.show_logout, NULL);

  gconf_client_set_string(gconf_client , GCONF_LOGOUT, G_cairo_menu_conf.logout, NULL);

  gconf_client_set_int(gconf_client, GCONF_BORDER_WIDTH, G_cairo_menu_conf.border_width, NULL);

  svalue = awncolor_to_string(&G_cairo_menu_conf.border_colour);
  gconf_client_set_string(gconf_client , GCONF_BORDER_COLOUR, svalue, NULL);
  g_free(svalue);

}

static gboolean _press_ok(GtkWidget *widget, GdkEventButton *event, GtkWidget * win)
{
  _save_config();
  gtk_widget_destroy(win);
  g_object_unref(gconf_client) ;
  GError *err = NULL;
  GtkWidget *dialog, *label;

  dialog = gtk_dialog_new_with_buttons("Cairo Menu Message",
                                       NULL,
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_STOCK_OK,
                                       GTK_RESPONSE_NONE,
                                       NULL);
  label = gtk_label_new("About to restart Cairo Menu.  Please shutdown any instances of awn-manager");

  /* Ensure that the dialog box is destroyed when the user responds. */

  g_signal_connect_swapped(dialog,
                           "response",
                           G_CALLBACK(gtk_widget_destroy),
                           dialog);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
                    label);
  gtk_widget_show_all(dialog);
  gtk_dialog_run(dialog);
  g_spawn_command_line_async("sh -c  'export T_STAMP=`date +\"%s\"`&& export AWN_G_ORIG=`gconftool-2 -g /apps/avant-window-navigator/applets_list | sed -e \"s/cairo_main_menu\.desktop::[0-9]*/cairo_main_menu\.desktop::$T_STAMP/\"` && export AWN_G_MOD=`echo $AWN_G_ORIG |sed -e \"s/[^,^\[]*cairo_main_menu\.desktop::[0-9]*,?//\"` && gconftool-2 --type list --list-type=string -s /apps/avant-window-navigator/applets_list \"$AWN_G_MOD\" && sleep 2 && gconftool-2 --type list --list-type=string -s /apps/avant-window-navigator/applets_list \"$AWN_G_ORIG\"'", &err);
  exit(0);
  return FALSE;
}


static gboolean _toggle_(GtkWidget *widget, gboolean * value)
{
  *value = !*value;

  return FALSE;
}

static gboolean _toggle_gtk(GtkWidget *widget, GtkWidget * gtk_off_section)
{
// gtk_toggle_button_set_active(widget,G_cairo_menu_conf.honour_gtk);
  G_cairo_menu_conf.honour_gtk = gtk_toggle_button_get_active(widget);

  if (G_cairo_menu_conf.honour_gtk)
  {
    gtk_widget_hide(gtk_off_section);
  }
  else
  {
    gtk_widget_show_all(gtk_off_section);
  }

  return TRUE;
}

int activate(GtkWidget *w, gchar **p)
{
  gchar * svalue = *p;
  g_free(svalue);
  svalue = g_filename_to_utf8(gtk_entry_get_text(w) , -1, NULL, NULL, NULL);
  *p = svalue;
  return FALSE;
}

/*I'm lazy.. and I do not like doing pref dialogs....*/
GtkWidget *gtk_off_table;
GtkWidget * hover_ex;
GtkWidget * normal_ex;

void _mod_colour(GtkColorButton *widget, AwnColor * user_data)
{
  GdkColor colr;
  gtk_color_button_get_color(widget, &colr);
  user_data->red = colr.red / 65535.0;
  user_data->green = colr.green / 65535.0;
  user_data->blue = colr.blue / 65535.0;
  user_data->alpha = gtk_color_button_get_alpha(widget) / 65535.0;
  gtk_widget_destroy(hover_ex);
  gtk_widget_destroy(normal_ex);
  hover_ex = build_menu_widget(&G_cairo_menu_conf.hover, "Hover", NULL, NULL, 200);
  normal_ex = build_menu_widget(&G_cairo_menu_conf.normal, "Normal", NULL, NULL, 200);

  gtk_table_attach_defaults(gtk_off_table, normal_ex, 3, 4, 0, 1);
  gtk_table_attach_defaults(gtk_off_table, hover_ex, 3, 4, 1, 2);
  gtk_widget_show(hover_ex);
  gtk_widget_show(normal_ex);
}

void spin_change(GtkSpinButton *spinbutton, double * val)
{
  *val = gtk_spin_button_get_value(spinbutton);
}

void spin_int_change(GtkSpinButton *spinbutton, int * val)
{
  *val = gtk_spin_button_get_value(spinbutton);
}

void _file_set(GtkFileChooserButton *filechooserbutton, gchar **p)
{
  gchar * svalue = *p;
  gchar * tmp;
  tmp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooserbutton));

  if (tmp)
  {
    g_free(svalue);
    svalue = g_filename_to_utf8(tmp, -1, NULL, NULL, NULL) ;
    g_free(tmp);
    *p = svalue;
  }
}

void show_prefs(void)
{
  G_cairo_menu_conf_copy = G_cairo_menu_conf;

  GtkWidget * prefs_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GdkColormap *colormap;
  GdkScreen *screen;
  gchar * tmp;

  screen = gtk_window_get_screen(GTK_WINDOW(prefs_win));
  colormap = gdk_screen_get_rgba_colormap(screen);

  if (colormap != NULL && gdk_screen_is_composited(screen))
  {
    gtk_widget_set_colormap(prefs_win, colormap);
  }

  gtk_window_set_title(prefs_win, "Cairo Menu Preferences");

  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  GtkWidget * gtk = gtk_check_button_new_with_label("Use Gtk");
  GtkWidget * places = gtk_check_button_new_with_label("Show Places");
  GtkWidget * search = gtk_check_button_new_with_label("Show Search");
  GtkWidget * run = gtk_check_button_new_with_label("Show Run");
  GtkWidget * logout = gtk_check_button_new_with_label("Show Logout");
  GtkWidget * fade_in = gtk_check_button_new_with_label("Fade in menu");
  GtkWidget * release = gtk_check_button_new_with_label("Activate On Release");
  GtkWidget * tooltips = gtk_check_button_new_with_label("Show tooltips");


  GtkWidget* gtk_off_section = gtk_vbox_new(FALSE, 0);
  gtk_off_table = gtk_table_new(2, 4, FALSE);

  GtkWidget *normal_label = gtk_label_new("Normal");
  GdkColor  colr;

  colr.red = G_cairo_menu_conf.normal.bg.red * 65535;
  colr.green = G_cairo_menu_conf.normal.bg.green * 65535;
  colr.blue = G_cairo_menu_conf.normal.bg.blue * 65535;
  GtkWidget *normal_bg = gtk_color_button_new_with_color(&colr);
  gtk_color_button_set_use_alpha(normal_bg, TRUE);
  gtk_color_button_set_alpha(normal_bg, G_cairo_menu_conf.normal.bg.alpha*65535);
  g_signal_connect(G_OBJECT(normal_bg), "color-set", G_CALLBACK(_mod_colour), &G_cairo_menu_conf.normal.bg);

  colr.red = G_cairo_menu_conf.normal.fg.red * 65535;
  colr.green = G_cairo_menu_conf.normal.fg.green * 65535;
  colr.blue = G_cairo_menu_conf.normal.fg.blue * 65535;
  GtkWidget *normal_fg = gtk_color_button_new_with_color(&colr);
  gtk_color_button_set_use_alpha(normal_fg, TRUE);
  gtk_color_button_set_alpha(normal_fg, G_cairo_menu_conf.normal.fg.alpha*65535);
  g_signal_connect(G_OBJECT(normal_fg), "color-set", G_CALLBACK(_mod_colour), &G_cairo_menu_conf.normal.fg);

  GtkWidget *hover_label = gtk_label_new("Hover");
// GtkWidget *hover_bg=gtk_button_new_with_label("Background");

  colr.red = G_cairo_menu_conf.hover.bg.red * 65535;
  colr.green = G_cairo_menu_conf.hover.bg.green * 65535;
  colr.blue = G_cairo_menu_conf.hover.bg.blue * 65535;
  GtkWidget *hover_bg = gtk_color_button_new_with_color(&colr);
  gtk_color_button_set_use_alpha(hover_bg, TRUE);
  gtk_color_button_set_alpha(hover_bg, G_cairo_menu_conf.hover.bg.alpha*65535);
  g_signal_connect(G_OBJECT(hover_bg), "color-set", G_CALLBACK(_mod_colour), &G_cairo_menu_conf.hover.bg);

// GtkWidget *hover_fg=gtk_button_new_with_label("Foreground");
  colr.red = G_cairo_menu_conf.hover.fg.red * 65535;
  colr.green = G_cairo_menu_conf.hover.fg.green * 65535;
  colr.blue = G_cairo_menu_conf.hover.fg.blue * 65535;
  GtkWidget *hover_fg = gtk_color_button_new_with_color(&colr);
  gtk_color_button_set_use_alpha(hover_fg, TRUE);
  gtk_color_button_set_alpha(hover_fg, G_cairo_menu_conf.hover.fg.alpha*65535);
  g_signal_connect(G_OBJECT(hover_fg), "color-set", G_CALLBACK(_mod_colour), &G_cairo_menu_conf.hover.fg);


  GtkWidget *border_label = gtk_label_new("Border");

  colr.red = G_cairo_menu_conf.border_colour.red * 65535;
  colr.green = G_cairo_menu_conf.border_colour.green * 65535;
  colr.blue = G_cairo_menu_conf.border_colour.blue * 65535;
  GtkWidget *border_colour = gtk_color_button_new_with_color(&colr);
  gtk_color_button_set_use_alpha(border_colour, TRUE);
  gtk_color_button_set_alpha(border_colour, G_cairo_menu_conf.border_colour.alpha*65535);
  g_signal_connect(G_OBJECT(border_colour), "color-set", G_CALLBACK(_mod_colour), &G_cairo_menu_conf.border_colour);


  GtkWidget * text_table = gtk_table_new(2, 4, FALSE);
// GtkWidget * search_cmd=gtk_entry_new();
  GtkWidget * search_cmd = gtk_file_chooser_button_new("Search Util", GTK_FILE_CHOOSER_ACTION_OPEN);
// GtkWidget * filemanager=gtk_entry_new();
  GtkWidget * filemanager = gtk_file_chooser_button_new("File Manager", GTK_FILE_CHOOSER_ACTION_OPEN);
// gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (filemanager),"/usr/bin");

  tmp = g_filename_from_utf8(G_cairo_menu_conf.filemanager, -1, NULL, NULL, NULL) ;
  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(filemanager), tmp);
  g_free(tmp);

  tmp = g_filename_from_utf8(G_cairo_menu_conf.search_cmd, -1, NULL, NULL, NULL) ;
  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(search_cmd), tmp);
  g_free(tmp);


  GtkWidget * adjust_gradient = gtk_spin_button_new_with_range(0.0, 1.0, 0.01);

  GtkWidget * adjust_textlen = gtk_spin_button_new_with_range(5, 30, 1);
  GtkWidget * adjust_textsize = gtk_spin_button_new_with_range(4, 40, 1);
  GtkWidget * adjust_borderwidth = gtk_spin_button_new_with_range(0, 10, 1);

  GtkWidget* buttons = gtk_hbox_new(FALSE, 0);
  GtkWidget* ok = gtk_button_new_with_label("Ok");

  Menu_item_color mic;
  mic.bg = G_cairo_menu_conf.normal.bg;
  mic.fg = G_cairo_menu_conf.normal.fg;
  normal_ex = build_menu_widget(&mic, "Normal", NULL, NULL, 200);

  mic.bg = G_cairo_menu_conf.hover.bg;
  mic.fg = G_cairo_menu_conf.hover.fg;
  hover_ex = build_menu_widget(&mic, "Hover", NULL, NULL, 200);


  gtk_window_set_keep_above(GTK_WINDOW(prefs_win), TRUE);
  gtk_window_set_accept_focus(GTK_WINDOW(prefs_win), TRUE);
  gtk_window_set_focus_on_map(GTK_WINDOW(prefs_win), TRUE);

  gtk_spin_button_set_value(adjust_gradient, G_cairo_menu_conf.menu_item_gradient_factor);
  gtk_spin_button_set_value(adjust_textlen, G_cairo_menu_conf.menu_item_text_len);
  gtk_spin_button_set_value(adjust_textsize, G_cairo_menu_conf.text_size);
  gtk_spin_button_set_value(adjust_borderwidth, G_cairo_menu_conf.border_width);
  g_signal_connect(G_OBJECT(adjust_gradient), "value-changed", G_CALLBACK(spin_change),
                   &G_cairo_menu_conf.menu_item_gradient_factor);
  g_signal_connect(G_OBJECT(adjust_textlen), "value-changed", G_CALLBACK(spin_int_change),
                   &G_cairo_menu_conf.menu_item_text_len);
  g_signal_connect(G_OBJECT(adjust_textsize), "value-changed", G_CALLBACK(spin_int_change),
                   &G_cairo_menu_conf.text_size);
  g_signal_connect(G_OBJECT(adjust_borderwidth), "value-changed", G_CALLBACK(spin_int_change),
                   &G_cairo_menu_conf.border_width);

  g_signal_connect(G_OBJECT(search_cmd), "file-set", G_CALLBACK(_file_set), &G_cairo_menu_conf.search_cmd);
  g_signal_connect(G_OBJECT(filemanager), "file-set", G_CALLBACK(_file_set), &G_cairo_menu_conf.filemanager);

  gtk_toggle_button_set_active(gtk, G_cairo_menu_conf.honour_gtk);

  gtk_toggle_button_set_active(search, G_cairo_menu_conf.show_search);
  g_signal_connect(G_OBJECT(search), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.show_search);
  gtk_toggle_button_set_active(places, G_cairo_menu_conf.show_places);
  g_signal_connect(G_OBJECT(places), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.show_places);
  gtk_toggle_button_set_active(release, G_cairo_menu_conf.on_button_release);
  g_signal_connect(G_OBJECT(release), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.on_button_release);
  gtk_toggle_button_set_active(tooltips, G_cairo_menu_conf.show_tooltips);
  g_signal_connect(G_OBJECT(tooltips), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.show_tooltips);

  gtk_toggle_button_set_active(run, G_cairo_menu_conf.show_run);
  g_signal_connect(G_OBJECT(run), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.show_run);
  gtk_toggle_button_set_active(logout, G_cairo_menu_conf.show_logout);
  g_signal_connect(G_OBJECT(logout), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.show_logout);

  gtk_toggle_button_set_active(fade_in, G_cairo_menu_conf.do_fade);
  g_signal_connect(G_OBJECT(fade_in), "toggled", G_CALLBACK(_toggle_), &G_cairo_menu_conf.do_fade);


  g_signal_connect(G_OBJECT(ok), "button-press-event", G_CALLBACK(_press_ok), prefs_win);

  gtk_container_add(GTK_CONTAINER(prefs_win), vbox);

  g_signal_connect(G_OBJECT(gtk), "toggled", G_CALLBACK(_toggle_gtk), gtk_off_section);

  gtk_box_pack_start(GTK_CONTAINER(vbox), search, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), places, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), run, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), logout, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), fade_in, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), release, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(vbox), tooltips, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_CONTAINER(vbox), text_table, FALSE, FALSE, 0);
  gtk_table_attach_defaults(text_table, gtk_label_new("Search command"), 0, 1, 0, 1);
  gtk_table_attach_defaults(text_table, search_cmd, 1, 2, 0, 1);
  gtk_table_attach_defaults(text_table, gtk_label_new("File Manager"), 0, 1, 1, 2);
  gtk_table_attach_defaults(text_table, filemanager, 1, 2, 1, 2);
  gtk_table_attach_defaults(text_table, gtk_label_new("Approx. Max Chars (worst case)"), 0, 1, 2, 3);
  gtk_table_attach_defaults(text_table, adjust_textlen, 1, 2, 2, 3);
  gtk_table_attach_defaults(text_table, gtk_label_new("Font Size"), 0, 1, 3, 4);
  gtk_table_attach_defaults(text_table, adjust_textsize, 1, 2, 3, 4);
  gtk_table_attach_defaults(text_table, gtk_label_new("Border Width"), 0, 1, 4, 5);
  gtk_table_attach_defaults(text_table, adjust_borderwidth, 1, 2, 4, 5);

  gtk_box_pack_start(GTK_CONTAINER(vbox), gtk, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_CONTAINER(vbox), gtk_off_section, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(gtk_off_section), gtk_off_table, FALSE, FALSE, 0);

  gtk_table_attach_defaults(gtk_off_table, normal_label, 0, 1, 0, 1);
  gtk_table_attach_defaults(gtk_off_table, normal_bg, 1, 2, 0, 1);
  gtk_table_attach_defaults(gtk_off_table, normal_fg, 2, 3, 0, 1);
  gtk_table_attach_defaults(gtk_off_table, normal_ex, 3, 4, 0, 1);

  gtk_table_attach_defaults(gtk_off_table, hover_label, 0, 1, 1, 2);
  gtk_table_attach_defaults(gtk_off_table, hover_bg, 1, 2, 1, 2);
  gtk_table_attach_defaults(gtk_off_table, hover_fg, 2, 3, 1, 2);
  gtk_table_attach_defaults(gtk_off_table, hover_ex, 3, 4, 1, 2);

  gtk_table_attach_defaults(gtk_off_table, border_label, 0, 1, 2, 3);
  gtk_table_attach_defaults(gtk_off_table, border_colour, 2, 3, 2, 3);

  gtk_table_attach_defaults(gtk_off_table, gtk_label_new("Gradient Factor"), 0, 1, 3, 4);
  gtk_table_attach_defaults(gtk_off_table, adjust_gradient, 2, 3, 3, 4);



  gtk_box_pack_start(GTK_CONTAINER(vbox), buttons, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_CONTAINER(buttons), ok, FALSE, FALSE, 0);
  gtk_widget_show_all(prefs_win);

  if (G_cairo_menu_conf.honour_gtk)
  {
    gtk_widget_hide(gtk_off_section);
  }

}

