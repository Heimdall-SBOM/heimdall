with Ada.Text_IO; use Ada.Text_IO;

package body Data_Reader is
   function Read_Data_File (File_Name : String) return String is
   begin
      return "This is a sample data file for the Heimdall Ada demo application." &
             " It is used to demonstrate resource tracking in SBOM generation.";
   exception
      when others =>
         return "[Error reading file]";
   end Read_Data_File;
end Data_Reader; 