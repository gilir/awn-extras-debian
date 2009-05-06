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
#undef NDEBUG
#include <assert.h>
#include <gdk/gdk.h>
#include <libawn/awn-cairo-utils.h>
#include <libsexy/sexy-icon-entry.h>
#include <string.h>

#include "menu.h"
#include "menu_list_item.h"
#include "render.h"

extern Cairo_menu_config G_cairo_menu_conf;

extern AwnApplet *G_applet;
//GtkWidget * G_Fixed;
GtkWidget * G_toplevel = NULL;
//GtkWidget * G_mainwindow;
gboolean  G_repression = FALSE;
extern Win_man * G_win_man;
gboolean G_cancel_hide_all = TRUE;
int G_max_width = -1;
extern gint G_Height;
typedef struct
{
  GtkWidget * subwidget;
  GtkWidget * normal;
  GtkWidget * hover;
  GtkWidget * click;
  GtkWidget * misc;
  GtkWidget * aux;
}Mouseover_data;

Mouseover_data * G_Search = NULL;
Mouseover_data * G_Run = NULL;

/*
At some point in time it might be good to cache cr and gradient.
At this point it's a bit inefficient on initial startup... but that's the only
time the code is run.
*/
GtkWidget * build_menu_widget(Menu_item_color * mic, char * text, GdkPixbuf *pbuf, GdkPixbuf *pover, int max_width)
{
  //printf("build_menu_widget\n");
  static cairo_t *cr = NULL;
  GtkWidget * widget;
  GdkScreen* pScreen;
  GdkPixmap * pixmap;
  GdkColormap* cmap;
  cairo_pattern_t *gradient = NULL;
  cairo_text_extents_t    extents;
  gint pixmap_width = max_width;
  gint pixmap_height = G_cairo_menu_conf.text_size * 1.6;

  if (pbuf)
    if (gdk_pixbuf_get_height(pbuf) != G_cairo_menu_conf.text_size)
    {
      pbuf = gdk_pixbuf_scale_simple(pbuf, G_cairo_menu_conf.text_size, G_cairo_menu_conf.text_size, GDK_INTERP_HYPER);
    }
    else
    {
      gdk_pixbuf_ref(pbuf);
    }

  if (pover)
    if (gdk_pixbuf_get_height(pover) != G_cairo_menu_conf.text_size*0.7)
    {
      pover = gdk_pixbuf_scale_simple(pover, G_cairo_menu_conf.text_size * 0.7, G_cairo_menu_conf.text_size * 0.7, GDK_INTERP_HYPER);
    }
    else
    {
      gdk_pixbuf_ref(pover);
    }


  pixmap = gdk_pixmap_new(NULL, pixmap_width,

                          G_cairo_menu_conf.text_size * 1.6, 32);   //FIXME
  widget = gtk_image_new_from_pixmap(pixmap, NULL);
  pScreen = gtk_widget_get_screen(G_toplevel);
  cmap = gdk_screen_get_rgba_colormap(pScreen);

  if (!cmap)
    cmap = gdk_screen_get_rgb_colormap(pScreen);

  gdk_drawable_set_colormap(pixmap, cmap);

  cr = gdk_cairo_create(pixmap);

  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);

  cairo_paint(cr);

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

  gradient = cairo_pattern_create_linear(0, 0, 0,
                                         G_cairo_menu_conf.text_size * 1.6);

  cairo_pattern_add_color_stop_rgba(gradient, 0,  mic->bg.red, mic->bg.green, mic->bg.blue,
                                    mic->bg.alpha*G_cairo_menu_conf.menu_item_gradient_factor);

  cairo_pattern_add_color_stop_rgba(gradient, 0.2, mic->bg.red, mic->bg.green, mic->bg.blue,
                                    mic->bg.alpha);

  cairo_pattern_add_color_stop_rgba(gradient, 0.8, mic->bg.red, mic->bg.green, mic->bg.blue,
                                    mic->bg.alpha);

  cairo_pattern_add_color_stop_rgba(gradient, 1, mic->bg.red, mic->bg.green, mic->bg.blue,
                                    mic->bg.alpha*G_cairo_menu_conf.menu_item_gradient_factor);

  cairo_set_source(cr, gradient);

  cairo_paint(cr);


  if (pbuf)
  {
    gdk_cairo_set_source_pixbuf(cr, pbuf, G_cairo_menu_conf.text_size*0.3, G_cairo_menu_conf.text_size*0.2);
    cairo_rectangle(cr, 0, 0, G_cairo_menu_conf.text_size*1.3,
                    G_cairo_menu_conf.text_size*1.2);
    cairo_fill(cr);

    if (pover)
    {
      gdk_cairo_set_source_pixbuf(cr, pover, G_cairo_menu_conf.text_size*0.5, G_cairo_menu_conf.text_size*0.4);
      cairo_rectangle(cr, 0, 0, G_cairo_menu_conf.text_size*1.3,
                      G_cairo_menu_conf.text_size*1.2);
      cairo_fill(cr);
    }
  }
  else if (pover)
  {
    gdk_cairo_set_source_pixbuf(cr, pover, G_cairo_menu_conf.text_size*0.3, G_cairo_menu_conf.text_size*0.2);
    cairo_rectangle(cr, 0, 0, G_cairo_menu_conf.text_size*1.3,
                    G_cairo_menu_conf.text_size*1.2);
    cairo_fill(cr);
  }

  if (G_cairo_menu_conf.border_width > 0)
  {
    cairo_set_source_rgba(cr, G_cairo_menu_conf.border_colour.red,
                          G_cairo_menu_conf.border_colour.green,
                          G_cairo_menu_conf.border_colour.blue,
                          G_cairo_menu_conf.border_colour.alpha);
    cairo_set_line_width(cr, G_cairo_menu_conf.border_width);
    cairo_move_to(cr, G_cairo_menu_conf.border_width / 2, 0);
    cairo_line_to(cr, G_cairo_menu_conf.border_width / 2, pixmap_height);
    cairo_stroke(cr);
    cairo_move_to(cr, pixmap_width - G_cairo_menu_conf.border_width / 2, 0);
    cairo_line_to(cr, pixmap_width - G_cairo_menu_conf.border_width / 2, pixmap_height);
    cairo_stroke(cr);
  }

  cairo_set_source_rgba(cr, mic->fg.red, mic->fg.green, mic->fg.blue, mic->fg.alpha);

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_move_to(cr, G_cairo_menu_conf.text_size*1.4 , G_cairo_menu_conf.text_size*1.1);
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, G_cairo_menu_conf.text_size);

  char * buf;
  int  nul_pos = strlen(text);
  buf = g_malloc(nul_pos + 3);
  strcpy(buf, text);
  cairo_text_extents(cr, buf, &extents);

  while ((nul_pos > 5) &&
         (extents.width +  G_cairo_menu_conf.text_size*1.3 >
          pixmap_width - G_cairo_menu_conf.text_size)
        )
  {
    nul_pos--;
    buf[nul_pos] = '\0';
    strcat(buf, "..."); /*good enough*/
    cairo_text_extents(cr, buf, &extents);
  }

  cairo_show_text(cr, buf);

  g_free(buf);
  cairo_destroy(cr);

  if (gradient)
    cairo_pattern_destroy(gradient);

  if (pbuf)
    g_object_unref(pbuf);

  if (pover)
    g_object_unref(pover);

  return widget;
}


