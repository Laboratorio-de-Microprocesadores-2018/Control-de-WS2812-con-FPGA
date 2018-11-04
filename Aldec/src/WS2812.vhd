library IEEE;
use IEEE.std_logic_1164.all;

entity WS2812 is
	generic(
		PERIOD	:integer := 125;   -- Period in CLK ticks
		ONE		:natural := 45;	   -- For a '1' pule, interval of period remaining in high
		ZERO	:natural := 85	   -- For a '0' pule, interval of period remaining in high
	);
	
	port( 
		RST 	: in STD_LOGIC;				   -- Reset signal
		CLK 	: in STD_LOGIC; 			   -- System clock
		R 	  	: in STD_LOGIC_VECTOR(0 to 7); -- Red value
		G	  	: in STD_LOGIC_VECTOR(0 to 7); -- Green value
		B	  	: in STD_LOGIC_VECTOR(0 to 7); -- Blue value
		SEND    : in STD_LOGIC; 			   -- Set to '1' between two falling edges of CLK to send  
		RDY   : out STD_LOGIC;				   -- Flag to notify when 'SEND' is available again
		LED_OUT : out STD_LOGIC			   -- Output modulated signal	
	    );
end WS2812;


architecture WS2812 of WS2812 is  
 
	type State_t is (IDLE,SENDING,READY);	  		-- States enumeration for FSM
													   -- IDLE:    LED_OUT is in '0' and module waiting for SEND='1'.
													   -- SENDING: A color is being sent, so RDY='0'.
													   -- READY:   During the second half of the las bit of color (bit 23)
													   --		   RDY='1' and a new color can be sent. (If no color sent
													   --          during this period, module goes back to IDLE).			
														
	signal state : State_t;							-- Current state
	signal color : STD_LOGIC_VECTOR(0 to 23);       -- Color being sent
	signal LED: STD_LOGIC;							-- Internal signal for LED_OUTPUT
	
begin 
	
	FSM_P : process(CLK) 
	
	variable count : natural;				   	  	-- Counter for timing (resets when reaching PERIOD)
	variable comp : natural;					   	-- When count reaches comp, falling edge in LED 	 
	variable currentBit : natural range 0 to 24;  	-- Index for color
	
		begin
		if(rising_edge(CLK)) then
			if(RST = '1') then					    -- Reset condition
				count :=0; comp := 1; LED <= '0';
				currentBit := 0; state <= IDLE;
			end if;			
			
			case state is
				when IDLE =>
					if(SEND = '1') then 			-- When SEND is asserted latch R,G,B into color 
						color <= G & R & B;			-- Reset variables
						count :=0;	
						currentBit := 0;
						state <= SENDING;			-- And go to SENDIND state
					end if;	
					
				when SENDING =>	
					count := count + 1;				-- Increment counter on every CLK
				
					if(count = 1) then				-- In the first count set 'LED'
						LED <= '1';		 			-- And set the compare value, checking current bit
						comp := ONE when(color(currentBit)='1') else ZERO;  
					elsif(count = comp+1) then 		-- When count exceeds compare value: 
						LED <= '0';	 				-- clear 'LED' and move to next bit. 
						currentBit := currentBit + 1;  
						if(currentBit = 24) then	-- If it's last bit of color, go to READY state
							state <= READY;
						end if;
					elsif (count >= PERIOD) then  	-- Reset counter when exceeds PERIOD 
						count := 0;				
					end if;	
					
					
				when READY => 						-- While being in READY state, a new color can be sent or not.
					count := count + 1;	  		    -- Keep increasing counter
					
					if(SEND = '1') then 			-- If SEND is asserted, latch the new color and go to SENDING state
						color <= R & G & B;	
						currentBit := 0;		
						state <= SENDING;
					elsif(count = PERIOD) then		-- If count reaches PERIOD and SEND has not been asserted, return to IDLE state
						count :=0;
						LED<='0';
						state <= IDLE;
					end if;
			end case;
		end if;
	end process;
	
	-- OUTPUT SIGNAL ASSIGNMENTS
	LED_OUT <= LED;	
	RDY <= '1' when(state /= SENDING) else '0';
		
end WS2812;
