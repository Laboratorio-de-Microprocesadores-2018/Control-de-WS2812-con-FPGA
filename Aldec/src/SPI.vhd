library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.math_real.all;

-- SPI MODULE TO RECEIVE DATA, ONLY MODE 0 (CPOL=0, CPHA=0)

entity SPI is	
	Generic(
		WORD_SIZE : natural;
		BIT_COUNT_MAX : natural := natural(ceil(log2(real(WORD_SIZE))))
	);
    Port (
        CLK      : in  std_logic; -- FPGA clock
        RST      : in  std_logic; -- Active high synchronous reset
        -- SPI SLAVE INTERFACE
        SCLK     : in  std_logic; -- SPI clock
        CS_N     : in  std_logic; -- SPI chip select, active in low
        MOSI     : in  std_logic; -- SPI serial data from master to slave
        -- LOGIC INTERFACE
        DOUT     : out std_logic_vector(WORD_SIZE-1 downto 0); -- output data from SPI master
        DOUT_VLD : out std_logic;  -- when DOUT_VLD = 1, output data are valid 
		IDLE	 : out std_logic   -- when IDLE = 1 SPI bus is in idle
    );
end SPI;

architecture SPI_SLAVE_ARCH of SPI is

	signal SCLK_sample_n      : std_logic;	 -- Sample of SCLK at time n
    signal SCLK_sample_n_1    : std_logic;	 -- Sample of SCLK at time n-1
    signal SCLK_falling       : std_logic;   -- 
    signal SCLK_rising   	  : std_logic;
    signal bit_cnt            : natural range 0 to WORD_SIZE-1;
    signal bit_cnt_max        : std_logic;
    signal last_bit_en        : std_logic;
    signal rx_data            : std_logic_vector(WORD_SIZE-1 downto 0);
    signal rx_data_vld        : std_logic; 
	signal idle_flag		  : std_logic;

begin

    -- -------------------------------------------------------------------------
    --  SPI CLOCK EDGE DETECTION
    -- -------------------------------------------------------------------------
    SCLK_EDGE_DETECTION_P : process (CLK)
    begin
        if (rising_edge(CLK)) then
            if (RST = '1') then
                SCLK_sample_n <= '0';  
				SCLK_sample_n_1 <= '0';
            else   
                SCLK_sample_n <= SCLK; 
				SCLK_sample_n_1 <= SCLK_sample_n;
            end if;
        end if;
    end process;

    SCLK_rising <= not SCLK_sample_n_1 and SCLK_sample_n;
	SCLK_falling <= SCLK_sample_n_1 and not SCLK_sample_n;
	
	-- -------------------------------------------------------------------------
    --  DATA SHIFT REGISTER
    -- -------------------------------------------------------------------------
    -- The shift register capture and store incoming data from master.
    SHIFT_REG_P : process (CLK)
    begin
        if (rising_edge(CLK)) then
            if (SCLK_rising = '1' and CS_N = '0') then
                rx_data <= rx_data(WORD_SIZE-2 downto 0) & MOSI;
            end if;
        end if;
    end process;
	
    -- -------------------------------------------------------------------------
    --  RECEIVED BITS COUNTER
    -- -------------------------------------------------------------------------
    -- The counter counts received bits from the master. Counter is enabled when
    -- rising edge of SPI clock is detected and not asserted CS_N.
    BIT_COUNT_P : process (CLK)
    begin
        if (rising_edge(CLK)) then
            if (RST = '1') then
                bit_cnt <= 0;
            elsif (SCLK_falling = '1' and CS_N = '0') then
                if (bit_cnt_max = '1') then
                    bit_cnt <= 0;
                else
                    bit_cnt <= bit_cnt + 1;
                end if;
            end if;
        end if;
    end process;

    -- The flag of maximal value of the bit counter.
    bit_cnt_max <= '1' when (bit_cnt = WORD_SIZE-1) else '0';

    -- -------------------------------------------------------------------------
    --  LAST BIT FLAG REGISTER
    -- -------------------------------------------------------------------------

    -- The flag of last bit of received byte is only registered the flag of
    -- maximal value of the bit counter.
    last_bit_en_p : process (CLK)
    begin
        if (rising_edge(CLK)) then
            if (RST = '1') then
                last_bit_en <= '0';
            else
                last_bit_en <= bit_cnt_max;
            end if;
        end if;
    end process;
	
	idle_flag_p : process (CLK)	
	begin
		 if (rising_edge(CLK)) then
            --if (SCLK_rising or SCLK_falling) then
				   idle_flag <= CS_n;
            --end if;
        end if;
	end process;
    -- -------------------------------------------------------------------------
    --  RECEIVED DATA VALID FLAG
    -- -------------------------------------------------------------------------

    -- Received data from master are valid when falling edge of SPI clock is
    -- detected and the last bit of received byte is detected.
    rx_data_vld <= SCLK_falling and last_bit_en;

    -- -------------------------------------------------------------------------
    --  ASSIGNING OUTPUT SIGNALS
    -- -------------------------------------------------------------------------
	IDLE <= idle_flag;
    DOUT     <= rx_data;
    DOUT_VLD <= rx_data_vld;

end SPI_SLAVE_ARCH;