void render_blank(Menu_list_item *entry, int max_width)
{
  //printf("render_blank\n");
  GtkIconTheme*  g;
  static cairo_t *cr = NULL;
  GdkScreen* pScreen;
  GdkPixmap * pixmap;
  GdkColormap* cmap;

  if (G_cairo_menu_conf.border_width > 0)
  {
    pixmap = gdk_pixmap_new(NULL, max_width,
                            G_cairo_menu_conf.border_width, 32);  //FIXME
  }
  else
  {
    pixmap = gdk_pixmap_new(NULL, max_width,
                            1, 32);  //FIXME

  }

  entry->widget = gtk_image_new_from_pixmap(pixmap, NULL);

  pScreen = gtk_widget_get_screen(G_toplevel);
  cmap = gdk_screen_get_rgba_colormap(pScreen);

  if (!cmap)
    cmap = gdk_screen_get_rgb_colormap(pScreen);

  gdk_drawable_set_colormap(pixmap, cmap);

  cr = gdk_cairo_create(pixmap);

  if (G_cairo_menu_conf.border_width > 0)
  {
    cairo_set_source_rgba(cr, G_cairo_menu_conf.border_colour.red,
                          G_cairo_menu_conf.border_colour.green,
                          G_cairo_menu_conf.border_colour.blue,
                          G_cairo_menu_conf.border_colour.alpha);
  }
  else
  {
    cairo_set_source_rgba(cr, G_cairo_menu_conf.normal.bg.red,
                          G_cairo_menu_conf.normal.bg.green,
                          G_cairo_menu_conf.normal.bg.blue,
                          G_cairo_menu_conf.normal.bg.alpha);
  }

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

  cairo_paint(cr);
  cairo_destroy(cr);
  g_object_unref(pixmap);

}

void render_separator(Menu_list_item *entry, int max_width)
{
  //printf("render_separator\n");
  GtkIconTheme*  g;
  static cairo_t *cr = NULL;
  GdkScreen* pScreen;
  GdkPixmap * pixmap;
  GdkColormap* cmap;

  pixmap = gdk_pixmap_new(NULL, max_width,
                          1, 32);  //FIXME
  entry->widget = gtk_image_new_from_pixmap(pixmap, NULL);
  pScreen = gtk_widget_get_screen(G_toplevel);
  cmap = gdk_screen_get_rgba_colormap(pScreen);

  if (!cmap)
    cmap = gdk_screen_get_rgb_colormap(pScreen);

  gdk_drawable_set_colormap(pixmap, cmap);

  cr = gdk_cairo_create(pixmap);

  cairo_set_source_rgba(cr, G_cairo_menu_conf.border_colour.red,
                        G_cairo_menu_conf.border_colour.green,
                        G_cairo_menu_conf.border_colour.blue,
                        G_cairo_menu_conf.border_colour.alpha);

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

  cairo_paint(cr);

  cairo_destroy(cr);

//    g_object_unref(pixmap);

}

void render_textentry(Menu_list_item *entry, int max_width)
{
  //printf("render_textentry\n");
  GtkIconTheme*  g;
  GdkPixbuf *pbuf = NULL;

// gtk_image_new_from_stock(GTK_STOCK_EXECUTE,GTK_ICON_SIZE_MENU);
  g = gtk_icon_theme_get_default();

  pbuf = gtk_icon_theme_load_icon(g, entry->icon, G_cairo_menu_conf.text_size, 0, NULL);

  if (!pbuf)
  {
    pbuf = gdk_pixbuf_new_from_file_at_size(entry->icon, -1, G_cairo_menu_conf.text_size, NULL);
  }

  entry->widget = gtk_event_box_new();

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(entry->widget), FALSE);
  gtk_event_box_set_above_child(GTK_EVENT_BOX(entry->widget), TRUE);
  entry->normal = build_menu_widget(&G_cairo_menu_conf.normal, entry->name, pbuf, NULL, max_width);
  entry->hover = build_menu_widget(&G_cairo_menu_conf.hover, entry->name, pbuf, NULL, max_width);
  entry->text_entry = sexy_icon_entry_new();
  sexy_icon_entry_set_icon((SexyIconEntry *)entry->search_entry, SEXY_ICON_ENTRY_PRIMARY,
                           GTK_IMAGE(gtk_image_new_from_pixbuf(pbuf)));
  sexy_icon_entry_add_clear_button((SexyIconEntry *)entry->search_entry);
  g_object_ref(entry->normal);
  gtk_container_add(GTK_CONTAINER(entry->widget), entry->normal);

  if (pbuf)
    g_object_unref(pbuf);
}


