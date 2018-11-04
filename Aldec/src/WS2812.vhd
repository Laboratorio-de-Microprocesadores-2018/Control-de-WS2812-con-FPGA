library IEEE;
use IEEE.std_logic_1164.all;

entity WS2812 is
	generic(
		PERIOD	:integer := 125;
		ONE		:natural := 45;
		ZERO	:natural := 85
	);
	
	port( 
		RST 	: in STD_LOGIC;
		CLK 	: in STD_LOGIC; 
		R 	  	: in STD_LOGIC_VECTOR(0 to 7);
		G	  	: in STD_LOGIC_VECTOR(0 to 7);
		B	  	: in STD_LOGIC_VECTOR(0 to 7); 
		SEND    : in STD_LOGIC; 
		LED_OUT : out STD_LOGIC;
		RDY   : out STD_LOGIC
	    );
end WS2812;


architecture WS2812 of WS2812 is  
 
	type State_t is (IDLE,SENDING,READY);	

	signal state : State_t;	
	signal color : STD_LOGIC_VECTOR(0 to 23);       -- Color being sent
	signal LED: STD_LOGIC;							-- Internal signal for LED_OUTPUT
	
begin 
	
	FSM_P : process(CLK) 
	variable count : natural;				   	  		-- Counter for timing	(resets when reaching PERIOD)
	variable comp : natural;					   		-- When count reaches comp, a falling edge in LED 	 
	variable bitCount : natural range 0 to 24;  		-- Index for color
	begin
		if(rising_edge(CLK)) then
			if(RST = '1') then
				count :=0; 
				comp := 1;
				bitCount := 0; 
				state <= IDLE;
				LED <= '0';
			end if;			
			
			case state is
				when IDLE =>
					if(SEND = '1') then 
						color <= G & R & B;
						count :=0;	
						bitCount := 0;
						state <= SENDING;
					end if;	
					
				when SENDING =>	
					count := count + 1;	
					if(count = 1) then
						LED <= '1';		 
						comp := ONE when(color(bitCount)='1') else ZERO;  
					elsif(count = comp+1) then 
						LED <= '0';	 
						bitCount := bitCount + 1;  
						if(bitCount = 24) then	 
							state <= READY;
						end if;
					elsif (count >= PERIOD) then	
						count := 0;				
					end if;	
					
					
				when READY => 
					count := count + 1;	  
					if(count = PERIOD) then
						count :=0;
						LED<='0';
						state <= IDLE;
					elsif(SEND = '1') then 
						color <= R & G & B;	
						bitCount := 0;		
						state <= SENDING;
					end if;
			end case;
		end if;
	end process;
	
	-- OUTPUT SIGNAL ASSIGNMENTS
	LED_OUT <= LED;--'1' when(count < comp) else '0';	
	RDY <= '1' when(state /= SENDING) else '0';
end WS2812;
