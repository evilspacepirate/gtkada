-----------------------------------------------------------------------
--               GtkAda - Ada95 binding for Gtk+/Gnome               --
--                                                                   --
--   Copyright (C) 1998-2000 E. Briot, J. Brobecker and A. Charlet   --
--                Copyright (C) 2000-2001 ACT-Europe                 --
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

--  <c_version>1.3.6</c_version>

with Gtk.Check_Menu_Item;
with Gtk.Widget; use Gtk.Widget;

package Gtk.Radio_Menu_Item is

   type Gtk_Radio_Menu_Item_Record is new
     Gtk.Check_Menu_Item.Gtk_Check_Menu_Item_Record with private;
   type Gtk_Radio_Menu_Item is access all Gtk_Radio_Menu_Item_Record'Class;

   procedure Gtk_New
     (Radio_Menu_Item : out Gtk_Radio_Menu_Item;
      Group           : Widget_SList.GSlist;
      Label           : String := "");

   procedure Gtk_New_With_Mnemonic
     (Radio_Menu_Item : out Gtk_Radio_Menu_Item;
      Group           : Widget_SList.GSlist;
      Label           : String);

   procedure Initialize
     (Radio_Menu_Item : access Gtk_Radio_Menu_Item_Record'Class;
      Group           : Widget_SList.GSlist;
      Label           : String := "");

   procedure Initialize_With_Mnemonic
     (Radio_Menu_Item : access Gtk_Radio_Menu_Item_Record'Class;
      Group           : Widget_SList.GSlist;
      Label           : String);

   function Get_Type return Gtk.Gtk_Type;
   --  Return the internal value associated with a Gtk_Radio_Menu_Item.

   function Group
     (Radio_Menu_Item : access Gtk_Radio_Menu_Item_Record)
      return Widget_SList.GSlist;

   procedure Set_Group
     (Radio_Menu_Item : access Gtk_Radio_Menu_Item_Record;
      Group           : in Widget_SList.GSlist);

   function Selected_Button (In_Group : Widget_SList.GSlist) return Natural;
   --  Returns the button number of the selected button in the group.
   --  Note: This function is not part of Gtk+ itself, but is provided as a
   --  convenient function

   ----------------
   -- Properties --
   ----------------

   --  <properties>
   --  The following properties are defined for this widget. See
   --  Glib.Properties for more information on properties.
   --
   --  </properties>

private
   type Gtk_Radio_Menu_Item_Record is new
     Gtk.Check_Menu_Item.Gtk_Check_Menu_Item_Record with null record;

   pragma Import (C, Get_Type, "gtk_radio_menu_item_get_type");
end Gtk.Radio_Menu_Item;
