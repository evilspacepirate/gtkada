/*
-----------------------------------------------------------------------
--          GtkAda - Ada95 binding for the Gimp Toolkit              --
--                                                                   --
--                     Copyright (C) 1998-2000                       --
--        Emmanuel Briot, Joel Brobecker and Arnaud Charlet          --
--                                                                   --
-- This library is free software; you can redistribute it and/or     --
-- modify it under the terms of the GNU General Public               --
-- License as published by the Free Software Foundation; either      --
-- version 2 of the License, or (at your option) any later version.  --
--                                                                   --
-- This library is distributed in the hope that it will be useful,   --
-- but WITHOUT ANY WARRANTY; without even the implied warranty of    --
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU --
-- General Public License for more details.                          --
--                                                                   --
-- You should have received a copy of the GNU General Public         --
-- License along with this library; if not, write to the             --
-- Free Software Foundation, Inc., 59 Temple Place - Suite 330,      --
-- Boston, MA 02111-1307, USA.                                       --
--                                                                   --
-- As a special exception, if other files instantiate generics from  --
-- this unit, or you link this unit with other files to produce an   --
-- executable, this  unit  does not  by itself cause  the resulting  --
-- executable to be covered by the GNU General Public License. This  --
-- exception does not however invalidate any other reasons why the   --
-- executable file  might be covered by the  GNU Public License.     --
-----------------------------------------------------------------------
*/

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktypeutils.h>

gint
convert_a (void* a)
{
   return (gint)a;
}

void*
convert_i (gint s)
{
   return (void*)s;
}

/********************************************************************
 *  Returns the major/minor/macro version number of Gtk+. This is
 *  needed as the windows version uses a different convention for the
 *  corresponding variables gtk_{major/minor/micro)_version than the
 *  unix version.
 ********************************************************************/

guint
ada_gtk_major_version () {
  return GTK_MAJOR_VERSION;
}

guint
ada_gtk_minor_version () {
  return GTK_MINOR_VERSION;
}

guint
ada_gtk_micro_version () {
  return GTK_MICRO_VERSION;
}

/********************************************************************
 **  Returns the real widget name (as opposed to gtk_widget_get_name,
 **  this one returns NULL instead of the class name if no name was
 **  set.
 ********************************************************************/

char*
ada_gtk_get_real_name (GtkWidget* w) {
  return w->name;
}

/********************************************************************
 **  This function parses the command line and returns
 **  true if macro_switch exists. It is also removed from
 **  the command line
 ********************************************************************/

int
ada_gtk_parse_cmd_line (char* macro_switch) {
  int i;
  extern int gnat_argc;
  extern char** gnat_argv;

  for (i=1; i<gnat_argc; i++)
    {
      if (! strcmp (gnat_argv[i], macro_switch)) {
	while (i < gnat_argc - 1) {
	  gnat_argv [i] = gnat_argv [i + 1];
	  i++;
	}
	gnat_argv [i] = NULL;
	gnat_argc--;
	return 1;
      }
    }
  return 0;
}
 
/********************************************************************
 **  This function checks the number of arguments for a given signal
 **  and a specific widget.
 **  Returns -1 if the signal does not exist
 ********************************************************************/

guint
ada_signal_count_arguments (gint type, char* signal_name)
{
  /* Implementation note: using gtk_signal_query adds an extra call to
     malloc and free, but using the internal variables/macros for
     gtksignal.c would make this too dependent from the exact implementation
     of gtk+ */
  guint signal_id = gtk_signal_lookup (signal_name, type);
  GtkSignalQuery * signal;
  guint nparams;
  if (signal_id == 0)
    return -1;
  signal = gtk_signal_query (signal_id);  
  nparams = signal->nparams;
  g_free (signal);
  return nparams;
}

/********************************************************************
 **  Returns the type of the num-th argument to the handlers for
 **  signal_name.
 **  If num is negative, returns the type returned by the handler
 ********************************************************************/

GtkType
ada_signal_argument_type (gint type, char* signal_name, gint num)
{
  /* Implementation note: using gtk_signal_query adds an extra call to
     malloc and free, but using the internal variables/macros for
     gtksignal.c would make this too dependent on the exact implementation
     of gtk+ */
  guint signal_id = gtk_signal_lookup (signal_name, type);
  GtkSignalQuery * signal = gtk_signal_query (signal_id);

  if (num < 0)
    return signal->return_val;
  else if (num >= signal->nparams)
    return GTK_TYPE_NONE;
  else
    {
      GtkType params = signal->params [num];
      g_free (signal);
      return params;
    }
}

void
ada_set_return_value (GtkType type, void* value, GtkArg* params, guint num) {
  
  params[num].type = type;
  switch (GTK_FUNDAMENTAL_TYPE (type))
    {
    case GTK_TYPE_BOOL:
      *(GTK_RETLOC_BOOL (params[num])) = (gboolean)value;
      break;
    case GTK_TYPE_INT:
      *(GTK_RETLOC_INT (params[num])) = (gint)value;
      break;
    case GTK_TYPE_UINT:
      *(GTK_RETLOC_UINT (params[num])) = (guint)value;
      break;
    case GTK_TYPE_LONG:
      *(GTK_RETLOC_LONG (params[num])) = (glong)value;
      break;
    case GTK_TYPE_ULONG:
      *(GTK_RETLOC_ULONG (params[num])) = (gulong)value;
      break;
    case GTK_TYPE_STRING:
      *(GTK_RETLOC_STRING (params[num])) = (char*)value;
      break;
    case GTK_TYPE_POINTER:
      *(GTK_RETLOC_POINTER (params[num])) = (gpointer*)value;
      break;
    case GTK_TYPE_BOXED:
      *(GTK_RETLOC_BOXED (params[num])) = (gpointer*)value;
      break;
    default:
      fprintf (stderr, "GtkAda: Return value type not supported (%d)\n",
	       type);
      break;
    }
}

/*********************************************************************
 ** Creating new widgets
 *********************************************************************/

void*
ada_initialize_class_record (GtkObject*  object,
			     gint        nsignals,
			     char*       signals[],
			     void*       old_class_record)
{
  if (old_class_record)
    {
      object->klass = old_class_record;
      return old_class_record;
    }
  else
    {
      int i;
      guint *signals_id = malloc (nsignals * sizeof (guint));

      /* Right now, object->klass points to the ancestor's class */
      GtkObjectClass* ancestor = (GtkObjectClass*)(object->klass);

      /* We need to know the ancestor's class size */
      GtkTypeQuery* query = gtk_type_query (ancestor->type);

      /* Note: The memory allocated here is never freed. No need to worry,
	 since this is only allocated once per user's widget type, and
	 might be used until the end of the application */
      object->klass = (GtkObjectClass*) malloc (query->class_size
						+ nsignals * sizeof (void*));
      memcpy (object->klass, ancestor, query->class_size);

      /* Implementation Note: We are actually cheating here: we should
	 really create a new type, instead of reusing the parent class's type.
	 This would be cleaner for some things, but since it seems to work
	 fine that way, no need to make things more complicated.
	 The important thing to test is whether the new signals are also
	 known in the ancestor's class or not (they should not, of course!) */

      for (i = 0; i < nsignals; i++)
	{
	  signals_id [i] = gtk_signal_new
	    (signals[i],
	     GTK_RUN_FIRST,
	     object->klass->type,
	     query->class_size + i * sizeof (void*) /*offset*/,
	     gtk_marshal_NONE__NONE,
	     GTK_TYPE_NONE, 0);
	}
      gtk_object_class_add_signals (object->klass, signals_id, nsignals);

      /* Initialize the function pointers for the new signals to NULL */
      memset ((char*)(object->klass) + query->class_size, 0, 
	      nsignals * sizeof (void*));

      g_free (query);
      free (signals_id);
      return object->klass;
    }
}

