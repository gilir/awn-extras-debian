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


#ifndef __CAIRO_MENU_CONFIG_
#define __CAIRO_MENU_CONFIG_

#define GCONF_MENU "/apps/avant-window-navigator/applets/cairo-menu"

#define GCONF_NORMAL_BG GCONF_MENU "/bg_normal_colour"
#define GCONF_NORMAL_FG GCONF_MENU "/text_normal_colour"
#define GCONF_HOVER_BG GCONF_MENU "/bg_hover_colour"
#define GCONF_HOVER_FG GCONF_MENU "/text_hover_colour"

#define GCONF_TEXT_SIZE GCONF_MENU "/text_size"

#define GCONF_SEARCH_CMD GCONF_MENU "/search_cmd"
#define GCONF_SHOW_SEARCH GCONF_MENU "/search_show"
#define GCONF_SHOW_RUN GCONF_MENU "/run_show"

#define GCONF_DO_FADE GCONF_MENU "/fade_in"

#define GCONF_MENU_GRADIENT GCONF_MENU "/menu_item_gradient_factor"
#define GCONF_MENU_ITEM_TEXT_LEN GCONF_MENU "/menu_item_text_len"

#define GCONF_SHOW_PLACES GCONF_MENU "/places_show"

#define GCONF_FILEMANAGER GCONF_MENU "/filemanager"
#define GCONF_APPLET_ICON GCONF_MENU "/applet_icon"

#define GCONF_ON_BUTTON_RELEASE GCONF_MENU "/activate_on_release"
#define GCONF_SHOW_TOOLTIPS GCONF_MENU "/show_tooltips"

#define GCONF_SHOW_LOGOUT GCONF_MENU "/show_logout"
#define GCONF_LOGOUT GCONF_MENU "/logout"

#define GCONF_BORDER_COLOUR GCONF_MENU "/border_colour"
#define GCONF_BORDER_WIDTH GCONF_MENU "/border_width"

#define GCONF_HONOUR_GTK GCONF_MENU "/honour_gtk"

typedef struct
{
  AwnColor bg;
  AwnColor fg;

}Menu_item_color;

typedef struct
{
  Menu_item_color normal;
  Menu_item_color hover;
  Menu_item_color selected;
  AwnColor border_colour;
  int   border_width;
  int   text_size;
  GTree *  submenu_deps;
  gboolean show_search;
  gchar  *search_cmd;
  gboolean show_logout;
  gchar  *logout;

  gboolean show_run;
  gboolean do_fade;
  gboolean show_places;
  gchar   *filemanager;
  gchar  *applet_icon;
  int   menu_item_text_len;
  double   menu_item_gradient_factor;
  gboolean honour_gtk;
  gboolean  on_button_release;
  gboolean show_tooltips;
}Cairo_menu_config;

void read_config(void);
void show_prefs(void);
void append_to_launchers(gchar * launcher);

#endif

