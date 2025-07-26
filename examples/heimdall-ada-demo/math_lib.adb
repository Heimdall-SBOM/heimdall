package body Math_Lib is
   function Factorial (N : Integer) return Integer is
      Result : Integer := 1;
   begin
      for I in 1 .. N loop
         Result := Result * I;
      end loop;
      return Result;
   end Factorial;
end Math_Lib; 