/*********************************************************************
 **  Gdk.RGB functions
 *********************************************************************/

guint32
ada_rgb_cmap_get (GdkRgbCmap* cmap, gint index)
{
  return cmap->colors [index];
}

guchar
ada_rgb_cmap_get8 (GdkRgbCmap* cmap, gint index)
{
  return cmap->lut [index];
}

void
ada_rgb_cmap_set (GdkRgbCmap* cmap, gint index, guint32 value)
{
  cmap->colors [index] = value;
}

void
ada_rgb_cmap_set8 (GdkRgbCmap* cmap, gint index, guchar value)
{
  cmap->lut [index] = value;
}


/*
 * Gnode macros
 *
 */

gboolean
ada_gnode_is_root (GNode * node)
{
  return G_NODE_IS_ROOT (node);
}

gboolean
ada_gnode_is_leaf (GNode * node)
{
  return G_NODE_IS_LEAF (node);
}

GNode*
ada_gnode_prev_sibling (GNode * node)
{
  return g_node_prev_sibling (node);
}

GNode*
ada_gnode_next_sibling (GNode * node)
{
  return g_node_next_sibling (node);
}

GNode*
ada_gnode_first_child (GNode * node)
{
  return g_node_first_child (node);
}

/*
 *
 * object macros
 *
 */

guint32 ada_object_flags (GtkObject * object)
{
  return GTK_OBJECT_FLAGS (object);
}

void
ada_object_set_flags (GtkObject * object, guint32 flags)
{
  GTK_OBJECT_SET_FLAGS (object, flags);
}

gint
ada_object_flag_is_set (GtkObject * object, guint32 flag)
{
  return ((GTK_OBJECT_FLAGS (object) & flag) != 0);
}

void 
ada_object_unset_flags (GtkObject * object, guint32 flags)
{
  GTK_OBJECT_UNSET_FLAGS (object, flags);
}

/*
 *
 * Widget macros
 *
 */

guint32 
ada_widget_is_sensitive (GtkWidget * widget)
{
  return GTK_WIDGET_IS_SENSITIVE (widget);
}

guint32
ada_widget_drawable (GtkWidget * widget)
{
  return GTK_WIDGET_DRAWABLE (widget);
}

guint16
ada_widget_allocation_height (GtkWidget* widget)
{
  return widget->allocation.height;
}

guint16
ada_widget_allocation_width (GtkWidget* widget)
{
  return widget->allocation.width;
}

gint16
ada_widget_allocation_x (GtkWidget* widget)
{
  return widget->allocation.x;
}

gint16
ada_widget_allocation_y (GtkWidget* widget)
{
  return widget->allocation.y;
}

/*
 *
 * radio_menu_item
 *
 */

GtkWidget*
ada_radio_menu_item_new_from_widget (GtkRadioMenuItem *group)
{
  GSList *l = NULL;
  if (group)
    l = gtk_radio_menu_item_group (group);
  return gtk_radio_menu_item_new (l);
}

GtkWidget *
ada_radio_menu_item_new_with_label_from_widget (GtkRadioMenuItem *group,
						const gchar      *label)
{
  GSList *l = NULL;
  if (group)
    l = gtk_radio_menu_item_group (group);
  return gtk_radio_menu_item_new_with_label (l, label);
}

/********************
 * Font_Selection
 ********************/

GtkWidget*
ada_gtk_font_selection_dialog_get_cancel (GtkFontSelectionDialog* fsd)
{
  return fsd->cancel_button;
}

GtkWidget*
ada_gtk_font_selection_dialog_get_ok (GtkFontSelectionDialog* fsd)
{
  return fsd->ok_button;
}

GtkWidget*
ada_gtk_font_selection_dialog_get_apply (GtkFontSelectionDialog* fsd)
{
  return fsd->apply_button;
}

/********************
 * Check_Menu_Item
 ********************/

guint
ada_check_menu_item_get_active (GtkCheckMenuItem* item) {
  return item->active;
}

/********************
 * Menu_Item
 ********************/

GtkWidget*
ada_gtk_menu_item_get_submenu (GtkMenuItem* item) {
  return item->submenu;
}

/********************
 * Paned
 ********************/

gint
ada_paned_get_child1_resize (GtkPaned* widget) {
  return widget->child1_resize;
}

gint
ada_paned_get_child2_resize (GtkPaned* widget) {
  return widget->child2_resize;
}

gint
ada_paned_get_child1_shrink (GtkPaned* widget) {
  return widget->child1_shrink;
}

gint
ada_paned_get_child2_shrink (GtkPaned* widget) {
  return widget->child2_shrink;
}

GtkWidget*
ada_paned_get_child1 (GtkPaned* widget) {
  return widget->child1;
}

GtkWidget*
ada_paned_get_child2 (GtkPaned* widget) {
  return widget->child2;
}

/********************
 * Progress_Bar
 ********************/

GtkAdjustment*
ada_progress_get_adjustment (GtkProgress* widget) {
  return widget->adjustment;
}

guint
ada_progress_get_activity_mode (GtkProgress* widget) {
  return widget->activity_mode;
}

/********************
 * GdkPoint
 ********************/

guint
ada_gdk_point_size () {
  return sizeof (GdkPoint);
}

/*
 *
 *  GdkCursor
 *
 */

GdkCursor* 
ada_gdk_cursor_new (gint cursor_type)
{
  return gdk_cursor_new (cursor_type);
}


/**********************************************************
 **  Graphic contexts
 **********************************************************/

GdkGCValues*
ada_gdk_gc_new_values ()
{
  return (GdkGCValues*) g_new (GdkGCValues, 1);;
}

void
ada_gdk_gc_free_values (GdkGCValues* values)
{
  g_free (values);
}

void
ada_gdk_gc_set_background (GdkGCValues* values,
			   GdkColor     color)
{
  values->background = color;
}

void
ada_gdk_gc_set_foreground (GdkGCValues* values,
			   GdkColor     color)
{
  values->foreground = color;
}

void
ada_gdk_gc_set_clip_origin (GdkGCValues* values,
			    gint x,
			    gint y)
{
  values->clip_x_origin = x;
  values->clip_y_origin = y;
}

void
ada_gdk_gc_set_exposures (GdkGCValues* values,
			  gint exposures)
{
  values->graphics_exposures = exposures;
}

void
ada_gdk_gc_set_fill (GdkGCValues* values,
		     GdkFill      fill)
{
  values->fill = fill;
}

void
ada_gdk_gc_set_font (GdkGCValues* values,
		     GdkFont*     font)
{
  values->font = font;
}

void
ada_gdk_gc_set_function (GdkGCValues* values,
			 GdkFunction  function)
{
  values->function = function;
}

void
ada_gdk_gc_set_line_attributes (GdkGCValues* values,
				gint         line_width,
				GdkLineStyle line_style,
				GdkCapStyle  cap_style,
				GdkJoinStyle join_style)
{
  values->line_width = line_width;
  values->line_style = line_style;
  values->cap_style  = cap_style;
  values->join_style = join_style;
}

void
ada_gdk_gc_set_subwindow (GdkGCValues*      values,
			  GdkSubwindowMode  subwindow_mode)
{
  values->subwindow_mode = subwindow_mode;
}

