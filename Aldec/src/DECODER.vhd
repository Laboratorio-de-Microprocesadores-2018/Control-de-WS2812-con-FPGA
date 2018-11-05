library IEEE;
use IEEE.std_logic_1164.all;

entity DECODER is
	port( 
		CLK : in STD_LOGIC;
		RST : in STD_LOGIC;
		
		CMD : in STD_LOGIC_VECTOR(7 downto 0);
		CMD_RDY : in STD_LOGIC;
		EN : in STD_LOGIC;
		
		C0 : out STD_LOGIC;
		C1 : out STD_LOGIC;
		C2 : out STD_LOGIC
	    );
end DECODER;

architecture DECODER of DECODER is

	type State_t is (IDLE,COMMAND);
	signal state : State_t;		
	signal C0_flag : std_logic;
	signal C1_flag : std_logic;
	signal C2_flag : std_logic;
	signal current_cmd: STD_LOGIC_VECTOR(7 downto 0);
	
begin  
	process(CLK)
	begin	
		if(rising_edge(CLK))then
			case state is
				when IDLE =>   
					 if(CMD_RDY = '1')	then
						current_cmd <= CMD;	  
						state <= COMMAND;
					end if;
				when COMMAND =>
					if(EN = '1') then
						current_cmd <= "00000000";
						state <= IDLE;
					end if;
			end case; 
		end if;
	end process;
	
	process(CLK)
	begin	
		case current_cmd is
			when "00000111" =>
				C0_flag <= '1';
			when "00010000" => 
				C1_flag <= '1';
			when "00010001" => 
				C2_flag <= '1';
			when others =>
				C0_flag <= '0';
				C1_flag <= '0';
				C2_flag <= '0';
			end case;
	end process;
	
	
	
	
	C0 <= C0_flag;	 
	C1 <= C1_flag;	
	C2 <= C2_flag;	
end DECODER;
