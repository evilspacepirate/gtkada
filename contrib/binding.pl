#!/usr/bin/env perl
use warnings;
use strict;

our ($ada_dir)="/home/briot/Ada/GtkAda/src/";
#$c_dir  ="/home/briot/gtk/gtk+-2.9/gtk+-2.9.0/";
our ($c_dir)  ="/home/briot/gtk/gtk+-2.8/gtk+-2.8.17/";

our ($debug) = 0;

## parameters are of the form "gtkbutton", "gtkbutton.h"
## They shouldn't contain directory indication
our (@c_files)=@ARGV;

## Find out the name of the Ada unit containing the binding to a specific
## c file

sub replace_word() {
  my ($file) = shift;
  my ($word) = shift;
  $file =~ s/([-_])($word)(.+)/$1$2_$3/;
  return $file;
}

sub ada_unit_from_c_file() {
  my ($cfile) = shift;
  my ($adafile) = $cfile;
  my (@words);
  $adafile =~ s,.*?/([^/]+?)(\.[ch])?$,$1,;  ## basename, no extension
  $adafile =~ s/^gtk(.+)/gtk-$1/;

  ## Order matters in this array
  @words = ("about", "accel", "action", "aspect", "button", "cell", "check",
            "color", "combo", "drawing", "event", "file", "recent", "chooser",
            "font", "handle", "icon", "input", "item", "list", "option",
            "renderer", "radio", "tearoff", "separator",
            "menu", "system", "size", "status", "toggle", "text",
            "tag", "tree", "view", "model", "printer", "operation", "page", "paper",
            "progress", "scrolled", "message", "action");
  for (@words) {
     $adafile = &replace_word ($adafile, $_);
  }
  $adafile =~ s/-bbox/-button_box/;
  $adafile =~ s/([^-])entry$/$1_entry/;
  $adafile =~ s/-entry$/-gentry/;
  $adafile =~ s/-entry(.+)/-entry_$1/;
  $adafile =~ s/_seldialog/_selection_dialog/;
  $adafile =~ s/_sel$/_selection/;
  $adafile =~ s/-toolitem/-tool_item/;
  $adafile =~ s/view_port/viewport/;
  return $adafile;
}

## Find out all C functions bound by a given Ada unit (.ads and .adb files).
## Returns a hash-table indexed by C functions, containing the name of the Ada
## subprogram
our $import_re      = 'pragma\s+Import\s+\(C\s*,\s*(\w+)\s*,\s*"(\w+)"\s*\)';
our $obsolescent_re = 'pragma\s+Obsolescent.*?--  (\w+)';

sub ada_bindings_in_unit() {
  my ($unit) = shift;
  my ($contents);
  my (%binding, %obsolescent);

  if (-f "$ada_dir/$unit.ads") {
     open (FILE, "$ada_dir/$unit.ads");
     $contents = join ("", <FILE>);
     close (FILE);
     print STDERR "Ada Unit $unit\n" if ($debug);
  } else {
     print STDERR "Ada unit doesn't exist yet: $unit.ads\n";
     return (0, %binding, %obsolescent);
  }

  if (-f "$ada_dir/$unit.adb") {
     open (FILE, "$ada_dir/$unit.adb");
     $contents .= join ("", <FILE>);
     close (FILE);
  }

  while ($contents =~ /($import_re)|($obsolescent_re)/iog) {
     if (defined $5) {
        $obsolescent{$5} = 1;
     } else {
        $binding{$3} = $2;
     }
  }

  return (1, \%binding, \%obsolescent);
}

## Find out all C functions defined in a C file.
## Return a hash table indexed on the functions
sub functions_from_c_file() {
  my ($fullname) = shift;
  my (%funcs, $contents);
  my ($deprecated) = 0;

  open (FILE, $fullname);
  $contents = join ("", <FILE>);
  close (FILE);

  while ($contents =~ /(ifndef|endif).*GTK_DISABLE_DEPRECATED|\b(\w+(\s*\*)?)\s*(\w+)\s+\(([^)]*\))(\s*G_GNUC_CONST)?;/g) {
     if (defined $1 && $1 eq "ifndef") {
        $deprecated = 1;
     } elsif (defined $1 && $1 eq "endif") {
        $deprecated = 0;
     } else {
        my ($returns, $args) = ($2, $5);
        ## Ignore internal gtk+ functions:
        if (substr($4,0,1) ne '_') {
           $funcs{$4} = [$args, $returns, $deprecated];
        }
    }
  }

  return %funcs;
}

## Capitalize a string (every letter after _)
sub capitalize () {
  my ($str) = shift;
  $str =~ s/^(.)/\u$1/;
  $str =~ s/_(.)/_\u$1/g;
  return $str;
}

## Return the Ada widget from a C widget.
## This doesn't return the final _Record
sub c_widget_to_ada () {
  my ($c) = shift;
  $c =~ /Gtk(.+)\*/;
  return "Gtk_Tool_Item" if ($c eq "GtkToolItem*");
  return "Gtk_$1";
}