void
ada_gdk_gc_set_ts_origin (GdkGCValues* values,
			  gint         x,
			  gint         y)
{
  values->ts_x_origin = x;
  values->ts_y_origin = y;
}

/**********************************************************
 **  Support for events
 **********************************************************/

extern const gdouble invalid_gdouble_value;
extern const gint    invalid_gint_value;
extern const gint16  invalid_gint16_value;
extern const guint   invalid_guint_value;
extern const guint16 invalid_guint16_value;
extern const guint32 invalid_guint32_value;
extern const gulong  invalid_gulong_value;

gdouble ada_gdk_event_get_x (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.x;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.x;
    case GDK_CONFIGURE:
      return event->configure.x;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gdouble ada_gdk_event_get_y (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.y;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.y;
    case GDK_CONFIGURE:
      return event->configure.y;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gint16 ada_gdk_event_get_width (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_CONFIGURE:
      return event->configure.width;
    default:
      break;
    }
  return invalid_gint16_value;
}

gint16 ada_gdk_event_get_height (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_CONFIGURE:
      return event->configure.height;
    default:
      break;
    }
  return invalid_gint16_value;
}

gdouble ada_gdk_event_get_x_root (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x_root;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.x_root;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.x_root;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gdouble ada_gdk_event_get_y_root (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y_root;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.y_root;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.y_root;
    default:
      break;
    }
  return invalid_gdouble_value;
}

guint ada_gdk_event_get_button (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.button;
    default:
      break;
    }
  return invalid_guint_value;
}

guint ada_gdk_event_get_state (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.state;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.state;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.state;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.state;
    case GDK_PROPERTY_NOTIFY:
      return event->property.state;
    default:
      break;
    }
  return invalid_guint_value;
}

GdkWindow* ada_gdk_event_get_subwindow (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.subwindow;
    default:
      break;
    }
  return NULL;
}

gint ada_gdk_event_get_mode (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.mode;
    default:
      break;
    }
  return invalid_gint_value;
}

gint ada_gdk_event_get_detail (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.detail;
    default:
      break;
    }
  return invalid_gint_value;
}

gint ada_gdk_event_get_focus (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.focus;
    default:
      break;
    }
  return invalid_gint_value;
}

gdouble ada_gdk_event_get_pressure (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.pressure;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.pressure;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gdouble ada_gdk_event_get_xtilt (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.xtilt;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.xtilt;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gdouble ada_gdk_event_get_ytilt (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.ytilt;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.ytilt;
    default:
      break;
    }
  return invalid_gdouble_value;
}

gint ada_gdk_event_get_source (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.source;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.source;
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      return event->proximity.source;
    default:
      break;
    }
  return invalid_gint_value;
}

guint32 ada_gdk_event_get_device_id (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.deviceid;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.deviceid;
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      return event->proximity.deviceid;
    default:
      break;
    }
  return invalid_guint32_value;
}

void
ada_gdk_event_get_area (GdkEvent * event, GdkRectangle* area)
{
  if (event->type == GDK_EXPOSE)
    *area = event->expose.area;
  else
    {
      fprintf (stderr, "event type=%d\n", event->type);
      area->width = invalid_guint16_value;
    }
}

gint ada_gdk_event_get_count (GdkEvent * event)
{
  if (event->type == GDK_EXPOSE)
    return event->expose.count;
  return invalid_gint_value;
}

gint ada_gdk_event_get_in (GdkEvent * event)
{
  if (event->type == GDK_FOCUS_CHANGE) 
    return event->focus_change.in;
  return invalid_gint_value;
}

gint ada_gdk_event_get_is_hint (GdkEvent * event)
{
  if (event->type == GDK_MOTION_NOTIFY)
    return event->motion.is_hint;
  return invalid_gint_value;
}

gint ada_gdk_event_get_key_val (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.keyval;
    default:
      break;
    }
  return invalid_gint_value;
}

char* ada_gdk_event_get_string (GdkEvent * event)
{
  switch (event->type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.string;
    default:
      break;
    }
  return NULL;
}

gulong ada_gdk_event_get_atom (GdkEvent * event)
{
  if (event->type == GDK_PROPERTY_NOTIFY)
    return event->property.atom;
  return invalid_gulong_value;
}

guint ada_gdk_event_get_property_state (GdkEvent * event)
{
  if (event->type == GDK_PROPERTY_NOTIFY)
    return event->property.state;
  return invalid_guint_value;
}

gint ada_gdk_event_get_visibility_state (GdkEvent * event)
{
  if (event->type == GDK_VISIBILITY_NOTIFY)
    return event->visibility.state;
  return invalid_gint_value;
}

gulong ada_gdk_event_get_selection (GdkEvent * event)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    return event->selection.selection;
  return invalid_gulong_value;
}

gulong ada_gdk_event_get_target (GdkEvent * event)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    return event->selection.target;
  return invalid_gulong_value;
}

gulong ada_gdk_event_get_property (GdkEvent * event)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    return event->selection.property;
  return invalid_gulong_value;
}

guint32 ada_gdk_event_get_requestor (GdkEvent * event)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    return event->selection.requestor;
  return invalid_guint32_value;
}

gulong ada_gdk_event_get_message_type (GdkEvent * event)
{
  if (event->type == GDK_CLIENT_EVENT)
    return event->client.message_type;
  return invalid_gulong_value;
}

GdkEvent * ada_gdk_event_create (gint type, GdkWindow* win)
{
  GdkEvent static_event;
  GdkEvent* event;
  
  static_event.any.type   = type;
  static_event.any.window = win;
  gdk_window_ref (win);
  switch (type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      static_event.key.string = NULL;
      break;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      static_event.crossing.subwindow = NULL;
      break;
    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
      /* ??? will create an error in gdk_event_copy */
      static_event.dnd.context = NULL;
      break;
    }
  event = gdk_event_copy(&static_event);
  return event;
}

short * ada_gdk_event_client_get_s (GdkEventClient * event)
{
  return event->data.s;
}

char * ada_gdk_event_client_get_b (GdkEventClient * event)
{
  return event->data.b;
}

long * ada_gdk_event_client_get_l (GdkEventClient * event)
{
  return event->data.l;
}

gushort ada_gdk_event_client_get_data_format (GdkEventClient * event)
{
  return event->data_format;
}

GdkEventType ada_gdk_event_get_type (GdkEventAny * event)
{
  return event->type;
}

gint8 ada_gdk_event_get_send_event (GdkEventAny * event)
{
  return event->send_event;
}

GdkWindow * ada_gdk_event_get_window (GdkEventAny * event)
{
  return event->window;
}

void ada_gdk_event_set_window (GdkEventAny * event, GdkWindow * window)
{
  event->window = window;
}

/*******************************************************************
 **  Setting the fields of events
 *******************************************************************/

