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

--  <description>
--  This widget implements a top level window.
--  It is used as the base class for dialogs, ...
--
--  A window has both a default widget (to which events are sent if no other
--  widget has been selected and has the focus), and a focus widget (which
--  gets the events and overrides the default widget).
--
--  You can set many hints on the window (its minimum and maximum size, its
--  decoration, etc.) but these are only hints to the window manager, which
--  might not respect them.
--
--  A useful hint, respected by most window managers, can be used to force
--  some secondary windows to stay on top of the main window on the screen
--  (for instance, so that a smaller window can not be hidden by a bigger
--  one). See the function Set_Transient_For below.
--
--  A window can also be modal, i.e. grab all the mouse and keyboard events
--  in the application while it is displayed.
--
--  </description>
--  <c_version>1.2.6</c_version>

with Gdk.Types;
with Gtk.Accel_Group;
with Gtk.Object;
with Gtk.Bin;
with Gtk.Enums;
with Gtk.Widget;

package Gtk.Window is

   type Gtk_Window_Record is new Bin.Gtk_Bin_Record with private;
   type Gtk_Window is access all Gtk_Window_Record'Class;

   procedure Gtk_New
     (Window   : out Gtk_Window;
      The_Type : in  Gtk.Enums.Gtk_Window_Type := Gtk.Enums.Window_Toplevel);
   --  Create a new window.
   --  The_Type specifies the type of the window, and can be either a
   --  top level window, a dialog or a popup window. You will most often only
   --  need to use Window_Toplevel, the other types are mostly used internally
   --  by gtk+.
   --  A Popup window is used to display a temporary information window. It has
   --  no borders nor resizing handles.

   procedure Initialize
     (Window   : access Gtk_Window_Record'Class;
      The_Type : in Gtk.Enums.Gtk_Window_Type);
   --  Internal initialization function.
   --  See the section "Creating your own widgets" in the documentation.

   function Get_Type return Gtk.Gtk_Type;
   --  Return the internal value associated with a Gtk_Window.

   procedure Set_Title
     (Window : access Gtk_Window_Record;
      Title  : in String);
   --  Change the title of the window, as it appears in the title bar.
   --  Note that on some systems you might not be able to change it.

   procedure Set_Wmclass
     (Window        : access Gtk_Window_Record;
      Wmclass_Name  : in String;
      Wmclass_Class : in String);
   --  Specify the string to look for in the user's configuration files
   --  (Xdefault...) for windows specific resources. See some X11
   --  documentation for more information (man XSetClassHint)
   --  The window should not be realized when you call this function.

   procedure Set_Policy
     (Window       : access Gtk_Window_Record;
      Allow_Shrink : in     Boolean;
      Allow_Grow   : in     Boolean;
      Auto_Shrink  : in     Boolean);
   --  Specify the behavior of the window with regards to size modifications.
   --  Default values when the window is created are:
   --
   --    Allow_Shrink => False,
   --
   --    Allow_Grow   => True,
   --
   --    Auto_Shrink  => False.
   --
   --  If Allow_Shrink is False, then the minimum size of the window is
   --  calculated once depending on its children, and the window can never be
   --  smaller.
   --  If Allow_Grow is False, then the maximum size of the window is
   --  calculated once depending on its children, and the window can never be
   --  bigger.
   --  If Auto_Shrink if False, then the window is not shrinked when its
   --  content changes.

   procedure Add_Accel_Group
     (Window      : access Gtk_Window_Record;
      Accel_Group : in Gtk.Accel_Group.Gtk_Accel_Group);
   --  Specify an accelerator group for the window.

   procedure Remove_Accel_Group
     (Window      : access Gtk_Window_Record;
      Accel_Group : in Gtk.Accel_Group.Gtk_Accel_Group);
   --  Remove the specified accelerator group for the window.

   procedure Set_Position
     (Window   : access Gtk_Window_Record;
      Position : in     Gtk.Enums.Gtk_Window_Position);
   --  Specify how the position of the window should be computed.
   --  If Position is Win_Pos_Center_Always or Win_Pos_Center, then the window
   --  is centered on the screen. In the first case, it is also recentered
   --  when the window is resized with Gtk.Widget.Set_Usize (ie except on
   --  user action).
   --  If Position is Win_Pos_Mouse, then the window is positioned so that it
   --  centered around the mouse.
   --  If Position is Win_Pos_None, no calculation is done. If
   --  Gtk.Widget.Set_Uposition as been called, it is respected. This is the
   --  default case.

   function Activate_Focus (Window : access Gtk_Window_Record) return Boolean;
   --  Call Gtk.Widget.Activate on the widget that currently has the focus in
   --  the window, ie sends an "activate" signal to that widget. Note that this
   --  signal does not really exists and is mapped to some widget-specific
   --  signal.
   --  Return True if the widget could be activated, False otherwise.
   --  The Focus widget is set through a signal "set_focus".

   function Activate_Default
     (Window : access Gtk_Window_Record) return Boolean;
   --  Activate the default widget in the window.
   --  In other words, send an "activate" signal to that widget. Note that
   --  this signal is a virtual one and is mapped to some widget specific
   --  signal.
   --  Return False is the widget could not be activated or if there was
   --  no default widget.
   --  You can set the default widget with the following calls:
   --
   --     Gtk.Widget.Set_Flags (Widget, Can_Default);
   --
   --     Gtk.Widget.Grab_Default (Widget);


   procedure Set_Transient_For
     (Window : access Gtk_Window_Record;
      Parent : access Gtk_Window_Record'Class);
   --  Specify that Window is a transient window.
   --  A transient window is a temporary window, like a popup menu or a
   --  dialog box). Parent is the toplevel window of the application to which
   --  Window belongs. A window that has set this can expect less decoration
   --  from the window manager (for instance no title bar and no borders).
   --  (see XSetTransientForHint(3) on Unix systems)
   --
   --  The main usage of this function is to force Window to be on top of
   --  Parent on the screen at all times. Most window managers respect this
   --  hint, even if this is not mandatory.

   procedure Set_Geometry_Hints
     (Window          : access Gtk_Window_Record;
      Geometry_Widget : Gtk.Widget.Gtk_Widget;
      Geometry        : Gdk.Types.Gdk_Geometry;
      Geom_Mask       : Gdk.Types.Gdk_Window_Hints);
   --  Specify some geometry hints for the window.
   --  This includes its minimal and maximal sizes, ...
   --  These attributes are specified in Geometry.
   --  Geom_Mask indicates which of the fields in Geometry are set.
   --  Geometry_Widget can be null (and thus is not an access parameter). It
   --  adds some extra size to Geometry based on the actual size of
   --  Geometry_Widget (the extra amount is Window'Size - Geometry_Widget'Size)
   --
   --  Geometry.Base_* indicates the size that is used by the window manager
   --  to report the size: for instance, if Base_Width = 600 and actual width
   --  is 200, the window manager will indicate a width of -400.
   --
   --  If your window manager respects the hints (and its doesn't have to),
   --  then the user will never be able to resize the window to a size not
   --  in Geometry.Min_* .. Geometry.Max_*.
   --
   --  Geometry.*_Inc specifies by which amount the size will be multiplied.
   --  For instance, if Width_Inc = 50 and the size reported by the Window
   --  Manager is 2x3, then the actual width of the window is 100.
   --  Your window's size will always be a multiple of the *_Inc values.
   --
   --  Geometry.*_Aspect specifies the aspect ratio for the window. The window
   --  will always be resized so that the ratio between its width and its
   --  height remains in the range Min_Aspect .. Max_Aspect.

   procedure Set_Default_Size
     (Window : access Gtk_Window_Record;
      Width  : in Gint;
      Height : in Gint);
   --  Specify a minimal size for the window.
   --  If its content needs to be bigger, then the actual window will grow
   --  accordingly.
   --  This is different from Gtk.Widget.Set_Usize which sets an absolute
   --  size for the widget.
   --  This has no effect on Popup windows (set in call to Gtk_New).

   procedure Set_Modal
     (Window : access Gtk_Window_Record;
      Modal  : in Boolean := True);
   --  Define the window as being Modal.
   --  It will grab the input from the keyboard and the mouse while it is
   --  displayed and will release it when it is hidden. The grab is only in
   --  effect for the windows that belong to the same application, and will not
   --  affect other applications running on the same screen.
   --  In cunjunction with Gtk.Main.Main, this is the easiest way to show a
   --  dialog to which the user has to answer before the application can
   --  continue.

   procedure Generate (N : in Node_Ptr; File : in File_Type);
   --  Gate internal procedure

   procedure Generate (Window : in out Gtk.Object.Gtk_Object; N : in Node_Ptr);
   --  Dgate internal function

   -------------
   -- Signals --
   -------------

   --  <signals>
   --  The following new signals are defined for this widget:
   --
   --  - "set_focus"
   --    procedure Handler (Window : access Gtk_Window_Record'Class;
   --                       Widget : access Gtk_Widget_Record'Class);
   --
   --    Called when the widget that has the focus has changed.
   --    This widget gets all keyboard events that happen in the window.
   --    You should not block the emission of this signal, since most of
   --    the work is done in the default handler.
   --  </signals>

private
   type Gtk_Window_Record is new Bin.Gtk_Bin_Record with null record;
   pragma Import (C, Get_Type, "gtk_window_get_type");
end Gtk.Window;

--  <example>
--  <include>../examples/documentation/banner.adb</include>
--  </example>
