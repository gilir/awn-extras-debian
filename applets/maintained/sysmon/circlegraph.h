/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
 
/* awn-circlegraph.h */

#ifndef _AWN_CIRCLEGRAPH
#define _AWN_CIRCLEGRAPH

#include <glib-object.h>
#include "graph.h"

G_BEGIN_DECLS

#define AWN_TYPE_CIRCLEGRAPH awn_circlegraph_get_type()

#define AWN_CIRCLEGRAPH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), AWN_TYPE_CIRCLEGRAPH, AwnCirclegraph))

#define AWN_CIRCLEGRAPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), AWN_TYPE_CIRCLEGRAPH, AwnCirclegraphClass))

#define AWN_IS_CIRCLEGRAPH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AWN_TYPE_CIRCLEGRAPH))

#define AWN_IS_CIRCLEGRAPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), AWN_TYPE_CIRCLEGRAPH))

#define AWN_CIRCLEGRAPH_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), AWN_TYPE_CIRCLEGRAPH, AwnCirclegraphClass))

typedef struct {
  AwnGraph parent;
} AwnCirclegraph;

typedef struct {
  AwnGraphClass parent_class;
} AwnCirclegraphClass;

GType awn_circlegraph_get_type (void);

AwnCirclegraph* awn_circlegraph_new (gdouble,gdouble);

G_END_DECLS

#endif /* _AWN_CIRCLEGRAPH */