gint ada_gdk_event_set_x (GdkEvent * event, gdouble x)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      event->motion.x = x;
      break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      event->button.x = x;
      break;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.x = x;
      break;
    case GDK_CONFIGURE:
      event->configure.x = x;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_y (GdkEvent * event, gdouble y)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      break;
      event->motion.y = y;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      event->button.y = y;
      break;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.y = y;
      break;
    case GDK_CONFIGURE:
      event->configure.y = y;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_width (GdkEvent * event, gint16 width)
{
  switch (event->type)
    {
    case GDK_CONFIGURE:
      event->configure.width = width;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_height (GdkEvent * event, gint16 height)
{
  switch (event->type)
    {
    case GDK_CONFIGURE:
      event->configure.height = height;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_button (GdkEvent * event, guint button)
{
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      event->button.button = button;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_state (GdkEvent * event, guint state)
{
  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      event->motion.state = state;
      break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      event->button.state = state;
      break;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      event->key.state = state;
      break;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.state = state;
      break;
    case GDK_PROPERTY_NOTIFY:
      event->property.state = state;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_subwindow (GdkEvent * event, GdkWindow* win)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.subwindow = win;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_mode (GdkEvent * event, gint mode)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.mode = mode;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_detail (GdkEvent * event, gint detail)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.detail = detail;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_focus (GdkEvent * event, gint focus)
{
  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      event->crossing.focus = focus;
      break;
    default:
      return 0;
    }
  return 1;
}

gint ada_gdk_event_set_area (GdkEvent * event, GdkRectangle area)
{
  if (event->type == GDK_EXPOSE)
    event->expose.area = area;
  else
    return 0;
  return 1;
}

gint ada_gdk_event_set_in (GdkEvent * event, gint in)
{
  if (event->type == GDK_FOCUS_CHANGE)
    {
      event->focus_change.in = in;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_is_hint (GdkEvent * event, gint is_hint)
{
  if (event->type == GDK_MOTION_NOTIFY)
    {
      event->motion.is_hint = is_hint;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_key_val (GdkEvent * event, gint keyval)
{
  switch (event->type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      event->key.keyval = keyval;
      break;
    default:
      return 0;
    }
  return 1;
}
gint ada_gdk_event_set_atom (GdkEvent * event, gulong atom)
{
  if (event->type == GDK_PROPERTY_NOTIFY)
    {
      event->property.atom = atom;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_property_state (GdkEvent * event, guint state)
{
  if (event->type == GDK_PROPERTY_NOTIFY)
    {
      event->property.state = state;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_visibility_state (GdkEvent * event, gint state)
{
  if (event->type == GDK_VISIBILITY_NOTIFY)
    {
      event->visibility.state = state;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_selection (GdkEvent * event, gulong selection)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    {
      event->selection.selection = selection;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_target (GdkEvent * event, gulong target)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    {
      event->selection.target = target;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_property (GdkEvent * event, gulong property)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    {
      event->selection.property = property;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_requestor (GdkEvent * event, guint32 requestor)
{
  if (event->type == GDK_SELECTION_NOTIFY)
    {
      event->selection.requestor = requestor;
      return 1;
    }
  return 0;
}

gint ada_gdk_event_set_message_type (GdkEvent * event, gulong type)
{
  if (event->type == GDK_CLIENT_EVENT)
    {
      event->client.message_type = type;
      return 1;
    }
  return 0;
}

/********************
 * GtkAdjustment
 ********************/

gfloat
ada_gtk_adjustment_get_value (GtkAdjustment * adjustment)
{
  return adjustment->value;
}


gfloat
ada_gtk_adjustment_get_page_size (GtkAdjustment * adjustment)
{
  return adjustment->page_size;
}

gfloat
ada_gtk_adjustment_get_page_increment (GtkAdjustment * adjustment)
{
  return adjustment->page_increment;
}

gfloat
ada_adjustment_get_lower (GtkAdjustment * adjustment)
{
  return adjustment->lower;
}

gfloat
ada_adjustment_get_upper (GtkAdjustment * adjustment)
{
  return adjustment->upper;
}

void
ada_adjustment_set_page_increment (GtkAdjustment * adjustment,
                                       gfloat value)
{
  adjustment->page_increment = value;
}

void
ada_adjustment_set_step_increment (GtkAdjustment * adjustment,
				   gfloat value)
{
  adjustment->step_increment = value;
}

void
ada_gtk_adjustment_set_lower (GtkAdjustment * adjustment, gfloat lower)
{
  adjustment->lower = lower;
}

void
ada_gtk_adjustment_set_upper (GtkAdjustment * adjustment, gfloat upper)
{
  adjustment->upper = upper;
}

gfloat
ada_gtk_adjustment_get_step_increment (GtkAdjustment * adjustment)
{
  return adjustment->step_increment;
}

void
ada_adjustment_set_page_size (GtkAdjustment * adjustment,
                                  gfloat value)
{
  adjustment->page_size = value;
}

/*
 *
 * GtkStyle
 *
 */

void
ada_style_set_fg (GtkStyle* style, gint state, GdkColor* color)
{
  if (color != NULL)
    style->fg[state] = *color;
}

GdkColor*
ada_style_get_fg (GtkStyle* style, gint state)
{
  return style->fg + state;
}

void
ada_style_set_bg (GtkStyle* style, gint state, GdkColor* color)
{
  if (color != NULL)
    style->bg[state] = *color;
}

GdkColor*
ada_style_get_bg (GtkStyle* style, gint state)
{
  return style->bg + state;
}

void
ada_style_set_light (GtkStyle* style, gint state, GdkColor* color)
{
  style->light[state] = *color;
}

GdkColor*
ada_style_get_light (GtkStyle* style, gint state)
{
  return style->light + state;
}

void
ada_style_set_dark (GtkStyle* style, gint state, GdkColor* color)
{
  style->dark[state] = *color;
}

GdkColor*
ada_style_get_dark (GtkStyle* style, gint state)
{
  return style->dark + state;
}

void
ada_style_set_mid (GtkStyle* style, gint state, GdkColor* color)
{
  style->mid[state] = *color;
}

GdkColor*
ada_style_get_mid (GtkStyle* style, gint state)
{
  return style->mid + state;
}

void
ada_style_set_text (GtkStyle* style, gint state, GdkColor* color)
{
  style->text[state] = *color;
}

GdkColor*
ada_style_get_text (GtkStyle* style, gint state)
{
  return style->text + state;
}

void
ada_style_set_base (GtkStyle* style, gint state, GdkColor* color)
{
  style->base[state] = *color;
}

GdkColor*
ada_style_get_base (GtkStyle* style, gint state)
{
  return style->base + state;
}

void
ada_style_set_black (GtkStyle* style, GdkColor* color)
{
  style->black = *color;
}

GdkColor*
ada_style_get_black (GtkStyle* style)
{
  return &style->black;
}

void
ada_style_set_white (GtkStyle* style, GdkColor* color)
{
  style->white = *color;
}

GdkColor*
ada_style_get_white (GtkStyle* style)
{
  return &style->white;
}

void
ada_style_set_font (GtkStyle* style, GdkFont* font)
{
  style->font = font;
}

GdkFont*
ada_style_get_font (GtkStyle* style)
{
  return style->font;
}

void
ada_style_set_fg_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->fg_gc[state] = gc;
}

GdkGC*
ada_style_get_fg_gc (GtkStyle * style, gint state)
{
  return style->fg_gc [state];
}

void
ada_style_set_bg_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->bg_gc[state] = gc;
}

GdkGC*
ada_style_get_bg_gc (GtkStyle * style, gint state)
{
  return style->bg_gc [state];
}

void
ada_style_set_light_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->light_gc[state] = gc;
}

GdkGC*
ada_style_get_light_gc (GtkStyle * style, gint state)
{
  return style->light_gc [state];
}

void
ada_style_set_dark_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->dark_gc[state] = gc;
}

GdkGC*
ada_style_get_dark_gc (GtkStyle * style, gint state)
{
  return style->dark_gc[state];
}

void
ada_style_set_mid_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->mid_gc[state] = gc;
}

GdkGC*
ada_style_get_mid_gc (GtkStyle * style, gint state)
{
  return style->mid_gc [state];
}

void
ada_style_set_text_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->text_gc[state] = gc;
}

GdkGC*
ada_style_get_text_gc (GtkStyle * style, gint state)
{
  return style->text_gc [state];
}

void
ada_style_set_base_gc (GtkStyle* style, gint state, GdkGC* gc)
{
  style->base_gc[state] = gc;
}

GdkGC*
ada_style_get_base_gc (GtkStyle * style, gint state)
{
  return style->base_gc [state];
}

void
ada_style_set_black_gc (GtkStyle* style, GdkGC* gc)
{
  style->black_gc = gc;
}

GdkGC*
ada_style_get_black_gc (GtkStyle * style)
{
  return style->black_gc;
}


void
ada_style_set_white_gc (GtkStyle* style, GdkGC* gc)
{
  style->white_gc = gc;
}

GdkGC*
ada_style_get_white_gc (GtkStyle * style)
{
  return style->white_gc;
}



void
ada_style_set_bg_pixmap (GtkStyle* style, gint state, GdkPixmap *pix)
{
	style->bg_pixmap[state] = pix;
}

GdkPixmap*
ada_style_get_bg_pixmap (GtkStyle* style, gint state)
{
	return style->bg_pixmap[state];
}

/***************************************************
 *  Functions for Objects
 ***************************************************/

gint
ada_object_get_type (GtkObject* object)
{
  return GTK_OBJECT_TYPE (object);
}

/***************************************************
 *  Functions for GtkArg
 ***************************************************/

gpointer
ada_gtkarg_value_object (GtkArg* args, guint num)
{
  gpointer return_value = NULL;
  switch (GTK_FUNDAMENTAL_TYPE (args [num].type))
    {
    case GTK_TYPE_OBJECT:
      return_value = (gpointer)GTK_VALUE_OBJECT (args [num]);
      break;
    case GTK_TYPE_POINTER:
      return_value = (gpointer)GTK_VALUE_POINTER (args [num]);
      break;
    case GTK_TYPE_STRING:
      return_value = (gpointer)GTK_VALUE_STRING (args [num]);
      break;
    case GTK_TYPE_BOXED:
      return_value = (gpointer)GTK_VALUE_BOXED (args [num]);
      break;
    case GTK_TYPE_INT:
      return_value = (gpointer)GTK_VALUE_INT (args [num]);
      break;
    case GTK_TYPE_BOOL:
      return_value = (gpointer)GTK_VALUE_BOOL (args [num]);
      break;
    case GTK_TYPE_UINT:
      return_value = (gpointer)GTK_VALUE_UINT (args [num]);
      break;
    case GTK_TYPE_LONG:
      return_value = (gpointer)GTK_VALUE_LONG (args [num]);
      break;
    case GTK_TYPE_ULONG:
      return_value = (gpointer)GTK_VALUE_ULONG (args [num]);
      break;
    case GTK_TYPE_INVALID:
      fprintf (stderr, "GtkAda: GTK_TYPE_INVALID found in ada_gtkarg_value_object\n");
      break;
    case GTK_TYPE_NONE:
      fprintf (stderr, "GtkAda: GTK_TYPE_NONE found in ada_gtkarg_value_object\n");
      break;
    default:
      {
	fprintf (stderr, "GtkAda: request for an Object value (%d) when we have a %d\n",
		 GTK_TYPE_OBJECT, (args[num].type % 256));
      }
    }
  return return_value;
}

/***************************************************
 *  Functions to get the field of a color selection
 *  dialog
 ***************************************************/

GtkWidget*
ada_colorsel_dialog_get_colorsel (GtkColorSelectionDialog* dialog)
{
  return dialog->colorsel;
}

GtkWidget*
ada_colorsel_dialog_get_ok_button (GtkColorSelectionDialog* dialog)
{
  return dialog->ok_button;
}

GtkWidget*
ada_colorsel_dialog_get_reset_button (GtkColorSelectionDialog* dialog)
{
  return dialog->reset_button;
}

GtkWidget*
ada_colorsel_dialog_get_cancel_button (GtkColorSelectionDialog* dialog)
{
  return dialog->cancel_button;
}

GtkWidget*
ada_colorsel_dialog_get_help_button (GtkColorSelectionDialog* dialog)
{
  return dialog->help_button;
}

/********************************************************
 *  Functions to get the fields of a gamma curve
 ********************************************************/

GtkWidget*
ada_gamma_curve_get_curve (GtkGammaCurve* widget)
{
  return widget->curve;
}

gfloat
ada_gamma_curve_get_gamma (GtkGammaCurve* widget)
{
  return widget->gamma;
}

/******************************************
 ** Functions for Alignment
 ******************************************/

gfloat
ada_alignment_get_xalign (GtkAlignment* widget)
{
   return widget->xalign;
}

gfloat
ada_alignment_get_xscale (GtkAlignment* widget)
{
   return widget->xscale;
}

gfloat
ada_alignment_get_yalign (GtkAlignment* widget)
{
   return widget->yalign;
}

gfloat
ada_alignment_get_yscale (GtkAlignment* widget)
{
   return widget->yscale;
}


/******************************************
 ** Functions for Ruler
 ******************************************/

gfloat
ada_ruler_get_lower (GtkRuler* widget)
{
   return widget->lower;
}

gfloat
ada_ruler_get_max_size (GtkRuler* widget)
{
   return widget->max_size;
}

gfloat
ada_ruler_get_position (GtkRuler* widget)
{
   return widget->position;
}

gfloat
ada_ruler_get_upper (GtkRuler* widget)
{
   return widget->upper;
}

/******************************************
 ** Functions for Editable
 ******************************************/

guint
ada_editable_get_editable (GtkEditable* widget)
{
  return widget->editable;
}

void
ada_editable_set_editable (GtkEditable* widget, guint val)
{
  widget->editable = val;
}

gchar*
ada_editable_get_clipboard_text (GtkEditable* widget)
{
   return widget->clipboard_text;
}

guint
ada_editable_get_has_selection (GtkEditable* widget)
{
   return widget->has_selection;
}

guint
ada_editable_get_selection_end_pos (GtkEditable* widget)
{
   return widget->selection_end_pos;
}

guint
ada_editable_get_selection_start_pos (GtkEditable* widget)
{
   return widget->selection_start_pos;
}

/******************************************
 ** Functions for Aspect_Frame
 ******************************************/

gfloat
ada_aspect_frame_get_ratio (GtkAspectFrame* widget)
{
   return widget->ratio;
}

gfloat
ada_aspect_frame_get_xalign (GtkAspectFrame* widget)
{
   return widget->xalign;
}

gfloat
ada_aspect_frame_get_yalign (GtkAspectFrame* widget)
{
   return widget->yalign;
}

/******************************************
 ** Functions for Gtk_Layout
 ******************************************/

guint
ada_gtk_layout_get_xoffset (GtkLayout* layout) {
  return layout->xoffset;
}

guint
ada_gtk_layout_get_yoffset (GtkLayout* layout) {
  return layout->yoffset;
}

guint
ada_gtk_layout_get_width (GtkLayout* layout) {
  return layout->width;
}

guint
ada_gtk_layout_get_height (GtkLayout* layout) {
  return layout->height;
}

GdkWindow*
ada_gtk_layout_get_bin_window (GtkLayout* layout) {
  return layout->bin_window;
}

/******************************************
 ** Functions for Text
 ******************************************/

guint
ada_text_get_gap_position (GtkText* widget)
{
   return widget->gap_position;
}

guint
ada_text_get_gap_size (GtkText* widget)
{
   return widget->gap_size;
}

guchar*
ada_text_get_text (GtkText* widget)
{
   return widget->text.ch;
}

guint
ada_text_get_text_end (GtkText* widget)
{
   return widget->text_end;
}

GtkAdjustment*
ada_text_get_hadj (GtkText* widget)
{
  return widget->hadj;
}

GtkAdjustment*
ada_text_get_vadj (GtkText* widget)
{
  return widget->vadj;
}

/******************************************
 ** Functions for File_Selection
 ******************************************/

GtkWidget*
ada_file_selection_get_action_area (GtkFileSelection* widget)
{
   return widget->action_area;
}
GtkWidget*
ada_file_selection_get_button_area (GtkFileSelection* widget)
{
   return widget->button_area;
}
GtkWidget*
ada_file_selection_get_cancel_button (GtkFileSelection* widget)
{
   return widget->cancel_button;
}

GtkWidget*
ada_file_selection_get_dir_list (GtkFileSelection* widget)
{
   return widget->dir_list;
}

GtkWidget*
ada_file_selection_get_file_list (GtkFileSelection* widget)
{
   return widget->file_list;
}

GtkWidget*
ada_file_selection_get_help_button (GtkFileSelection* widget)
{
   return widget->help_button;
}

GtkWidget*
ada_file_selection_get_history_pulldown (GtkFileSelection* widget)
{
   return widget->history_pulldown;
}

GtkWidget*
ada_file_selection_get_ok_button (GtkFileSelection* widget)
{
   return widget->ok_button;
}

GtkWidget*
ada_file_selection_get_selection_entry (GtkFileSelection* widget)
{
   return widget->selection_entry;
}

GtkWidget*
ada_file_selection_get_selection_text (GtkFileSelection* widget)
{
   return widget->selection_text;
}

/******************************************
 ** Functions for Dialog
 ******************************************/

GtkWidget*
ada_dialog_get_action_area (GtkDialog* widget)
{
   return widget->action_area;
}

GtkWidget*
ada_dialog_get_vbox (GtkDialog* widget)
{
   return widget->vbox;
}

/******************************************
 ** Functions for Combo
 ******************************************/

GtkWidget*
ada_combo_get_entry (GtkCombo* widget)
{
  return widget->entry;
}

void
ada_combo_set_entry (GtkCombo* widget, GtkWidget* entry)
{
  widget->entry = entry;
}

GtkWidget*
ada_combo_get_list (GtkCombo* widget)
{
  return widget->list;
}

/********************************************
 ** Functions for Widget
 ********************************************/

GtkStyle*
ada_widget_get_style (GtkWidget* widget)
{
  return widget->style;
}

GdkWindow*
ada_widget_get_window (GtkWidget* widget)
{
  return widget->window;
}

GtkWidget*
ada_widget_get_parent (GtkWidget* widget)
{
  return widget->parent;
}

gint
ada_widget_get_motion_notify (GtkWidget* widget, GdkEvent* event)
{
  return (GTK_WIDGET_CLASS (GTK_OBJECT (widget)->klass)
	  ->motion_notify_event)(widget, (GdkEventMotion*)event);
}

/******************************************
 ** Functions for Pixmap
 ******************************************/

GdkBitmap*
ada_pixmap_get_mask (GtkPixmap* widget)
{
   return widget->mask;
}

GdkPixmap*
ada_pixmap_get_pixmap (GtkPixmap* widget)
{
   return widget->pixmap;
}

/*******************************************
 ** Functions for Notebook
 *******************************************/

gint
ada_notebook_get_tab_pos (GtkNotebook* widget)
{
   return widget->tab_pos;
}

GList*
ada_notebook_get_children (GtkNotebook* widget)
{
  return widget->children;
}

GtkNotebookPage*
ada_notebook_get_cur_page (GtkNotebook* widget)
{
  return widget->cur_page;
}

GtkWidget*
ada_notebook_get_menu_label (GtkNotebookPage* widget)
{
  return widget->menu_label;
}

GtkWidget*
ada_notebook_get_tab_label (GtkNotebookPage* widget)
{
  return widget->tab_label;
}

/**********************************************
 ** Functions for NotebookPage
 **********************************************/

GtkNotebookPage*
ada_notebook_page_get_child (GtkNotebookPage* widget)
{
  return (GtkNotebookPage*)widget->child;
}

/**********************************************
 ** Functions for Box
 **********************************************/

GtkWidget*
ada_box_get_child (GtkBox* widget, gint num)
{
  if (num < g_list_length (widget->children))
    return ((GtkBoxChild*)(g_list_nth_data (widget->children, num)))->widget;
  else
    return NULL;
}

/**********************************************
 ** Functions for Glib.Glist
 **********************************************/

GList*
ada_list_next (GList* list)
{
  if (list)
    return list->next;
  else
    return NULL;
}

GList*
ada_list_prev (GList* list)
{
  if (list)
    return list->prev;
  else
    return NULL;
}

gpointer
ada_list_get_data (GList* list)
{
  return list->data;
}

/**********************************************
 ** Functions for Glib.GSlist
 **********************************************/

GSList*
ada_gslist_next (GSList* list)
{
  if (list)
    return list->next;
  else
    return NULL;
}

gpointer
ada_gslist_get_data (GSList* list)
{
  return list->data;
}

/******************************************
 ** Functions for Fixed
 ******************************************/

GList*
ada_fixed_get_children (GtkFixed* widget)
{
   return widget->children;
}

/******************************************
 ** Functions for Status bar
 ******************************************/

GSList *
ada_status_get_messages (GtkStatusbar* widget)
{
  return widget->messages;
}

/******************************************
 ** Functions for GtkList
 ******************************************/

GList*
ada_list_get_children (GtkList* widget)
{
   return widget->children;
}

GList*
ada_list_get_selection (GtkList* widget)
{
   return widget->selection;
}

/******************************************
 ** Functions for Tree
 ******************************************/

GList*
ada_tree_get_children (GtkTree* widget)
{
   return widget->children;
}

GList*
ada_tree_get_selection (GtkTree* widget)
{
   return widget->selection;
}

/*******************************************
 ** Functions for Tree_Item
 *******************************************/

GtkWidget*
ada_tree_item_get_subtree (GtkTreeItem* widget)
{
  return GTK_TREE_ITEM_SUBTREE (widget);
}


/******************************************
 ** Functions for Gtk_CList
 ******************************************/

gint
ada_clist_get_focus_row (GtkCList* widget)
{
  return widget->focus_row;
}

gint
ada_clist_get_columns (GtkCList* clist)
{
  return clist->columns;
}

gint
ada_clist_get_rows (GtkCList* clist)
{
  return clist->rows;
}

GList*
ada_clist_get_selection (GtkCList* widget)
{
   return widget->selection;
}

gint
ada_clist_get_selection_mode (GtkCList* widget)
{
  return widget->selection_mode;
}

GdkWindow*
ada_clist_get_clist_window (GtkCList* widget)
{
  return widget->clist_window;
}

GList*
ada_clist_get_row_list (GtkCList* widget)
{
  return widget->row_list;
}

GtkSortType
ada_gtk_clist_get_sort_type (GtkCList* widget) {
  return widget->sort_type;
}

gint
ada_gtk_clist_get_sort_column (GtkCList* widget) {
  return widget->sort_column;
}

void
ada_gtk_clist_set_cell_contents (GtkCList* clist,
				 GtkCListRow* row,
				 gint column,
				 GtkCellType type,
				 char* string,
				 guint8 spacing,
				 GdkPixmap *pixmap,
				 GdkBitmap *mask)
{
  GTK_CLIST_CLASS (((GtkObject*)clist)->klass)->set_cell_contents
     (clist, row, column, type, string, spacing, pixmap, mask);
}

int
ada_gtk_clist_get_text (GtkCList* clist,
			GtkCListRow * row,
			gint column,
			gchar** string)
{
  if (row->cell[column].type == GTK_CELL_TEXT)
    *string = GTK_CELL_TEXT (row->cell[column])->text;
  else if (row->cell[column].type == GTK_CELL_PIXTEXT)
    *string = GTK_CELL_PIXTEXT (row->cell[column])->text;
  else
    return 0;
  return 1;
}

int
ada_gtk_clist_get_pixmap (GtkCList    *clist,
			  GtkCListRow *row,
			  gint         column,
			  GdkPixmap  **pixmap,
			  GdkBitmap  **mask)
{
  if (row->cell[column].type == GTK_CELL_PIXMAP) {
    *pixmap = GTK_CELL_PIXMAP (row->cell[column])->pixmap;
    *mask = GTK_CELL_PIXMAP (row->cell[column])->mask;
  } else if (row->cell[column].type == GTK_CELL_PIXTEXT) {
    *pixmap = GTK_CELL_PIXTEXT (row->cell[column])->pixmap;
    *mask = GTK_CELL_PIXTEXT (row->cell[column])->mask;
  } else {
    return 0;
  }
  return 1;
}

gpointer
ada_gtk_clist_get_row_data (GtkCList    *clist,
			    GtkCListRow *row)
{
  return row->data;
}

void
ada_gtk_clist_set_row_data_full (GtkCList        *clist,
				 GtkCListRow     *row,
				 gpointer         data,
				 GtkDestroyNotify destroy)
{
  row->data = data;
  row->destroy = destroy;
}

/******************************************
 ** Functions for Gtk_Packer
 ******************************************/

GtkPackerChild*
ada_packer_find_child (GtkPacker* packer, GtkWidget* child) {
  GList * list;

  list = g_list_first (packer->children);
  while (list) {
    if (((GtkPackerChild*)(list->data))->widget == child) {
      return (GtkPackerChild*)(list->data);
    }
    list = g_list_next(list);
  }
  return NULL;
}

GtkPackerChild*
ada_packer_get_nth_child (GtkPacker* packer, guint n) {
  GList * list;
  guint m = 1;

  list = g_list_first (packer->children);
  while (list && m != n) {
    list = g_list_next (list);
    m ++;
  }
  if (list) {
    return (GtkPackerChild*)(list->data);
  }
  return NULL;
}

GtkAnchorType
ada_gtk_packer_get_anchor (GtkPackerChild* child) {
  return child->anchor;
}

GtkSideType
ada_gtk_packer_get_side (GtkPackerChild* child) {
  return child->side;
}

GtkPackerOptions
ada_gtk_packer_get_options (GtkPackerChild* child) {
  return child->options;
}

guint
ada_gtk_packer_get_border_width (GtkPackerChild* child) {
  return child->border_width;
}

guint
ada_gtk_packer_get_pad_x (GtkPackerChild* child) {
  return child->pad_x;
}

guint
ada_gtk_packer_get_pad_y (GtkPackerChild* child) {
  return child->pad_y;
}

guint
ada_gtk_packer_get_i_pad_x (GtkPackerChild* child) {
  return child->i_pad_x;
}

guint
ada_gtk_packer_get_i_pad_y (GtkPackerChild* child) {
  return child->i_pad_y;
}

/******************************************
 ** Functions for CTree
 ******************************************/

GtkCTreeExpanderStyle
ada_ctree_get_expander_style (GtkCTree* widget)
{
  return widget->expander_style;
}

GtkCTreeLineStyle
ada_ctree_get_line_style (GtkCTree* widget)
{
  return widget->line_style;
}

gboolean
ada_ctree_get_show_stub (GtkCTree* widget)
{
  return widget->show_stub;
}

gint
ada_ctree_get_tree_column (GtkCTree* widget)
{
   return widget->tree_column;
}

gint
ada_ctree_get_tree_indent (GtkCTree* widget)
{
   return widget->tree_indent;
}

gint
ada_ctree_get_tree_spacing (GtkCTree* widget)
{
   return widget->tree_spacing;
}

GtkCTreeRow*
ada_ctree_node_get_row (GtkCTreeNode* node)
{
  return GTK_CTREE_ROW (node);
}

GtkCTreeNode*
ada_ctree_row_get_children (GtkCTreeRow* row)
{
  g_return_val_if_fail (row != NULL, NULL);
  return row->children;
}

guint
ada_ctree_row_get_expanded (GtkCTreeRow* row)
{
  g_assert (row != NULL);
  return row->expanded;
}

guint
ada_ctree_row_get_is_leaf (GtkCTreeRow* row)
{
  g_assert (row != NULL);
  return row->is_leaf;
}

GtkCTreeNode*
ada_ctree_row_get_parent (GtkCTreeRow* row)
{
  g_return_val_if_fail (row != NULL, NULL);
  return row->parent;
}

GtkCTreeNode*
ada_ctree_row_get_sibling (GtkCTreeRow* row)
{
  g_return_val_if_fail (row != NULL, NULL);
  return row->sibling;
}


/*
 *
 * GdkWindowAttr
 *
 */

GdkWindowAttr*
ada_gdk_window_attr_new (void)
{
  GdkWindowAttr *result;
  
  result = (GdkWindowAttr*) g_new (GdkWindowAttr, 1);

  if (result)
    {
      result->title = NULL;
      result->visual = NULL;
      result->colormap = NULL;
      result->cursor = NULL;
      result->wmclass_name = NULL;
      result->wmclass_class = NULL;
      /* 
       * Here, we only set the pointers to NULL to avoid any dangling
       * pointer. All the other values are left as is. It is the
       * responsibility of the client to make sure they are properly
       * set before they are accessed.
       */
    }

  return result;
}

void
ada_gdk_window_attr_destroy (GdkWindowAttr *window_attr)
{
  g_return_if_fail (window_attr != NULL);

  if (window_attr->title) g_free (window_attr->title);
  if (window_attr->visual) gdk_visual_unref (window_attr->visual);
  if (window_attr->colormap) gdk_colormap_unref (window_attr->colormap);
  if (window_attr->cursor) gdk_cursor_destroy (window_attr->cursor);
  if (window_attr->wmclass_name) g_free (window_attr->wmclass_name);
  if (window_attr->wmclass_class) g_free (window_attr->wmclass_class);

  g_free (window_attr);
}

void
ada_gdk_window_attr_set_title (GdkWindowAttr *window_attr,
			       gchar * title)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->title = title;
}

gchar*
ada_gdk_window_attr_get_title (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->title;
}

void
ada_gdk_window_attr_set_event_mask (GdkWindowAttr *window_attr,
				    gint event_mask)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->event_mask = event_mask;
}

gint
ada_gdk_window_attr_get_event_mask (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, 0);
  
  return window_attr->event_mask;
}

void
ada_gdk_window_attr_set_x (GdkWindowAttr * window_attr, gint16 x)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->x = x;
}

gint16
ada_gdk_window_attr_get_x (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, 0);
  
  return window_attr->x;
}

void
ada_gdk_window_attr_set_y (GdkWindowAttr * window_attr, gint16 y)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->y = y;
}
  
gint16
ada_gdk_window_attr_get_y (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, 0);
  
  return window_attr->y;
}

void
ada_gdk_window_attr_set_width (GdkWindowAttr * window_attr, gint16 width)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->width = width;
}
  
gint16
ada_gdk_window_attr_get_width (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, 0);
  
  return window_attr->width;
}

void
ada_gdk_window_attr_set_height (GdkWindowAttr * window_attr, gint16 height)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->height = height;
}
  
gint16
ada_gdk_window_attr_get_height (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, 0);
  
  return window_attr->height;
}

void
ada_gdk_window_attr_set_wclass (GdkWindowAttr *window_attr,
				GdkWindowClass wclass)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->wclass = wclass;
}