## Return the Ada type to use for a given C type
sub c_to_ada() {
   my ($c_type) = shift;
   my ($param_index) = shift;  ## -1 for return type
   return "Boolean"            if ($c_type eq "gboolean"); 
   return "Gtk_Relief_Style"   if ($c_type eq "GtkReliefStyle");
   return "Gtk_Orientation"    if ($c_type eq "GtkOrientation");
   return "Gtk_Toolbar_Style"  if ($c_type eq "GtkToolbarStyle");
   return "Gtk_Icon_Size"      if ($c_type eq "GtkIconSize");
   return "Gfloat"             if ($c_type eq "gfloat"); 
   return "String"             if ($c_type eq "gchar*");
   return "out Gfloat"         if ($c_type eq "gfloat*");

   if ($c_type =~ /Gtk(.+)\*/) {
      if ($param_index == -1) {
         return &c_widget_to_ada ($c_type);
      } elsif ($param_index == 0) {
         return "access " . &c_widget_to_ada ($c_type) . "_Record";
      } else {
         return "access " . &c_widget_to_ada ($c_type) . "_Record'Class";
      }
   }
   return &capitalize ($c_type);
}

## Same as c_to_ada, but return the type to use in the function
## that directly imports the C function
sub c_to_low_ada() {
   my ($c_type) = shift;
   my ($param_index) = shift; ## -1 for return type
   return "Gboolean"       if ($c_type eq "gboolean");
   return "Gtk_Relief_Style" if ($c_type eq "GtkReliefStyle");
   return "Gtk_Orientation"  if ($c_type eq "GtkOrientation");
   return "Gtk_Toolbar_Style"  if ($c_type eq "GtkToolbarStyle");
   return "Gtk_Icon_Size"  if ($c_type eq "GtkIconSize");
   return "Gfloat"         if ($c_type eq "gfloat");
   return "out Gfloat"     if ($c_type eq "gfloat*");
   return "String"         if ($c_type eq "gchar*");
   return "System.Address" if ($c_type =~ /Gtk(.+)\*/);
   return &capitalize ($c_type);
}

## Return the code to pass or convert back an Ada type to a
## C type
sub c_to_call_ada() {
   my ($name) = shift;
   my ($type) = shift;
   return "Boolean'Pos ($name)" if ($type eq "gboolean");
   return "$name & ASCII.NUL"   if ($type eq "gchar*");
   if ($type =~ /Gtk(.+)\*/) {
      return "Get_Object ($name)";
   }
   return $name;
}

## Return the length of the longuest name in the array
sub longuest() {
   my ($longuest) = 0;
   foreach (@_) {
      $longuest=length($_) if (length ($_) > $longuest);
   }
   return $longuest;
}