void render_entry(Menu_list_item *entry, int max_width)
{
  //printf("render_entry\n");
  GtkIconTheme*  g;
  GdkPixbuf *pbuf = NULL;
  g = gtk_icon_theme_get_default();

  pbuf = gtk_icon_theme_load_icon(g, entry->icon, G_cairo_menu_conf.text_size, 0, NULL);

  if (!pbuf)
  {
    pbuf = gdk_pixbuf_new_from_file_at_size(entry->icon, -1, G_cairo_menu_conf.text_size, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, entry->name, G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, entry->exec, G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.svg", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.png", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.xpm", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, "applications-other", G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, "application-x-executable", G_cairo_menu_conf.text_size, 0, NULL);
  }

  entry->widget = gtk_event_box_new();

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(entry->widget), FALSE);
  gtk_event_box_set_above_child(GTK_EVENT_BOX(entry->widget), TRUE);
  entry->normal = build_menu_widget(&G_cairo_menu_conf.normal, entry->name, pbuf, NULL, max_width);
  entry->hover = build_menu_widget(&G_cairo_menu_conf.hover, entry->name, pbuf, NULL, max_width);

  if (G_cairo_menu_conf.on_button_release)
  {
    entry->click = build_menu_widget(&G_cairo_menu_conf.hover, entry->name, pbuf, NULL, max_width);
  }

  g_object_ref(entry->normal);

  gtk_container_add(GTK_CONTAINER(entry->widget), entry->normal);

  if (pbuf)
    g_object_unref(pbuf);
}


void render_directory(Menu_list_item *directory, int max_width)
{
  //printf("render_directory\n");
  GtkIconTheme*  g;
  GdkPixbuf *pbuf1 = NULL;
  GdkPixbuf *pbuf2 = NULL;
  GdkPixbuf *pbuf_over = NULL;
  g = gtk_icon_theme_get_default();

  if (!pbuf1)
  {
    pbuf1 = gtk_icon_theme_load_icon(g, "stock_folder", G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf1)
  {
    pbuf1 = gdk_pixbuf_new_from_file_at_size("folder", -1, G_cairo_menu_conf.text_size, NULL);
  }

  pbuf2 = gtk_icon_theme_load_icon(g, "stock_open", G_cairo_menu_conf.text_size, 0, NULL);

  if (!pbuf2)
  {
    pbuf2 = gdk_pixbuf_new_from_file_at_size("folder_open", -1, G_cairo_menu_conf.text_size, NULL);
  }

  if (!pbuf2)
  {
    pbuf2 = gdk_pixbuf_new_from_file_at_size("folder-open", -1, G_cairo_menu_conf.text_size, NULL);
  }

  if (!pbuf1)
    pbuf1 = pbuf2;

  if (!pbuf2)
    pbuf2 = pbuf1;

  if (directory->icon)
  {
    pbuf_over = gtk_icon_theme_load_icon(g, directory->icon, G_cairo_menu_conf.text_size, 0, NULL);
  }

  directory->widget = gtk_event_box_new();

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(directory->widget), FALSE);
  gtk_event_box_set_above_child(GTK_EVENT_BOX(directory->widget), TRUE);
  directory->normal = build_menu_widget(&G_cairo_menu_conf.normal, directory->name, pbuf1, pbuf_over, max_width);
  directory->hover = build_menu_widget(&G_cairo_menu_conf.hover, directory->name, pbuf2, pbuf_over, max_width);
  g_object_ref(directory->normal);
  gtk_container_add(GTK_CONTAINER(directory->widget), directory->normal);

  if (pbuf1)
    g_object_unref(pbuf1);

  if ((pbuf2) && (pbuf1 != pbuf2))
    g_object_unref(pbuf2);

  if (pbuf_over)
    g_object_unref(pbuf_over);

}