GdkWindowClass
ada_gdk_window_attr_get_wclass (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, GDK_INPUT_OUTPUT);
  
  return window_attr->wclass;
}

void
ada_gdk_window_attr_set_visual (GdkWindowAttr *window_attr,
				GdkVisual *visual)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->visual = visual;
}

GdkVisual*
ada_gdk_window_attr_get_visual (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->visual;
}

void
ada_gdk_window_attr_set_colormap (GdkWindowAttr *window_attr,
				  GdkColormap *colormap)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->colormap = colormap;
}

GdkColormap*
ada_gdk_window_attr_get_colormap (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->colormap;
}

void
ada_gdk_window_attr_set_window_type (GdkWindowAttr *window_attr,
				     GdkWindowType window_type)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->window_type = window_type;
}

GdkWindowType
ada_gdk_window_attr_get_window_type (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, GDK_WINDOW_ROOT);
  
  return window_attr->window_type;
}

void
ada_gdk_window_attr_set_cursor (GdkWindowAttr *window_attr,
				GdkCursor *cursor)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->cursor = cursor;
}

GdkCursor*
ada_gdk_window_attr_get_cursor (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->cursor;
}

void
ada_gdk_window_attr_set_wmclass_name (GdkWindowAttr *window_attr,
				      gchar *wmclass_name)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->wmclass_name = wmclass_name;
}

