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

with System;

package body Gtk.Toggle_Button is

   ----------------
   -- Get_Active --
   ----------------

   function Get_Active
     (Toggle_Button : access Gtk_Toggle_Button_Record) return Boolean
   is
      function Internal (Widget : System.Address) return Gboolean;
      pragma Import (C, Internal, "gtk_toggle_button_get_active");

   begin
      return To_Boolean (Internal (Get_Object (Toggle_Button)));
   end Get_Active;

   ----------------------
   -- Get_Inconsistent --
   ----------------------

   function Get_Inconsistent
     (Toggle_Button : access Gtk_Toggle_Button_Record) return Boolean
   is
      function Internal (Widget : System.Address) return Gboolean;
      pragma Import (C, Internal, "gtk_toggle_button_get_inconsistent");

   begin
      return To_Boolean (Internal (Get_Object (Toggle_Button)));
   end Get_Inconsistent;

   -------------
   -- Gtk_New --
   -------------

   procedure Gtk_New
     (Toggle_Button : out Gtk_Toggle_Button;
      Label         : String := "") is
   begin
      Toggle_Button := new Gtk_Toggle_Button_Record;
      Initialize (Toggle_Button, Label);
   end Gtk_New;

   ----------------
   -- Initialize --
   ----------------

   procedure Initialize
     (Toggle_Button : access Gtk_Toggle_Button_Record'Class;
      Label         : String := "")
   is
      function Internal (Label : String) return System.Address;
      pragma Import (C, Internal, "gtk_toggle_button_new_with_label");

      function Internal2 return System.Address;
      pragma Import (C, Internal2, "gtk_toggle_button_new");

   begin
      if Label = "" then
         Set_Object (Toggle_Button, Internal2);
      else
         Set_Object (Toggle_Button, Internal (Label & ASCII.NUL));
      end if;

      Initialize_User_Data (Toggle_Button);
   end Initialize;

   ----------------
   -- Set_Active --
   ----------------

   procedure Set_Active
     (Toggle_Button : access Gtk_Toggle_Button_Record;
      Is_Active     : Boolean)
   is
      procedure Internal
        (Toggle_Button : System.Address;
         Is_Active     : Gint);
      pragma Import (C, Internal, "gtk_toggle_button_set_active");

   begin
      Internal (Get_Object (Toggle_Button), Boolean'Pos (Is_Active));
   end Set_Active;

   ----------------------
   -- Set_Inconsistent --
   ----------------------

   procedure Set_Inconsistent
     (Toggle_Button : access Gtk_Toggle_Button_Record;
      Setting       : Boolean := True)
   is
      procedure Internal
        (Toggle_Button : System.Address;
         Setting       : Gboolean);
      pragma Import (C, Internal, "gtk_toggle_button_set_inconsistent");

   begin
      Internal (Get_Object (Toggle_Button), To_Gboolean (Setting));
   end Set_Inconsistent;

   --------------
   -- Set_Mode --
   --------------

   procedure Set_Mode
     (Toggle_Button  : access Gtk_Toggle_Button_Record;
      Draw_Indicator : Boolean)
   is
      procedure Internal
        (Toggle_Button  : System.Address;
         Draw_Indicator : Gint);
      pragma Import (C, Internal, "gtk_toggle_button_set_mode");

   begin
      Internal (Get_Object (Toggle_Button), Boolean'Pos (Draw_Indicator));
   end Set_Mode;

   -------------
   -- Toggled --
   -------------

   procedure Toggled (Toggle_Button : access Gtk_Toggle_Button_Record) is
      procedure Internal (Toggle_Button : System.Address);
      pragma Import (C, Internal, "gtk_toggle_button_toggled");

   begin
      Internal (Get_Object (Toggle_Button));
   end Toggled;

end Gtk.Toggle_Button;