## Output the list of parameters for the function
sub output_params() {
   my ($convert) = shift;
   my ($indent) = shift;
   my ($rargs) = shift;
   my ($rarg_types) = shift;
   my ($returns) = shift;
   my (@args) = @$rargs;
   my (@arg_types) = @$rarg_types;
   my ($longuest) = &longuest (@args);

   if ($#args >= 0) {
      print $indent, "  (";
      my ($index) = 0;
      while ($index <= $#args) {
         my ($n) = &capitalize ($args[$index]);
         print $indent, "   " if ($index != 0);
         print $n;
         print ' ' x ($longuest - length ($n)), " : ";
         print &$convert ($arg_types[$index], $index);
         print "", (($index != $#args) ? ";\n" : ")");
         $index++;
      }
   }
   if ($returns ne "void") {
      print "\n$indent   return ", &$convert ($returns, -1);
   }
}

## Name of the Ada entity, given its C name
sub ada_entity_from_c() {
   my ($ada_pkg) = shift;
   my ($func) = shift;
   my ($name) = $func;

   $ada_pkg =~ s/-/_/g;
   $name =~ s/${ada_pkg}_//i;
   $name = &capitalize ($name);
   return $name;
}

## Show the binding for a specific C function. This is only a rough binding,
## and needs to be reviewed manually
sub create_binding() {
   my ($ada_pkg) = shift;
   my ($func) = shift;
   my ($args) = shift;
   my ($returns) = shift; 
   my ($specs_only) = shift;
   my ($deprecated) = shift;
   my (@args, @arg_types);
   my ($longuest_param) = 0;

   my ($name) = &ada_entity_from_c ($ada_pkg, $func);

   $returns =~ s/\s//;

   $args =~ s/\s+/ /g;
   $args =~ s/ \* ?/\* /g;

   while ($args =~ / ?(\w+\*?) (\w+) ?[,)]/g) {
      push (@args, $2);
      $longuest_param = (length ($2) > $longuest_param) ? length ($2) : $longuest_param;
      push (@arg_types, $1);
   }

   if (!$specs_only) {
      # Subprogram box
      print "\n   ", '-' x (length($name) + 6), "\n";
      print "   -- $name --\n";
      print "   ", '-' x (length($name) + 6), "\n\n";
   }

   ## Prototype of Ada subprogram
   print (($returns eq "void") ? "   procedure " : "   function ");
   print $name, "\n";
   &output_params (\&c_to_ada, "   ", \@args, \@arg_types, $returns);

   if ($specs_only) {
     print ";\n";
     if ($deprecated) {
        print "   pragma Obsolescent;\n";
     }
     return;
   }

   print "\n";
   print "   is\n";

   ## Prototype for Internal
   print "      ", (($returns eq "void") ? "procedure " : "function ");
   print "Internal\n";
   &output_params (\&c_to_low_ada, "      ", \@args, \@arg_types, $returns);
   print ";\n";
   print "      pragma Import (C, Internal, \"$func\");\n";

   ## If we are returning a complex Widget type
   if ($returns =~ /Gtk(.+)\*/) {
      print "      Stub : ". &c_widget_to_ada ($returns) . "_Record;\n";
   }

   ## The body of the Ada subprogram
   print "   begin\n";

   ## Call to Internal
   my ($index) = 0;
   if ($returns ne "void") {
      if ($returns =~ /Gtk(.+)\*/) {
         print "      return ", &c_widget_to_ada ($returns) . "\n";
         print "        (Get_User_Data\n";
         print "          (";
      } elsif ($returns eq "gboolean") {
         print "      return Boolean'Val (";
      } else {
         print "      return ";
      }
   } else {
      print "      ";
   }
   print "Internal";

   while ($index <= $#args) {
      if ($index == 0) {
         print " (";
      } else {
         print ", ";
      }
      print &c_to_call_ada (&capitalize ($args[$index]), $arg_types[$index]);
      $index++;
   }
   if ($#args >= 0) {
      if ($returns =~ /Gtk(.+)\*/) {
         print "), Stub));\n";
      } elsif ($returns eq "gboolean") {
         print "));\n";
      } else {
         print ");\n";
      }
   }

   ## End of Ada subprogram
   print "   end $name;\n"; 
}


## Process a specific C file
sub process_c_file() {
  my ($c_file) = shift;
  my ($with_dir);
  my (%funcs, %binding, %obsolescent, $func, $ada_unit, $success);
  my ($binding, $obsolescent);

  if (-f $c_file) {
    $with_dir = $c_file;
  } elsif (-f "$c_dir/gtk/$c_file") {
    $with_dir = "$c_dir/gtk/$c_file";
  } elsif (-f "$c_dir/gtk/$c_file.h") {
    $with_dir = "$c_dir/gtk/$c_file.h";
  }

  if (!defined $with_dir) {
     print STDERR "File $c_file not found\n";
     return;
  }

  %funcs = &functions_from_c_file ($with_dir);
  $ada_unit = &ada_unit_from_c_file ($c_file);
  ($success, $binding, $obsolescent) = &ada_bindings_in_unit ($ada_unit);
  %binding = %$binding;
  %obsolescent = %$obsolescent;
  return if (!$success);

  foreach $func (sort keys %funcs) {
     if (!defined $binding{$func}) {
        print "No binding for:  $func\n";
     }
  }
  foreach $func (sort keys %binding) {
     ## functions generated for the sake of GtkAda always start with
     ## ada_, so we ignore these for now
     if (substr($func,0,4) ne "ada_" && !defined $funcs{$func}) {
        print "No longer valid: $func\n";
     }
  }

  ## Output specs
  print "\n\n";
  foreach $func (sort keys %funcs) {
     if (!defined $binding{$func}) {
        my ($args, $returns,$deprecated) = ($funcs{$func}->[0], $funcs{$func}->[1], $funcs{$func}->[2]);
        &create_binding ($ada_unit, $func, $args, $returns, 1, $deprecated);
     }
  }

  ## Output list of deprecated subprograms
  foreach $func (sort keys %funcs) {
     my ($args, $returns,$deprecated) =
       ($funcs{$func}->[0], $funcs{$func}->[1], $funcs{$func}->[2]);
     if ($deprecated) {
        my ($name) = &ada_entity_from_c ($ada_unit, $func);
        if (!defined $obsolescent{$name}) {
           print "  pragma Obsolescent; --  $name\n";
        }
     }
  }

  ## Output bodies
  foreach $func (sort keys %funcs) {
     if (!defined $binding{$func}) {
        my ($args, $returns,$deprecated) =
          ($funcs{$func}->[0], $funcs{$func}->[1], $funcs{$func}->[2]);
        &create_binding ($ada_unit, $func, $args, $returns, 0, $deprecated);
     }
  }
}

## Process the command line
our $c_file;
foreach $c_file (@c_files) {
   &process_c_file ($c_file);
}