gchar*
ada_gdk_window_attr_get_wmclass_name (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->wmclass_name;
}

void
ada_gdk_window_attr_set_wmclass_class (GdkWindowAttr *window_attr,
				      gchar *wmclass_class)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->wmclass_class = wmclass_class;
}

gchar*
ada_gdk_window_attr_get_wmclass_class (GdkWindowAttr *window_attr)
{
  g_return_val_if_fail (window_attr != NULL, NULL);
  
  return window_attr->wmclass_class;
}

void
ada_gdk_window_attr_set_override_redirect (GdkWindowAttr *window_attr,
					   gboolean override_redirect)
{
  g_return_if_fail (window_attr != NULL);
  
  window_attr->override_redirect = override_redirect;
}

gboolean
ada_gdk_window_attr_get_override_redirect (GdkWindowAttr * window_attr)
{
  g_return_val_if_fail (window_attr != NULL, FALSE);
  
  return window_attr->override_redirect;
}

/*
 *
 * Gdk properties
 *
 */

void
ada_gdk_property_get (GdkWindow	 *window,
		      GdkAtom     property,
		      GdkAtom     type,
		      gulong      offset,
		      gulong      length,
		      gint        pdelete,
		      GdkAtom    *actual_property_type,
		      gint       *actual_format,
		      gint       *actual_length,
		      guchar    **data,
		      gint       *success)
{
  *success = gdk_property_get (window, property, type, offset, length,
			       pdelete, actual_property_type, actual_format,
			       actual_length, data);
}

