with Ada.Text_IO; use Ada.Text_IO;

package body Data_Reader is
   function Read_Data_File (File_Name : String) return String is
      File : File_Type;
      Line : String (1 .. 1024);
      Last : Natural;
      Result : String := "";
   begin
      Open (File, In_File, File_Name);
      while not End_Of_File (File) loop
         Get_Line (File, Line, Last);
         Result := Result & Line (1 .. Last) & "\n";
      end loop;
      Close (File);
      return Result;
   exception
      when others =>
         return "[Error reading file]";
   end Read_Data_File;
end Data_Reader; 