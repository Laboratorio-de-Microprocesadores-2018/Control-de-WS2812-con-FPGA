library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.MATH_REAL.ALL;

entity SPI_TB is
end SPI_TB;

architecture SPI_TB of SPI_TB is

    constant CLK_PERIOD  : time := 20 ns;
	constant SCLK_PERIOD : time := 1us;
    signal CLK        : std_logic := '0';
    signal RST        : std_logic := '1';

    signal sclk       : std_logic;
    signal cs_n       : std_logic;
    signal mosi       : std_logic;

    signal s_dout     : std_logic_vector(7 downto 0);
    signal s_dout_vld : std_logic;	   
	signal s_idle 	  : std_logic;

begin

    slave : entity work.SPI
	generic map(WORD_SIZE => 8)
    port map (
        CLK      => CLK,
        RST      => RST,
        -- SPI MASTER INTERFACE
        SCLK     => sclk,
        CS_N     => cs_n,
        MOSI     => mosi,
        -- USER INTERFACE
        DOUT     => s_dout,
        DOUT_VLD => s_dout_vld,
		IDLE 	 => s_idle
    );

    clk_process : process
    begin
        CLK <= '1';
        wait for CLK_PERIOD/2;
        CLK <= '0';
        wait for CLK_PERIOD/2;
    end process;

    rst_process : process
    begin
        RST <= '1';
        wait for 40 ns;
        RST <= '0';
        wait;
    end process;
	
	master_proces : process
	constant T_SIZE : integer := 3;
	type table_t is array(0 to T_SIZE-1) of STD_LOGIC_VECTOR(0 to 7);
	constant table : table_t := ("00000001","00000010","00000100");
    begin  
		cs_n<='0';
		wait for 30ns;
		for i in 0 to T_SIZE-1 loop
			for	j in 0 to 7 loop 
				mosi <= table(i)(j);	 
				sclk <= '1';
		        wait for SCLK_PERIOD/2;
		        sclk <= '0';
		        wait for SCLK_PERIOD/2;
			end loop; 
		end loop;
		
		
		wait for 30ns;
		cs_n<='1';
		wait;
    end process;

end SPI_TB;