#if GTK_MICRO_VERSION < 4
/* Define gtk_container_set_reallocate_redraws for Gtk+ versions < 1.2.4 */
void
gtk_container_set_reallocate_redraws (GtkContainer *container,
                                      gboolean      needs_redraws)
{
  g_warning
    ("Gtk+ %d.%d.%d does not provide gtk_container_set_reallocate_redraws\n",
     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
}
#endif

#if GTK_MINOR_VERSION > 2
/* For Gtk+ 1.3.0 snapshots and future 1.4.x, the following functions have
   been obsoleted and replaced by macros. Since we cannot import macros from
   Ada, define the proper wrappers to stay compatible with 1.2.x */

#undef gdk_draw_pixmap
#undef gdk_draw_bitmap
#undef gdk_window_get_size
#undef gdk_window_get_type
#undef gdk_window_get_colormap
#undef gdk_window_set_colormap
#undef gdk_window_get_visual
#undef gdk_window_ref
#undef gdk_window_unref
#undef gdk_bitmap_ref
#undef gdk_bitmap_unref
#undef gdk_pixmap_ref
#undef gdk_pixmap_unref
#undef gdk_window_copy_area
#undef gdk_gc_destroy
#undef gdk_image_destroy
#undef gdk_cursor_destroy

void
gdk_draw_pixmap (GdkDrawable  *drawable,
                 GdkGC        *gc,
                 GdkDrawable  *src,
                 gint          xsrc,
                 gint          ysrc,
                 gint          xdest,
                 gint          ydest,
                 gint          width,
                 gint          height)
{
  gdk_draw_drawable
    (drawable, gc, src, xsrc, ysrc, xdest, ydest, width, height);
}

void
gdk_window_get_size (GdkWindow      *drawable,
                     gint           *width,
                     gint           *height)
{
  gdk_drawable_get_size (drawable, width, height);
}

GdkDrawableType
gdk_window_get_type (GdkDrawable    *window)
{
  return gdk_drawable_get_type (window);
}

GdkColormap*
gdk_window_get_colormap (GdkDrawable    *drawable)
{
  return gdk_drawable_get_colormap (drawable);
}

void
gdk_window_set_colormap (GdkDrawable    *drawable,
                         GdkColormap    *colormap)
{
  gdk_drawable_set_colormap (drawable, colormap);
}

GdkVisual*
gdk_window_get_visual   (GdkDrawable    *drawable)
{
  return gdk_drawable_get_visual (drawable);
}

GdkDrawable*
gdk_window_ref          (GdkDrawable    *drawable)
{
  return gdk_drawable_ref (drawable);
}

void
gdk_window_unref        (GdkDrawable    *drawable)
{
  gdk_drawable_unref (drawable);
}

GdkBitmap *
gdk_bitmap_ref          (GdkDrawable    *drawable)
{
  return gdk_drawable_ref (drawable);
}
 
void
gdk_bitmap_unref        (GdkDrawable    *drawable)
{
  gdk_drawable_unref (drawable);
}

GdkPixmap *
gdk_pixmap_ref          (GdkDrawable    *drawable)
{
  return gdk_drawable_ref (drawable);
}
 
void
gdk_pixmap_unref        (GdkDrawable    *drawable)
{
  gdk_drawable_unref (drawable);
}

void
gdk_window_copy_area   (GdkWindow    *window,
                        GdkGC        *gc,
                        gint          x,
                        gint          y,
                        GdkWindow    *source_window,
                        gint          source_x,
                        gint          source_y,
                        gint          width,
                        gint          height)
{
  gdk_draw_pixmap
    (window, gc, source_window, source_x, source_y, x, y, width, height);
}

void
gdk_gc_destroy          (GdkGC            *gc)
{
  gdk_gc_unref (gc);
}

void
gdk_image_destroy       (GdkImage     *image)
{
  gdk_image_unref (image);
}

void
gdk_cursor_destroy      (GdkCursor      *cursor)
{
  gdk_cursor_unref (cursor);
}
#endif
