/*
 * Copyright (C) 2008 Rodney Cryderman <rcryderman@gmail.com>
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

#ifndef __WEBAPPLET_APPLET
#define __WEBAPPLET_APPLET

#include <libawn/libawn.h>

typedef struct
{
  AwnApplet       *applet;
  GtkWidget       *mainwindow;
  GdkPixbuf       *icon;
  GtkWidget       *box;
  GtkWidget       *viewer;
  GtkWidget       *entry;
  GtkWidget       *start;
  GtkWidget       *check_home;
  GtkWidget       *location_dialog;

  DesktopAgnosticConfigClient *config;

  GKeyFile        *sites_file;

  gint             applet_icon_height;
  gchar           *applet_icon_name;
  gchar           *uid;

} WebApplet;

#define APPLET_NAME "webapplet"

#endif
/* vim: set et ts=2 sts=2 sw=2 : */
