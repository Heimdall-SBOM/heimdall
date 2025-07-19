with Ada.Text_IO; use Ada.Text_IO;
with Math_Lib; -- static library
with String_Utils; -- dynamic library
with Data_Reader;

procedure Main is
   Data : String := Data_Reader.Read_Data_File ("../data/sample_data.txt");
   Result : Integer := Math_Lib.Factorial (5);
   Upper : String := String_Utils.To_Upper ("heimdall sbom ada demo");
begin
   Put_Line ("Heimdall Ada Demo Application");
   Put_Line ("Factorial(5) from Math_Lib: " & Integer'Image(Result));
   Put_Line ("Uppercase string from String_Utils: " & Upper);
   Put_Line ("Data file contents: " & Data);
end Main; 