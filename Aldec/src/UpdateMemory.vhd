library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all ;


entity MemoryUpdater is

	generic
	(
		NUMBER_OF_LEDS:STD_LOGIC_VECTOR(7 downto 0):="01000000"
	);
	port
	(
		-- Input ports
		CE	: in STD_LOGIC;
		NewData	: in  STD_LOGIC;
		Clk : in STD_LOGIC;
		Reset : in STD_LOGIC;
		DataIn : in STD_LOGIC_VECTOR (7 downto 0);

		-- Output ports
		OutputReady : out STD_LOGIC;--to the R/nW RAM's input(shared with UDATE_DISPLAY) 
		Address	: out STD_LOGIC_VECTOR (7 downto 0);
		DataOut	: out STD_LOGIC_VECTOR (23 downto 0);
		nMemEnable : out std_logic
	);
end MemoryUpdater;

architecture rtl of MemoryUpdater is

	-- Build an enumerated type for the state machine
	--type state_type is (s0, s1, s2, s3);

	-- Register to hold the current state
	--signal state   : state_type;
	type state_t is (stateAddr,stateB1,stateB2,stateB3, statePixelReady);
	signal state : state_t;
	signal savedAddress : STD_LOGIC_VECTOR (7 downto 0);
	signal pixelData1 : STD_LOGIC_VECTOR (7 downto 0);
	signal pixelData2 : STD_LOGIC_VECTOR (7 downto 0);
	signal pixelData3 : STD_LOGIC_VECTOR (7 downto 0);

begin

	-- Logic to advance to the next state
	process (Clk, Reset, CE)
	begin
		if (reset = '1' OR CE='0') then
			state <= stateAddr;
		elsif (Clk'event and clk= '0') then
			case state is
				when stateAddr=>   
					if(NewData='1') then
						savedAddress<=DataIn;
						state<=stateB1;	 
					end if;
				when stateB1=>	 
					if(NewData='1') then
						pixelData1<=DataIn;
						state<=stateB2;
					end if;
				when stateB2=> 
					if(NewData='1') then
						pixelData2<=DataIn;
						state<=stateB3;	
					end if;
				when stateB3 =>
					if(NewData='1') then
						pixelData3<=DataIn;
						state<=statePixelReady;	
					end if;
				when statePixelReady =>	
						if(savedAddress /= NUMBER_OF_LEDS ) then
							savedAddress<=savedAddress+1;
						else
							savedAddress<="00000000";
						end if;	
					state<=stateB1;
			end case;
		end if;
	end process;

	--Set Output 
	process (state,CE)
	begin
		if(state=statePixelReady) then
			OutputReady<='0';
			Address<= savedAddress;	
			DataOut<= pixelData1 & pixelData2 & pixelData3;	
			nMemEnable<= '0';
		else
			OutputReady<= 'Z';
			Address<="ZZZZZZZZ";
			DataOut<="ZZZZZZZZZZZZZZZZZZZZZZZZ";
			nMemEnable <= '1';
		end if;	
	end process;

end rtl;