void render_drive(Menu_list_item *entry, int max_width)
{
  //printf("render_drive\n");
  GtkIconTheme*  g;
  GdkPixbuf *pbuf = NULL;
  GdkPixbuf *pbuf_over = NULL;
  g = gtk_icon_theme_get_default();

  pbuf = gtk_icon_theme_load_icon(g, entry->icon, G_cairo_menu_conf.text_size, 0, NULL);

  if (!pbuf)
  {
    pbuf = gdk_pixbuf_new_from_file_at_size(entry->icon, -1, G_cairo_menu_conf.text_size, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, entry->name, G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, entry->exec, G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.svg", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.png", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    gchar * filename;
    filename = g_strdup_printf("/usr/share/pixmaps/%s.xpm", entry->icon);
    pbuf = gdk_pixbuf_new_from_file_at_size(filename, -1, G_cairo_menu_conf.text_size, NULL);
    g_free(filename);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, "applications-other", G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (!pbuf)
  {
    pbuf = gtk_icon_theme_load_icon(g, "application-x-executable", G_cairo_menu_conf.text_size, 0, NULL);
  }

  if (entry->drive_mount)
    pbuf_over = gtk_icon_theme_load_icon(g, "important", G_cairo_menu_conf.text_size, 0, NULL);

  entry->widget = gtk_event_box_new();

  gtk_event_box_set_visible_window(GTK_EVENT_BOX(entry->widget), FALSE);

  gtk_event_box_set_above_child(GTK_EVENT_BOX(entry->widget), TRUE);

  entry->normal = build_menu_widget(&G_cairo_menu_conf.normal, entry->name, pbuf, pbuf_over, max_width);

  entry->hover = build_menu_widget(&G_cairo_menu_conf.hover, entry->name, pbuf, pbuf_over, max_width);

  g_object_ref(entry->normal);

  gtk_container_add(GTK_CONTAINER(entry->widget), entry->normal);

  if (pbuf)
    g_object_unref(pbuf);

  if (pbuf_over)
    g_object_unref(pbuf_over);

}

void _fixup_menus(GtkWidget * node, GtkWidget * subwidget)
{

  static int maxheight = -1;

  if (maxheight == -1)
    maxheight = gdk_screen_get_height(gdk_screen_get_default()) * 0.60;

  if (!subwidget)
  {
    if (node != G_toplevel)
    {
      gtk_widget_hide(node->parent->parent);
      return;
    }
  }

  if (node->allocation.height > maxheight)
  {
    maxheight = node->allocation.height;
  }

  if (node != G_toplevel)
  {
    if (node != subwidget)
    {
      gboolean found = FALSE;
      void * ptr = g_tree_lookup(G_cairo_menu_conf.submenu_deps, subwidget);

      for (;ptr != G_toplevel;ptr = g_tree_lookup(G_cairo_menu_conf.submenu_deps, ptr))
      {
        if (ptr == node)
        {
          found = TRUE;
        }
      }

      if (!found)
      {
        gtk_widget_hide(node->parent->parent);
      }
    }
    else
    {
      gtk_widget_show(subwidget->parent->parent);
      gtk_widget_show(subwidget->parent);
      gtk_widget_show(subwidget);
    }
  }
}


GtkWidget *G_cur_menu = NULL;


static gboolean _delayed_show_menu(GtkWidget * subwidget)
{

  if ((G_cur_menu != subwidget) && subwidget)
  {
    return FALSE;
  }

  if (subwidget != G_toplevel)
  {
    g_list_foreach(G_win_man->children, (GFunc)_fixup_menus, subwidget);
  }
  else
  {
    g_list_foreach(G_win_man->children, (GFunc)_fixup_menus, NULL);
  }

  if (gtk_window_get_opacity(GTK_WINDOW(subwidget->parent->parent)) < 0.99)
  {
    gtk_window_set_opacity(GTK_WINDOW(subwidget->parent->parent), 1.0);
  }

  gtk_widget_show(subwidget);

  gtk_widget_show(subwidget->parent);
  gtk_widget_show(subwidget->parent->parent);
  gtk_widget_grab_focus(subwidget->parent->parent);
  G_cur_menu = NULL;
  //printf("EXIT _delayed_show_menu\n");
  return FALSE;
}

static gboolean _enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, Mouseover_data *data)
{
  //printf("_enter_notify_event\n");
  int maxheight = gdk_screen_get_height(gdk_screen_get_default()) * 0.80;
  maxheight = maxheight / (double)(G_cairo_menu_conf.text_size * 1.6);
  maxheight = maxheight * (G_cairo_menu_conf.text_size * 1.6) + 2 * G_cairo_menu_conf.border_width;

  G_cancel_hide_all = TRUE;

  if ((gtk_bin_get_child(GTK_BIN(widget)) == data->misc) || G_repression)
  {
    return FALSE;
  }

  GtkWidget * subwidget = data->subwidget;

  g_object_ref(data->hover);
  gtk_container_remove(GTK_CONTAINER(widget), gtk_bin_get_child(GTK_BIN(widget)));
  gtk_container_add(GTK_CONTAINER(widget), data->hover);
  gtk_widget_show_all(data->hover);

  if (!GTK_WIDGET_REALIZED(subwidget)) //keeps blank menus from occuring..
  {
    g_timeout_add(500, (GSourceFunc)_delayed_show_menu, G_cur_menu = subwidget);
  }
  else
    g_timeout_add(50, (GSourceFunc)_delayed_show_menu, G_cur_menu = subwidget);

  gint xpos_sub, tmp, ypos_sub;

  gdk_window_get_origin(widget->parent->parent->window, &xpos_sub, &ypos_sub);

  //printf("w = %d , h = %d \n",subwidget->allocation.width,subwidget->allocation.height);

  if (!GTK_WIDGET_REALIZED(subwidget)) //keeps blank menus from occuring..
  {
    gtk_widget_realize(subwidget);
    gtk_widget_realize(subwidget->parent);
    gtk_widget_realize(subwidget->parent->parent);
    gtk_widget_show_all(subwidget->parent->parent);
    gtk_widget_hide(subwidget->parent->parent);
  }

  if (subwidget->allocation.height < subwidget->parent->allocation.height)
  {
    gtk_widget_set_size_request(subwidget, -1, subwidget->allocation.height);
    gtk_widget_set_size_request(subwidget->parent, -1, subwidget->allocation.height);
    gtk_widget_set_size_request(subwidget->parent->parent, -1, subwidget->allocation.height);
    gtk_window_resize(GTK_WINDOW(subwidget->parent->parent), subwidget->allocation.width, subwidget->allocation.height);
  }

  if (subwidget->allocation.height > maxheight)
  {
    gtk_widget_set_size_request(subwidget, -1, maxheight);
    gtk_widget_set_size_request(subwidget->parent, -1, maxheight);
    gtk_widget_set_size_request(subwidget->parent->parent, -1, maxheight);
    gtk_window_resize(GTK_WINDOW(subwidget->parent->parent), subwidget->allocation.width, maxheight);
  }

  tmp = xpos_sub;

  xpos_sub = xpos_sub + widget->allocation.width * 0.99;
  ypos_sub = widget->allocation.y + ypos_sub - widget->allocation.height / 3;

  if (ypos_sub + subwidget->allocation.height > G_win_man->y + G_win_man->height
      - GTK_WIDGET(G_applet)->allocation.width / 2)
  {
    ypos_sub = G_win_man->y + G_win_man->height - subwidget->allocation.height-G_Height;
    gtk_window_move(GTK_WINDOW(subwidget->parent->parent), xpos_sub, ypos_sub);
  }
  else
  {
    gtk_window_move(GTK_WINDOW(subwidget->parent->parent), xpos_sub, ypos_sub);
  }

  if ((xpos_sub + subwidget->allocation.width > G_win_man->x + G_win_man->width) ||
      (xpos_sub < widget->allocation.x + widget->allocation.width * 0.90) )
  {
/*    g_debug ("attempting to adjust submenu\n");*/
    /*This appears to be causing the overlapping menus...*/
    if (subwidget->allocation.width < tmp)
    {
/*      g_debug ("subwidget->allocation.width = %d, tmp = %d\n",subwidget->allocation.width,tmp);*/
      xpos_sub = tmp - subwidget->allocation.width*0.95;
      gtk_window_move(GTK_WINDOW(subwidget->parent->parent), xpos_sub, ypos_sub);
    }
  }

  return TRUE;
}



static gboolean _enter_notify_event_entry(GtkWidget *widget, GdkEventCrossing *event, Mouseover_data *data)
{
  //printf("_enter_notify_event_entry\n");
  G_cancel_hide_all = TRUE;
  G_cur_menu = NULL;

  if ((gtk_bin_get_child(GTK_BIN(widget)) != data->misc) && ! G_repression)
  {
    GtkWidget * subwidget = data->subwidget;

    g_object_ref(data->hover);
    gtk_container_remove(GTK_CONTAINER(widget), gtk_bin_get_child(GTK_BIN(widget)));
    gtk_container_add(GTK_CONTAINER(widget), data->hover);
    gtk_widget_show_all(widget);
    g_timeout_add(100, (GSourceFunc) _delayed_show_menu, G_cur_menu = subwidget);
    return TRUE;
  }

  return FALSE;
}

static gboolean _leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, Mouseover_data *data)
{
  //printf("_leave_notify_event\n");
  G_cur_menu = NULL;

  if ((gtk_bin_get_child(GTK_BIN(widget)) != data->misc) && ! G_repression)
  {
    // gtk_widget_show(data->subwidget);
    g_object_ref(data->normal);
    gtk_container_remove(GTK_CONTAINER(widget), gtk_bin_get_child(GTK_BIN(widget)));
    gtk_container_add(GTK_CONTAINER(widget), data->normal);
    gtk_widget_show_all(widget);
    return TRUE;
  }

  return FALSE;
}

