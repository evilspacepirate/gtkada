with Glib; use Glib;
with Gtk.Box; use Gtk.Box;
with Gtk.Button; use Gtk.Button;
with Gtk.Container; use Gtk.Container;
with Gtk.Dialog; use Gtk.Dialog;
with Gtk.Label; use Gtk.Label;
with Gtk.Main; use Gtk.Main;
with Gtk.Misc; use Gtk.Misc;
with Gtk.Signal; use Gtk.Signal;
with Gtk.Object; use Gtk.Object;
with Gtk.Status_Bar; use Gtk.Status_Bar;
with Gtk.Widget; use Gtk.Widget;
with Gtk; use Gtk;

package body Create_Test_Idle is

   package Widget_Cb is new Signal.Object_Callback (Gtk_Widget);
   package Label_Idle is new Idle (Gtk_Label'Class);
   package Label_Cb is new Signal.Object_Callback (Gtk_Label);

   Dialog : Gtk_Dialog;
   Idle   : Guint;
   Count  : Integer := 0;

   function Idle_Test (Label : in Gtk_Label'Class) return Boolean is
   begin
      Count := Count + 1;
      Set (Label, "count:" & Integer'Image (Count));
      return True;
   end Idle_Test;

   procedure Stop_Idle (Label : in out Gtk_Label'Class) is
      pragma Warnings (Off, Label);
   begin
      if Idle /= 0 then
         Idle_Remove (Idle);
         Idle := 0;
      end if;
   end Stop_Idle;

   procedure Destroy_Idle (Window : in out Gtk_Widget'Class) is
      Dummy_Label : Gtk_Label;
   begin
      Stop_Idle (Dummy_Label);
      Gtk.Widget.Destroy (Window);
   end Destroy_Idle;

   procedure Start_Idle (Label : in out Gtk_Label'Class) is
   begin
      if Idle = 0 then
         Idle := Label_Idle.Add (Idle_Test'Access, Label);
      end if;
   end Start_Idle;

   Label    : Gtk_Label;
   --  FIXME : label has to be a global variable for the window to be
   --  destroyed correctly. Otherwise, gtk seems stuck in a infinite
   --  loop. Another solution would be to do a Ref (Label) somewhere in
   --  Run.

   procedure Run (Widget : in out Gtk.Button.Gtk_Button'Class) is
      Id       : Guint;
      Button   : Gtk_Button;
   begin

      if not Is_Created (Dialog) then
         Gtk_New (Dialog);
         Id := Widget_Cb.Connect (Dialog, "destroy", Destroy_Idle'Access,
                                  Dialog);
         Set_Title (Dialog, "Idle");
         Border_Width (Dialog, Border_Width => 0);

         Gtk_New (Label, "count : 0");
         Set_Padding (Label, 10, 10);
         Pack_Start (Get_Vbox (Dialog), Label, True, True, 0);
         Show (Label);

         Gtk_New (Button, "close");
         Id := Widget_Cb.Connect (Button, "clicked", Destroy_Idle'Access,
                                  Dialog);
         Set_Flags (Button, Can_Default);
         Grab_Default (Button);
         Pack_Start (Get_Action_Area (Dialog), Button, True, True, 0);
         Show (Button);

         Gtk_New (Button, "start");
         Id := Label_Cb.Connect (Button, "clicked", Start_Idle'Access, Label);
         Set_Flags (Button, Can_Default);
         Pack_Start (Get_Action_Area (Dialog), Button, True, True, 0);
         Show (Button);

         Gtk_New (Button, "stop");
         Id := Label_Cb.Connect (Button, "clicked", Stop_Idle'Access, Label);
         Set_Flags (Button, Can_Default);
         Pack_Start (Get_Action_Area (Dialog), Button, True, True, 0);
         Show (Button);
      end if;

      if not Gtk.Widget.Visible_Is_Set (Dialog) then
         Gtk.Widget.Show (Dialog);
      else
         Gtk.Widget.Destroy (Dialog);
      end if;

   end Run;

end Create_Test_Idle;

