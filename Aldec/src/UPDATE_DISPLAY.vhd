library IEEE;
use IEEE.std_logic_1164.all;

entity UPDATE_DISPLAY is
	 port(
		 UPDATE_N : in STD_LOGIC;
		 UPDATE_X : in STD_LOGIC;
		 CLK : in STD_LOGIC; 
		 RST : in STD_LOGIC;
		 DATA : in STD_LOGIC_VECTOR(0 to 7);
		 ADDR : in STD_LOGIC_VECTOR(0 to 7);
		 BUSY : out STD_LOGIC;
		 READ : out STD_LOGIC;
		 LED_OUT : out STD_LOGIC
	     );
end UPDATE_DISPLAY;


architecture UPDATE_DISPLAY of UPDATE_DISPLAY is 
	signal R 		: STD_LOGIC_VECTOR(0 to 7);
	signal G		: STD_LOGIC_VECTOR(0 to 7);
	signal B		: STD_LOGIC_VECTOR(0 to 7); 
	signal SEND 	: STD_LOGIC; 
	signal RDY   	: STD_LOGIC;
begin 
	modulator : entity work.WS2812
	--generic map(PERIOD => 6, ONE=>4, ZERO=>2)
	port map(
			RST 	=> RST,
			CLK 	=> CLK,
			LED_OUT => LED_OUT,
			R 	  	=> R,
			G	  	=> G,
			B	  	=> B,
			SEND    => SEND,

			RDY   	=> RDY); 


end UPDATE_DISPLAY;