static gboolean _leave_notify_event_entry(GtkWidget *widget, GdkEventCrossing *event, Mouseover_data *data)
{
  //printf("_leave_notify_event_entry\n");
  G_cur_menu = NULL;

  if ((gtk_bin_get_child(GTK_BIN(widget)) != data->misc) && ! G_repression)
  {
    gtk_widget_show(data->subwidget);
    g_object_ref(data->normal);
    gtk_container_remove(GTK_CONTAINER(widget), gtk_bin_get_child(GTK_BIN(widget)));
    gtk_container_add(GTK_CONTAINER(widget), data->normal);
    gtk_widget_show_all(widget);
    return TRUE;
  }

  return FALSE;
}

gboolean hide_menu_on_a_timer(GtkWidget * widget)
{
  //printf("hide_menu_on_a_timer\n");
  gtk_widget_hide(widget);
  return FALSE;
}

//FIXME the following needs to be placed into the gnome backend.

static gboolean _add_to_bar(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  //printf("add_to_bar\n");
  append_to_launchers(menu_item->desktop);
  G_repression = FALSE;
  hide_all_menus();
  return TRUE;
}


static gboolean _button_do_event(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  GError *err = NULL;
  //printf("_button_do_event\n");

  if (event->button == 1)
  {
    G_repression = FALSE;
    hide_all_menus();
    printf("spawn=%s\n", menu_item->exec);
    g_spawn_command_line_async(menu_item->exec, &err);
    hide_all_menus();
    g_timeout_add(250, (GSourceFunc) hide_menu_on_a_timer, widget->parent->parent);
  }
  else if (event->button == 3)
  {
    static GtkWidget  *G_right_menu = NULL;
    GtkWidget *item;

    if (G_right_menu)
      gtk_widget_destroy(G_right_menu);

    G_right_menu = gtk_menu_new();

    item = gtk_menu_item_new_with_label("Add to Bar");

    gtk_widget_show(item);

    gtk_menu_shell_append(GTK_MENU_SHELL(G_right_menu), item);

    g_signal_connect(G_OBJECT(item), "button-press-event", G_CALLBACK(_add_to_bar), menu_item);

    G_repression = TRUE;

    gtk_menu_popup(GTK_MENU(G_right_menu), NULL, NULL, NULL, NULL,
                   event->button, event->time);

  }

  return TRUE;
}

void hide_search(void)
{
  //printf("hide_search\n");
  if (G_Search)
  {
    if (gtk_bin_get_child(GTK_BIN(G_Search->aux)) == G_Search->misc)
    {
      g_object_ref(gtk_bin_get_child(GTK_BIN(G_Search->aux)));
      gtk_container_remove(GTK_CONTAINER(G_Search->aux), gtk_bin_get_child(GTK_BIN(G_Search->aux)));
      gtk_container_add(GTK_CONTAINER(G_Search->aux), G_Search->normal);
      gtk_widget_show_all(G_Search->normal);
    }
  }
}


