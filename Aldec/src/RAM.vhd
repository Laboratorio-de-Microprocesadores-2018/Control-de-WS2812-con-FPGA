 library	ieee;
use	ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity RAM is 
	generic (memorySize : natural := 256);	
	port(address: in std_logic_vector(7 downto 0);
		 data: inout std_logic_vector(23 downto 0);
		 rnw, ncs, clk: in std_logic);	-- read/nWrite, nChipSelect
end RAM;

architecture simpleRam of RAM is
type memory1 is array (0 to memorySize-1) of std_logic_vector(23 downto 0);
signal memory: memory1 := (others => (others => '0'));	
begin	   
	process(clk)  			   
	begin
		if(rising_edge(clk)) then
	   		if(ncs = '1')	then
				data <= (others => 'Z');
			else		   						 
				if (rnw = '1') then	--READ
					data <= memory(conv_integer(address));
				else --WRITE
					memory(conv_integer(address)) <= data;
				end if;	    		
			end if;	
		end if;
	end process;	
end simpleRam;