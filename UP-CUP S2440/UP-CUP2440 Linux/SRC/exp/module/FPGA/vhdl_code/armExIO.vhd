---------------------------------------------------------------------------------------------------
--
-- Title       : armExIO for UP-NET3000
-- Design      : ExIO
-- Author      : 0
-- Company     : 0
--
---------------------------------------------------------------------------------------------------
--
-- File        : armExIO.vhd
-- Generated   : Fri Dec 21 14:56:41 2003
-- From        : interface description file
-- By          : Itf2Vhdl ver. 1.20
--
---------------------------------------------------------------------------------------------------
--
-- Description : 
--
---------------------------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity armExIO is
	 port(
		 reset,mclk,wr,rd,cs:in std_logic;
		 led:out std_logic_vector(2 downto 0);
		 switch:in std_logic_vector(3 downto 0);
		 unused:in std_logic_vector(5 downto 0);
		 address:in std_logic_vector(9 downto 0);
		 data:inout std_logic_vector(15 downto 0);
		 IOP3:inout std_logic_vector(25 downto 0);
		 IOP1,IOP2:inout std_logic_vector(36 downto 0)
	);
	--pin define
	attribute altera_chip_pin_lc:string;
	attribute altera_chip_pin_lc of wr:signal is"@78";
	attribute altera_chip_pin_lc of rd:signal is"@184";
	attribute altera_chip_pin_lc of cs:signal is"@125";
	attribute altera_chip_pin_lc of mclk:signal is"@183";
	attribute altera_chip_pin_lc of reset:signal is"@80";
	attribute altera_chip_pin_lc of led:signal is"@134,@135,@136";
	attribute altera_chip_pin_lc of switch:signal is"@139,@140,@141,@142";
	attribute altera_chip_pin_lc of unused:signal is"@126,@127,@128,@131,@132,@133";
	attribute altera_chip_pin_lc of address:signal is"@180,@186,@187,@189,@190,@191,@192,@193,@195,@196";
	attribute altera_chip_pin_lc of data:signal is"@122,@121,@120,@119,@116,@115,@114,@113,@112,@111,@104,@103,@102,@101,@100,@99";
	attribute altera_chip_pin_lc of IOP3:signal is"@143,@144,@147,@148,@149,@150,@157,@158,@159,@160,@161,@162,@163,@164,@166,@167,@168,@169,@170,@172,@173,@174,@175,@176,@177,@179";
	attribute altera_chip_pin_lc of IOP1:signal is"@97, @96, @95, @94, @93, @92, @90, @89, @88, @87, @86, @85, @83, @75, @74, @73, @71, @70, @69, @68, @67, @65, @64, @63, @61, @60, @58, @57, @56, @55, @54, @53,@47, @46, @45, @44, @41";
	attribute altera_chip_pin_lc of IOP2:signal is"@40, @39, @38, @37, @36, @31, @30, @29, @28, @27, @26, @25, @24, @19, @18, @17, @16, @15, @14, @13, @12, @11, @10,  @9,  @8,  @7,@208,@207,@206,@205,@204,@203,@202,@200,@199,@198,@197";
end armExIO;

architecture armExIO of armExIO is
	signal cnt4:std_logic_vector(7 downto 0);
begin
	led_proc:process(mclk)
	variable cnt1:integer range 0 to 3 :=0;
	variable cnt2:std_logic_vector(7 downto 0);
	variable cnt3:integer range 0 to 1000000 :=0;
	variable aaaa:std_logic_vector(2 downto 0) :="000";
	begin
		if(mclk'event and mclk='0')then
			if(cnt3>=1000000)then							--0.1s @10MHz
				cnt3:=0;
				cnt2:=cnt2+1;
				if(cnt2>=cnt4)then
					cnt2:="00000000";
					cnt1:=cnt1+1;
					if(cnt1=1)then
						aaaa:="110";
					elsif(cnt1=2)then
						aaaa:="101";
					elsif(cnt1=3)then
						cnt1:=0;
						aaaa:="011";
					end if;
					led<=aaaa;
		   		end if;
			else
			cnt3 :=cnt3+1;
			end if;
		end if;
	end process led_proc;

	write:process(wr,cs)
	begin
		if(wr='0' and cs='0')then
			case address is
				when"0000000000"=>							--write IOP1_L
					IOP1(15 downto 0)<=data;
				when"0000000001"=>							--write IOP1_H
					IOP1(31 downto 16)<=data;
				when"0000000010"=>							--write IOP2_L
					IOP2(15 downto 0)<=data;
				when"0000000011"=>							--write IOP2_H
					IOP2(31 downto 16)<=data;
				when"0000000100"=>							--write IOP3_L
					IOP3(15 downto 0)<=data;
				when"0000000101"=>							--write IOP3_H
					IOP3(25 downto 16)<=data(9 downto 0);
				when"0000000110"=>							--write LED_CONTROL 
					cnt4<=data(7 downto 0);
				when"0000011111"=>							--reset port
					IOP1<=(others=>'0');
					IOP2<=(others=>'0');
					IOP3<=(others=>'0');
				when others=> 
			end case;
		end if;
	end process write;

	read:process(rd,cs)
	begin
		if(cs='1' or rd='1')then
			data<="ZZZZZZZZZZZZZZZZ";
		elsif(rd='0' and cs='0')then
			case address is
				when"0000000000"=>							--read IOP1_L
					data<=IOP1(15 downto 0);
				when"0000000001"=>							--read IOP1_H
					data<=IOP1(31 downto 16);
				when"0000000010"=>							--read IOP2_L
					data<=IOP2(15 downto 0);
				when"0000000011"=>							--read IOP2_H
					data<=IOP2(31 downto 16);
				when"0000000100"=>							--read IOP3_L
					data<=IOP3(15 downto 0);
				when"0000000101"=>							--read IOP3_H
					data<="000000"&IOP3(25 downto 16);
				when"0000000110"=>							--read LED_CONTROL
					data<="00000000"&cnt4;
				when others=>
					data<="ZZZZZZZZZZZZZZZZ";
			end case;
		end if;
	end process read;
end armExIO;