static gboolean _focus_out_search(GtkWidget *widget, GdkEventButton *event, Mouseover_data *data)
{
  //printf("_focus_out_search\n");
  hide_search();
  G_repression = FALSE;
// printf(" _focus_out_search\n");
  return FALSE;
}

int activate_search(GtkWidget *w, Menu_list_item * menu_item)
{
  GError *err = NULL;
  char  *cmd;
//printf("activate_search\n");
  G_repression = FALSE;
  cmd = g_malloc(strlen(G_cairo_menu_conf.search_cmd) + strlen(gtk_entry_get_text(w)) + 4);
  strcpy(cmd, G_cairo_menu_conf.search_cmd);
  strcat(cmd, " '");
  strcat(cmd, gtk_entry_get_text(w));
  strcat(cmd, "'");
  g_spawn_command_line_async(cmd, &err);
  g_free(cmd);
  hide_all_menus();
  return FALSE;
}

static _eject(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  //printf("_eject\n");
  backend_unmount(menu_item);
  G_repression = FALSE;
  hide_all_menus();
  return FALSE;
}

static _unmount(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  //printf("_unmount\n");
  backend_unmount(menu_item);
  G_repression = FALSE;
  return FALSE;
}

static void hide_textentries(void)
{
  //printf("hide_textentries\n");
  static gboolean block_reenter = FALSE;

  if (block_reenter)
    return;

  block_reenter = TRUE;

  if (G_Search)
  {
    if (gtk_bin_get_child(G_Search->aux) == G_Search->misc)
    {
      gtk_container_remove(G_Search->aux, gtk_bin_get_child(G_Search->aux));
      gtk_container_add(G_Search->aux, G_Search->normal);
      gtk_widget_show_all(G_Search->normal);
    }
  }

  if (G_Run)
  {
    if (gtk_bin_get_child(G_Run->aux) == G_Run->misc)
    {
      gtk_container_remove(G_Run->aux, gtk_bin_get_child(G_Run->aux));
      gtk_container_add(G_Run->aux, G_Run->normal);
      gtk_widget_show_all(G_Run->normal);
    }
  }

  block_reenter = FALSE;
}

static gboolean _button_clicked_ignore(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  //printf("_button_clicked_ignore\n");
  G_repression = FALSE;
  hide_textentries();
  return TRUE;
}

static gboolean _button_do_event_textentry(GtkWidget *widget, GdkEventButton *event, Mouseover_data *data)
{
  //printf("_button_do_event_textentry\n");
  hide_textentries();
  g_list_foreach(G_win_man->children, _fixup_menus, NULL);
  gtk_widget_show(data->misc);
  g_object_ref(data->misc);
  gtk_container_remove(widget, gtk_bin_get_child(widget));
  gtk_container_add(widget, data->misc);
  G_repression = TRUE;
  gtk_widget_show_all(widget);
// printf("_button_do_event_textentry\n");
  assert(GTK_WIDGET_CAN_FOCUS(data->misc));
  gtk_widget_grab_focus(data->misc);
  return FALSE;
}

static gboolean _focus_out_textentry(GtkWidget *widget, GdkEventButton *event, Mouseover_data *data)
{
  //printf("_focus_out_textentry\n");
  hide_textentries();
  G_repression = FALSE;
  return FALSE;
}


int activate_run(GtkWidget *w, Menu_list_item * menu_item)
{
  //printf("activate_run\n");
  GError *err = NULL;
  G_repression = FALSE;
  g_spawn_command_line_async(gtk_entry_get_text(w), &err);
  gtk_entry_set_text(w, "") ;
  hide_all_menus();
  return FALSE;
}

static gboolean _button_do_drive_event(GtkWidget *widget, GdkEventButton *event, Menu_list_item * menu_item)
{
  //printf("_button_do_drive_event\n");
  GError *err = NULL;
  gchar * cmd;

  if (event->button == 1)
  {
    hide_all_menus();

    if (menu_item->drive_prep)
    {
      if (!menu_item->drive_prep(menu_item, G_cairo_menu_conf.filemanager))
      {
        //drive_prep failed. or it has taken care of opening file (if mount async op)
        return TRUE;
      }
    }
    else
    {
      cmd = g_strdup_printf("%s %s", G_cairo_menu_conf.filemanager, menu_item->mount_point);
      g_spawn_command_line_async(cmd, &err);
      g_free(cmd);
    }
  }
  else if (event->button == 3)
  {
    static gboolean done_once = FALSE;
    static GtkWidget * menu = NULL;
    static GtkWidget * item;
    G_repression = TRUE;

    if (menu)
    {
      gtk_widget_destroy(menu);
    }

    menu = gtk_menu_new();

    if (!menu_item->drive_prep)
    {
      item = gtk_menu_item_new_with_label("Unmount");
      gtk_widget_show(item);
      gtk_menu_shell_append(menu, item);
      g_signal_connect(G_OBJECT(item), "button-press-event", G_CALLBACK(_unmount), menu_item);
    }

    item = gtk_menu_item_new_with_label("Eject");

    gtk_widget_show(item);
    gtk_menu_shell_append(menu, item);
    g_signal_connect(G_OBJECT(item), "button-press-event", G_CALLBACK(_eject), menu_item);
    gtk_menu_set_screen(menu, NULL);
    gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                   event->button, event->time);
    return FALSE;
  }

  return TRUE;
}



