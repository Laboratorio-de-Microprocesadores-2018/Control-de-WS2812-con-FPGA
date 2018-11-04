library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;

entity WS2812_TB is
end WS2812_TB;

architecture WS2812_TB of WS2812_TB is	

	constant CLK_PERIOD  : time := 10 ns;

	signal RST 		: STD_LOGIC;
	signal CLK 		: STD_LOGIC; 
	signal R 		: STD_LOGIC_VECTOR(0 to 7);
	signal G		: STD_LOGIC_VECTOR(0 to 7);
	signal B		: STD_LOGIC_VECTOR(0 to 7); 
	signal SEND 	: STD_LOGIC; 
	signal LED_OUT	: STD_LOGIC;
	signal RDY   	: STD_LOGIC;

begin
	modulator : entity work.WS2812
	generic map(PERIOD => 10, ONE=>8, ZERO=>2)
	port map(
			RST 	=> RST,
			CLK 	=> CLK,
			R 	  	=> R,
			G	  	=> G,
			B	  	=> B,
			SEND    => SEND,
			LED_OUT => LED_OUT,
			RDY   	=> RDY); 
			
	CLK_P : process
	begin			
		CLK <= '0';
		wait for CLK_PERIOD/2;
		CLK <= '1';	 
		wait for CLK_PERIOD/2;
	end process;  
	
	TEST : process
	begin
		RST <= '1';	
		SEND <='0';
		wait for 30ns;
		RST <= '0';
		wait for 30ns;	
		G<="11110000";
		R<="11001100";
		B<="10101010";
		wait until falling_edge(CLK);
		SEND <= '1';
		wait until falling_edge(CLK);
		SEND <= '0'; 
		
		wait until RDY = '1'; 
		wait for CLK_PERIOD*5;
		R<="00000000";
		G<="00000000";
		B<="00000000";
		wait until falling_edge(CLK);
		SEND <= '1';
		wait until falling_edge(CLK);
		SEND <= '0'; 
		
		wait until RDY = '1';  
		wait for CLK_PERIOD*5;
		R<="11111111";
		G<="11111111";
		B<="11111111";
		wait until falling_edge(CLK);
		SEND <= '1';
		wait until falling_edge(CLK);
		SEND <= '0'; 	 
		
		wait until RDY = '1';  
		wait for CLK_PERIOD*10;
		R<="10000001";
		G<="01111110";
		B<="00011000";
		wait until falling_edge(CLK);
		SEND <= '1';
		wait until falling_edge(CLK);
		SEND <= '0'; 
		
		
		wait; 
	end process;
end WS2812_TB;
