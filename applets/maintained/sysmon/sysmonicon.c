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

/* awn-sysmonicon.c */

#include <cairo/cairo-xlib.h>

#include "sysmonicon.h"
#include "graph.h"
#include "util.h"

G_DEFINE_TYPE (AwnSysmonicon, awn_sysmonicon, AWN_TYPE_THEMED_ICON)

#include "sysmoniconprivate.h"
enum
{
  PROP_0,
  PROP_APPLET,
  PROP_GRAPH,
  PROP_GRAPH_TYPE,
  PROP_GRAPH_TYPE_DEFAULT,
  PROP_CLIENT,
  PROP_ID,
  PROP_INVALIDATE,
  PROP_RENDER_BG
};

static void awn_sysmonicon_create_surfaces (AwnSysmonicon * sysmonicon);
static void _size_changed(AwnApplet *app, guint size, AwnSysmonicon *object);


static void
awn_sysmonicon_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  AwnSysmoniconPrivate * priv;
  AwnSysmonicon * sysmonicon = AWN_SYSMONICON(object);
  priv = AWN_SYSMONICON_GET_PRIVATE (sysmonicon);
  switch (property_id) {
    case PROP_APPLET:
      g_value_set_object (value, priv->applet); 
      break;    
    case PROP_GRAPH:
      g_value_set_object (value, priv->graph); 
      break;
    case PROP_GRAPH_TYPE:
      g_value_set_int (value, priv->graph_type[CONF_STATE_INSTANCE]); 
      break;    
    case PROP_GRAPH_TYPE_DEFAULT:
      g_value_set_int (value, priv->graph_type[CONF_STATE_BASE]); 
      break;          
    case PROP_ID:
      g_value_set_string (value, priv->id); 
      break;
    case PROP_CLIENT:
      g_value_set_pointer (value,priv->client);
      break;
    case PROP_INVALIDATE:
      g_value_set_boolean (value,priv->invalidate);
      break;
    case PROP_RENDER_BG:
      g_value_set_boolean (value,priv->render_bg);
      break;
    default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
awn_sysmonicon_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  AwnSysmoniconPrivate * priv;
  AwnSysmonicon * sysmonicon = AWN_SYSMONICON(object);
  priv = AWN_SYSMONICON_GET_PRIVATE (sysmonicon);

  switch (property_id) {
    case PROP_APPLET:
      priv->applet = g_value_get_object (value);
      break;    
    case PROP_GRAPH:
      if (priv->graph)
      {
        g_object_unref (priv->graph);
      }
      priv->graph = g_value_get_object (value);
      break;          
    case PROP_GRAPH_TYPE:
      priv->graph_type[CONF_STATE_INSTANCE] = g_value_get_int (value);
      break;          
    case PROP_GRAPH_TYPE_DEFAULT:
      priv->graph_type[CONF_STATE_BASE] = g_value_get_int (value);
      break;                
    case PROP_ID:
      if (priv->id)
      {
        g_free (priv->id);
      }
      priv->id = g_value_dup_string (value);
      break;        
    case PROP_CLIENT:
      g_assert (!priv->client); /*this should not be set more than once!*/
      priv->client = g_value_get_pointer (value);
      break;
    case PROP_INVALIDATE:
      priv->invalidate = g_value_get_boolean (value);
      break;
    case PROP_RENDER_BG:
      priv->render_bg = g_value_get_boolean (value);
      awn_sysmonicon_create_surfaces (AWN_SYSMONICON(object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
awn_sysmonicon_dispose (GObject *object)
{
  AwnSysmoniconPrivate * priv = AWN_SYSMONICON_GET_PRIVATE (object); 
  
  if (priv->client)
  {
    g_object_unref (priv->client);
  }
  
  G_OBJECT_CLASS (awn_sysmonicon_parent_class)->dispose (object);
}

static void
awn_sysmonicon_finalize (GObject *object)
{
  G_OBJECT_CLASS (awn_sysmonicon_parent_class)->finalize (object);
}


void
awn_sysmonicon_constructed (GObject *object)
{
  DesktopAgnosticConfigClient * client_baseconf;
  AwnSysmoniconPrivate * priv;
  AwnApplet * applet;
  gchar * name;
  gint size;
  GError * err=NULL;
  
  priv = AWN_SYSMONICON_GET_PRIVATE (object);
  
  if (G_OBJECT_CLASS (awn_sysmonicon_parent_class)->constructed )
  {
    G_OBJECT_CLASS (awn_sysmonicon_parent_class)->constructed (object);
  }
  
  g_object_get (object,
                "applet", &applet,
                NULL);
  g_assert (applet);
  
  g_object_get (applet,
                "canonical-name",&name,
                "client-baseconf",&client_baseconf,
                NULL);
  priv->client = awn_config_get_default_for_applet_by_info (name, priv->id,NULL);

  size = awn_applet_get_size (AWN_APPLET (applet));
  awn_icon_set_custom_paint (AWN_ICON (object), size, size);
  
  g_assert (priv->client);
  
  do_bridge ( applet,object,
             "icon","graph_type","graph-type");

  desktop_agnostic_config_client_bind (client_baseconf,
                                       "applet", "render_bg",
                                       object, "render-bg", FALSE,
                                       DESKTOP_AGNOSTIC_CONFIG_BIND_METHOD_INSTANCE,
                                       &err);
  if (err)
  {
    g_warning ("%s: error binding %s",__func__,err->message);
    g_error_free (err);
    err = NULL;
  }
  
  g_signal_connect (G_OBJECT (priv->applet), "size-changed", 
                    G_CALLBACK (_size_changed), object);
  g_signal_connect_swapped (G_OBJECT (priv->applet), "realize",
                            G_CALLBACK (awn_sysmonicon_create_surfaces), 
                            object);
  g_free (name);
}

static void
awn_sysmonicon_class_init (AwnSysmoniconClass *klass)
{
  GParamSpec   *pspec;  
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = awn_sysmonicon_get_property;
  object_class->set_property = awn_sysmonicon_set_property;
  object_class->dispose = awn_sysmonicon_dispose;
  object_class->finalize = awn_sysmonicon_finalize;
  object_class->constructed = awn_sysmonicon_constructed;
  
  pspec = g_param_spec_object ("applet",
                               "Applet",
                               "AwnApplet",
                               AWN_TYPE_APPLET,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_APPLET, pspec);  

  pspec = g_param_spec_object ("graph",
                               "Graph",
                               "Graph",
                               AWN_TYPE_GRAPH,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_GRAPH, pspec);  
  
  pspec = g_param_spec_int ("graph-type",
                               "Graph_type",
                               "Graph Type",
                               GRAPH_DEFAULT,
                               GRAPH_LAST,
                               GRAPH_DEFAULT,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_GRAPH_TYPE, pspec);  

  pspec = g_param_spec_int ("graph-type-base",
                               "Graph_type default",
                               "Graph Type default",
                               GRAPH_DEFAULT,
                               GRAPH_LAST,
                               GRAPH_DEFAULT,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_GRAPH_TYPE_DEFAULT, pspec);  
  
  
  pspec = g_param_spec_string ("id",
                               "ID",
                               "ID",
                               "default",
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_ID, pspec);  
  
  pspec = g_param_spec_pointer ("client",
                               "client",
                               "config client",
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CLIENT, pspec);   
  
  pspec = g_param_spec_boolean ("invalidate",
                               "invalidate surface",
                               "invalidate surface",
                               TRUE,
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_INVALIDATE, pspec);   

  pspec = g_param_spec_boolean ("render-bg",
                              "Render Background",
                              "Render Background",
                               TRUE,
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_RENDER_BG, pspec);   

  g_type_class_add_private (object_class, sizeof (AwnSysmoniconPrivate));
  
}

static gboolean _expose(GtkWidget *self,
                        GdkEventExpose *event,
                        gpointer null)
{
  AwnSysmoniconPrivate * priv;
  cairo_t * ctx;
  AwnEffects * effects;
  
  priv = AWN_SYSMONICON_GET_PRIVATE (self);
  
  g_return_val_if_fail (priv->graph_cr, FALSE);
  g_return_val_if_fail (priv->bg_cr, FALSE);
  g_return_val_if_fail (priv->fg_cr, FALSE);

  effects = awn_overlayable_get_effects (AWN_OVERLAYABLE (self));
  g_return_val_if_fail (effects, FALSE);
//  ctx = awn_effects_cairo_create (effects);
  ctx = awn_effects_cairo_create_clipped (effects, event);
  g_return_val_if_fail (ctx, FALSE);


  if (priv->invalidate)
  {
    gint size;
    size = awn_applet_get_size (AWN_APPLET (priv->applet));

    awn_graph_render_to_context (priv->graph, priv->graph_cr, size, size);
    priv->invalidate = FALSE;
  }
  /*FIXME
   Have a background, rendered graph, and foregrond and slap them together.
   
   The graph surface is just layered on top of bg. fg will be handled differently.  
   Not rendering the graph on top of the surface to allow the graph render to be
   optimized by moving chunks of the graph surface around instead of rerendering 
   the whole thing... 
   
   fg probably needs to be rendered on top on every pass instead of creating a 
   (potentially) reusable surface.
   */
  
  /*FIXME call (for the moment just setting it create_surfaces) ->set_bg ()
   */

  cairo_set_operator (ctx,CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface (ctx, priv->bg_surface,0.0,0.0);
  cairo_paint (ctx);
  cairo_set_operator (ctx,CAIRO_OPERATOR_OVER);
  cairo_set_source_surface (ctx, priv->graph_surface,0.0,0.0);
  cairo_paint (ctx);

  /*should call something along the lines of render_fg() which will be in 
   vtable
   */
  awn_effects_cairo_destroy (effects);

  return TRUE;
}

static void
awn_sysmonicon_init (AwnSysmonicon *self)
{
  AwnSysmoniconPrivate * priv;
  priv = AWN_SYSMONICON_GET_PRIVATE (self);

  priv->graph = NULL;
  priv->graph_cr = NULL;
  priv->fg_cr = NULL;
  priv->bg_cr = NULL;
  priv->graph_surface = NULL;
  priv->fg_surface = NULL;
  priv->bg_surface = NULL;
  g_signal_connect (G_OBJECT (self), "expose-event", 
                    G_CALLBACK (_expose), NULL);
}

GtkWidget*
awn_sysmonicon_new (AwnApplet *applet)
{
  return g_object_new (AWN_TYPE_SYSMONICON, 
                       "Applet",applet,
                       NULL);
}

AwnGraph *
awn_sysmonicon_get_graph(AwnSysmonicon * self)
{
  AwnSysmoniconPrivate * priv;
  priv = AWN_SYSMONICON_GET_PRIVATE (self);
  return priv->graph;
}

static void
awn_sysmonicon_create_surfaces (AwnSysmonicon * sysmonicon)
{
  cairo_t * temp_cr =NULL;
  AwnSysmoniconPrivate * priv;
  gint size;
  
  priv = AWN_SYSMONICON_GET_PRIVATE (sysmonicon);

  temp_cr = gdk_cairo_create(GTK_WIDGET(priv->applet)->window);
  if (!temp_cr)
  {
    return;
  }
  size = awn_applet_get_size (AWN_APPLET (priv->applet));
  
  if (priv->graph_cr)
  {
    cairo_destroy(priv->graph_cr);
    priv->graph_cr = NULL;
  }

  if (priv->graph_surface)
  {
    cairo_surface_destroy(priv->graph_surface);
    priv->graph_surface = NULL;
  }

  if (priv->bg_cr)
  {
    cairo_destroy(priv->bg_cr);
    priv->bg_cr = NULL;
  }

  if (priv->bg_surface)
  {
    cairo_surface_destroy(priv->bg_surface);
    priv->bg_surface = NULL;
  }

  if (priv->fg_cr)
  {
    cairo_destroy(priv->fg_cr);
    priv->fg_cr = NULL;
  }

  if (priv->fg_surface)
  {
    cairo_surface_destroy(priv->fg_surface);
    priv->fg_surface = NULL;
  }  
  
  priv->graph_surface = cairo_surface_create_similar (cairo_get_target(temp_cr),CAIRO_CONTENT_COLOR_ALPHA, size,size);
  priv->graph_cr = cairo_create(priv->graph_surface);
  priv->bg_surface = cairo_surface_create_similar (cairo_get_target(temp_cr),CAIRO_CONTENT_COLOR_ALPHA, size,size);
  priv->bg_cr = cairo_create(priv->bg_surface);
  priv->fg_surface = cairo_surface_create_similar (cairo_get_target(temp_cr),CAIRO_CONTENT_COLOR_ALPHA, size,size);
  priv->fg_cr = cairo_create(priv->fg_surface);

  if (priv->render_bg)
  {
    awn_cairo_rounded_rect (priv->bg_cr,0.0, 0.0,size+1,size+1,1.0,ROUND_ALL );
    cairo_set_source_rgba (priv->bg_cr,0.0,0.0,0.0,0.3);
    cairo_fill (priv->bg_cr);
  }
  /*FIXME should be in vtable ->set_bg() or something similar
    in most cases would set the surface once.... then just let it be 
   reused
   
   fg_* will probably end up being removed (see comment in update_icon()
   */

/*  remove so glow effect looks ok.
  cairo_set_source_rgba (priv->bg_cr,0.2,0.2,0.2,0.05);
  cairo_set_operator (priv->bg_cr,CAIRO_OPERATOR_SOURCE);
  cairo_paint (priv->bg_cr);
  */
  cairo_destroy(temp_cr);

  priv->invalidate = TRUE;
}

void
awn_sysmonicon_update_icon (AwnSysmonicon * icon)
{
  AwnSysmoniconPrivate * priv;
  priv = AWN_SYSMONICON_GET_PRIVATE (icon);

  gtk_widget_queue_draw (GTK_WIDGET(icon));
}

static 
void _size_changed(AwnApplet *app, guint size, AwnSysmonicon *icon)
{
  AwnSysmoniconPrivate * priv;
  
  priv = AWN_SYSMONICON_GET_PRIVATE (icon);
  
  g_debug ("Resizing\n");
  awn_icon_set_custom_paint (AWN_ICON (icon), size, size);
  awn_sysmonicon_create_surfaces (icon);
}