static gboolean _scroll_event(GtkWidget *widget, GdkEventMotion *event, GtkWidget * box)
{
  //printf("_scroll_event\n");
  GtkBoxChild *boxchild;

  if (event->type == GDK_SCROLL)
  {
    GList*  node;
    gint first = 1;
    gint last = g_list_length(GTK_BOX(box)->children) - 2;

    if (event->state & GDK_SHIFT_MASK)
    {
      do
      {
        GList * second;

        if (last > 4)
        {
          second = g_list_next(g_list_first(GTK_BOX(box)->children));
          boxchild = second->data;
          gtk_box_reorder_child(GTK_BOX(box), boxchild->widget, last);
        }
        else
          break;
      }
      while (boxchild->widget->allocation.height < G_cairo_menu_conf.text_size);
    }
    else
    {
      do
      {
        GList *secondlast;

        if (last > 4)
        {
          secondlast = g_list_previous(g_list_last(GTK_BOX(box)->children));
          boxchild = secondlast->data;
          gtk_box_reorder_child(GTK_BOX(box), boxchild->widget, first);
        }
        else
          break;
      }
      while (boxchild->widget->allocation.height < G_cairo_menu_conf.text_size);
    }

    gtk_widget_show_all(widget);
  }

  return TRUE;
}

gboolean _hide_all_windows(gpointer null)
{
  //printf("_hide_all_windows\n");
  g_list_foreach(G_win_man->children, _fixup_menus, NULL);
  return FALSE;
}


static gboolean _focus_out_window(GtkWidget *widget, GdkEventButton *event, Mouseover_data *data)
{
  //printf("_focus_out_window\n");
  G_cancel_hide_all = FALSE;
  return FALSE;
}

void rerender(Menu_list_item ** menu_items, GtkWidget *box)
{
  //printf("rerender\n");
  gtk_container_foreach(box, gtk_widget_destroy, NULL);
  g_slist_foreach(*menu_items, render_menu_widgets, box);
  gtk_widget_show_all(box);

}

void measure_width(Menu_list_item * menu_item, int * max_width)
{
  //printf("_measure_width\n");
  static cairo_t *cr = NULL;
  static cairo_surface_t*  surface;
  cairo_text_extents_t    extents;

  if (!cr)
  {
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, G_cairo_menu_conf.text_size * G_cairo_menu_conf.menu_item_text_len,
                                         G_cairo_menu_conf.text_size * 1.6);
    cr = cairo_create(surface);
  }

  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

  cairo_set_font_size(cr, G_cairo_menu_conf.text_size);
  cairo_text_extents(cr, menu_item->name, &extents);

  if (extents.width + G_cairo_menu_conf.text_size*1.5 > *max_width)
  {
    if (extents.width + G_cairo_menu_conf.text_size*1.5 > G_cairo_menu_conf.text_size*G_cairo_menu_conf.menu_item_text_len)
    {
      *max_width = G_cairo_menu_conf.text_size * G_cairo_menu_conf.menu_item_text_len;
    }
    else
    {
      *max_width = extents.width + G_cairo_menu_conf.text_size * 2.5;
    }
  }
}

void render_menu_widgets(Menu_list_item * menu_item, GtkWidget * box)
{
  //printf("render_menu_widgets\n");
  static int max_width = -1;
  static Xpos = 0;

  if (max_width == -1)
  {
    max_width = G_max_width;
  }

  switch (menu_item->item_type)
  {

    case MENU_ITEM_DIRECTORY:
    {
      int temp_width = max_width;
      GtkWidget *newbox;
      render_directory(menu_item, max_width);
      max_width = -1;
      g_slist_foreach(menu_item->sublist, measure_width, &max_width);
#if GTK_CHECK_VERSION(2,12,0)

      if (G_cairo_menu_conf.show_tooltips && menu_item->comment)
        gtk_widget_set_tooltip_text(menu_item->widget, menu_item->comment);

#endif
      newbox = menu_new(box->parent->parent);

      gtk_widget_set_app_paintable(newbox , TRUE);

      Xpos = Xpos + G_cairo_menu_conf.text_size * G_cairo_menu_conf.menu_item_text_len * 4 / 5;//FIXME

      fixed_put(newbox, Xpos, 0);

      g_slist_foreach(menu_item->sublist, render_menu_widgets, newbox);

      /*if monitor is set then we call it to initialize callbacks.
      whenever there is a change to the contents of this dir
      the first arg(rerender) will be called with an updated list
      of menu items and the vbox they are to be in*/
      if (menu_item->monitor)
      {
        menu_item->monitor(rerender, &menu_item->sublist, newbox);
      }

      Mouseover_data *data;

      data = g_malloc(sizeof(Mouseover_data));
      data->subwidget = newbox;
      data->normal = menu_item->normal;
      data->hover = menu_item->hover;
      data->misc = NULL;
      g_signal_connect(menu_item->widget, "enter-notify-event", G_CALLBACK(_enter_notify_event), data);
      g_signal_connect(menu_item->widget, "leave-notify-event", G_CALLBACK(_leave_notify_event), data);
      g_signal_connect(newbox, "scroll-event" , G_CALLBACK(_scroll_event), newbox);
      g_signal_connect(menu_item->widget, "button-press-event", G_CALLBACK(_button_clicked_ignore), data);
      g_tree_insert(G_cairo_menu_conf.submenu_deps, newbox, box);

      Xpos = Xpos - G_cairo_menu_conf.text_size * G_cairo_menu_conf.menu_item_text_len * 4 / 5;//FIXME
      g_signal_connect(G_OBJECT(newbox->parent->parent), "focus-out-event", G_CALLBACK(_focus_out_window), NULL);
      gtk_widget_show_all(newbox);
  /*    gtk_window_set_transient_for(newbox->parent->parent, box->parent->parent);*/
      gtk_widget_realize(newbox->parent->parent);
      max_width = temp_width;
    }

    break;

    case MENU_ITEM_ENTRY:
    {
      render_entry(menu_item, max_width);
#if GTK_CHECK_VERSION(2,12,0)

      if (G_cairo_menu_conf.show_tooltips && menu_item->comment)
        gtk_widget_set_tooltip_text(menu_item->widget, menu_item->comment);

#endif
      Mouseover_data *data;

      data = g_malloc(sizeof(Mouseover_data));

      data->subwidget = box;

      data->normal = menu_item->normal;

      data->hover = menu_item->hover;

      data->click = menu_item->click;

      data->misc = NULL;

      g_signal_connect(menu_item->widget, "enter-notify-event", G_CALLBACK(_enter_notify_event_entry), data);

      g_signal_connect(menu_item->widget, "leave-notify-event", G_CALLBACK(_leave_notify_event_entry), data);

      if (G_cairo_menu_conf.on_button_release)
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_clicked_ignore), data);
        g_signal_connect(G_OBJECT(menu_item->widget), "button-release-event", G_CALLBACK(_button_do_event), menu_item);
      }
      else
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_do_event), menu_item);
      }
    }

    break;

    case MENU_ITEM_DRIVE:
    {
      render_drive(menu_item, max_width);
#if GTK_CHECK_VERSION(2,12,0)

      if (G_cairo_menu_conf.show_tooltips && menu_item->comment)
        gtk_widget_set_tooltip_text(menu_item->widget, menu_item->comment);

#endif
      Mouseover_data *data;

      data = g_malloc(sizeof(Mouseover_data));

      data->subwidget = box;

      data->normal = menu_item->normal;

      data->hover = menu_item->hover;

      data->click = menu_item->click;

      data->misc = NULL;

      g_signal_connect(menu_item->widget, "enter-notify-event", G_CALLBACK(_enter_notify_event_entry), data);

      g_signal_connect(menu_item->widget, "leave-notify-event", G_CALLBACK(_leave_notify_event_entry), data);

#if 1
      if (G_cairo_menu_conf.on_button_release)
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_clicked_ignore), data);
        g_signal_connect(G_OBJECT(menu_item->widget), "button-release-event", G_CALLBACK(_button_do_drive_event), menu_item);
      }
      else
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_do_drive_event), menu_item);
      }

