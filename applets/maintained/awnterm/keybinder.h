//
// This file comes from the Tomboy project.
// http://www.gnome.org/projects/tomboy/
//
/*
 * Copyright (C) 2004-2007  Alex Graveley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __AWN_KEY_BINDER_H__
#define __AWN_KEY_BINDER_H__

#include <glib/gtypes.h>

G_BEGIN_DECLS

typedef void (*AwnBindkeyHandler) (const char *keystring, gpointer user_data);

void awn_keybinder_init (void);

gboolean awn_keybinder_bind (const char *keystring,
                             AwnBindkeyHandler  handler,
                             gpointer user_data);

gboolean awn_keybinder_unbind (const char *keystring,
                               AwnBindkeyHandler handler,
                               gpointer user_data);

gboolean awn_keybinder_is_modifier (guint keycode);

guint32 awn_keybinder_get_current_event_time (void);

G_END_DECLS

#endif /* __AWN_KEY_BINDER_H__ */

