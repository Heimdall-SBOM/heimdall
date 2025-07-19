with Ada.Characters.Handling;

package body String_Utils is
   function To_Upper (S : String) return String is
      Result : String (S'Range);
   begin
      for I in S'Range loop
         Result (I) := Ada.Characters.Handling.To_Upper (S (I));
      end loop;
      return Result;
   end To_Upper;
end String_Utils; 