#endif
    }

    break;

    case MENU_ITEM_SEARCH:
    {
      render_textentry(menu_item, max_width);
#if GTK_CHECK_VERSION(2,12,0)

      if (G_cairo_menu_conf.show_tooltips && menu_item->comment)
        gtk_widget_set_tooltip_text(menu_item->widget, menu_item->comment);

#endif
      Mouseover_data *data;

      data = g_malloc(sizeof(Mouseover_data));

      data->subwidget = box;

      data->normal = menu_item->normal;

      data->hover = menu_item->hover;

      data->misc = menu_item->search_entry;

      data->aux = menu_item->widget;

      G_Search = data;

      g_signal_connect(menu_item->widget, "enter-notify-event", G_CALLBACK(_enter_notify_event_entry), data);

      g_signal_connect(menu_item->widget, "leave-notify-event", G_CALLBACK(_leave_notify_event_entry), data);

      if (G_cairo_menu_conf.on_button_release)
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_clicked_ignore), data);
        g_signal_connect(G_OBJECT(menu_item->widget), "button-release-event", G_CALLBACK(_button_do_event_textentry), data);
      }
      else
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_do_event_textentry), data);
      }

      g_signal_connect(G_OBJECT(data->misc), "focus-out-event", G_CALLBACK(_focus_out_textentry), data);

      g_signal_connect(G_OBJECT(data->misc), "activate", G_CALLBACK(activate_search), menu_item);

    }

    break;

    case MENU_ITEM_RUN:
    {
      render_textentry(menu_item, max_width);
#if GTK_CHECK_VERSION(2,12,0)

      if (G_cairo_menu_conf.show_tooltips && menu_item->comment)
        gtk_widget_set_tooltip_text(menu_item->widget, menu_item->comment);

#endif
      Mouseover_data *data;

      data = g_malloc(sizeof(Mouseover_data));

      data->subwidget = box;

      data->normal = menu_item->normal;

      data->hover = menu_item->hover;

      data->misc = menu_item->run_entry;

      data->aux = menu_item->widget;

      G_Run = data;

      g_signal_connect(menu_item->widget, "enter-notify-event", G_CALLBACK(_enter_notify_event_entry), data);

      g_signal_connect(menu_item->widget, "leave-notify-event", G_CALLBACK(_leave_notify_event_entry), data);

      if (G_cairo_menu_conf.on_button_release)
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event", G_CALLBACK(_button_clicked_ignore), data);
        g_signal_connect(G_OBJECT(menu_item->widget), "button-release-event", G_CALLBACK(_button_do_event_textentry),
                         data);
      }
      else
      {
        g_signal_connect(G_OBJECT(menu_item->widget), "button-press-event",
                         G_CALLBACK(_button_do_event_textentry),
                         data);
      }

      g_signal_connect(G_OBJECT(data->misc), "focus-out-event", G_CALLBACK(_focus_out_textentry), data);

      g_signal_connect(G_OBJECT(data->misc), "activate", G_CALLBACK(activate_run), menu_item);

    }

    break;

    case MENU_ITEM_SEPARATOR:
    {
      render_separator(menu_item, max_width);
      break;
    }

    case MENU_ITEM_BLANK:
    {
      render_blank(menu_item, max_width);
      break;
    }

    default:
      menu_item->widget = NULL;
  }

  if (menu_item->widget)
  {
    gtk_box_pack_start(box, menu_item->widget, FALSE, FALSE, 0);
    gtk_widget_show_all(menu_item->widget);
  }
}
