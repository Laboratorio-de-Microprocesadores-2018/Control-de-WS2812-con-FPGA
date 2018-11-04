library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.math_real.all;	  
use ieee.numeric_std.all;

entity Counter is 
	generic(
			MAX_COUNT : natural := 6
	);
	 port(
	 	 CLK : in STD_LOGIC;
	 	 CLR : in STD_LOGIC;
		 EN : in STD_LOGIC;
		 COUNT : out NATURAL range 0 to MAX_COUNT
	     );
end Counter;


architecture Counter of Counter is		 
begin
	process(CLK,CLR)
	begin	
		if(CLK'event and CLK='1') then
			if(CLR='1') then
				COUNT <= 0;
			elsif(EN = '1') then	
				if(COUNT = MAX_COUNT) then
					COUNT <= 0;
				else
					COUNT <= COUNT + 1;	
				end if;
			end if;
		end if;
	end process;

end Counter;
