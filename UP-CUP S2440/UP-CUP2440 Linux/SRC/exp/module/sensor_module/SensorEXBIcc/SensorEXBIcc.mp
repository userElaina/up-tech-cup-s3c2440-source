NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                        func_lit   0026   0022 =     34. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0026  __func_lit_start
       0048  __func_lit_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                             lit   0048   006A =    106. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0048  __lit_start
       00B2  __lit_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   00B2   000D =     13. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       00B2  __idata_start
       00BF  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   00C0   17D4 =   6100. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0060  __start
       0060  __text_start
       007F  _exit
       0080  _init_1820
       00A5  _write_1820
       00C5  _read_1820
       00EB  _Get_Temperature
       0127  _ADC_Init
       0130  _StartADC
       017C  _GetSensorMessage
       059B  _port_init
       05A7  _main
       05D2  _SPI_MasterInit
       05D7  _SPI_MasterTransmitaByte
       05DB  _DisplayData
       0775  _delay_50us
       0787  _delay_ms
       0794  _delay_50ms
       07A6  _delay_s
       07B3  _timer0_init
       07C2  _timer0_ovf_isr
       07D8  _StartCountHole
       07DA  _FinishCountHole
       07DD  _EndCountHole
       07EB  _timer1_init
       07FA  _GetHS1101
       08FF  _USART_Init
       090A  _UARTSendaByte
       090E  _UARTSendAMessage
       0943  _UARTRcvaByte
       0947  _WaitForCMD
       09CF  push_arg4
       09D1  push_arg2
       09D4  mod8u
       09D6  div8u
       09D7  xdiv8u
       09EC  empy16s
       09FC  pop_gset2
       09FE  pop_gset3
       0A00  pop_gset4
       0A02  pop_gset5
       0A04  push_gset5
       0A06  push_gset4
       0A08  push_gset3
       0A0A  push_gset2
       0A0C  push_gset1
       0A0F  pop_gset1
       0A10  pop
       0A23  neg32
       0A2C  lpm32
       0A40  lsl8
       0A46  fp2int
       0A4D  nojump1
       0A51  fp2long
       0A56  nojump2
       0A57  FTOI
       0A59  FTOL
       0A84  int2fp
       0A8A  long2fp
       0A8D  ITOF
       0A9F  itof2
       0AA1  itof3
       0AAF  save_floatregs
       0ABA  restore_floatregs
       0AC5  stk2arg1
       0ACA  stk2arg0
       0ACF  float_epilog
       0AD6  float_prolog
       0AD9  add32fs
       0ADB  add32f
       0ADE  sub32fs
       0AE0  sub32f
       0AE3  fret2
       0AE9  FSUB
       0AEA  FADD
       0B29  usub24
       0B2D  div32fs
       0B2F  div32f
       0B4E  fdiv10
       0B53  FDIV
       0B85  cmp32f
       0B88  FLTCP
       0BB4  empy32fs
       0BB6  empy32f
       0BBD  eFMUL
       0BD3  eumul24
       0BFC  savfacc1
       0BFF  restorefacc1
       0C02  unpackfacc1
       0C0B  negmant
       0C11  maxres
       0C18  minres
       0C1E  ac1toac2
       0C24  swapacc
       0C30  unpack
       0C42  repack
       0C4A  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0060   000D =     13. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0060  __data_start
       0060  _CMD
       0061  _CharacterTable
       006B  _HoleCountH
       006D  __data_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                             bss   006D   000B =     11. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       006D  _count
       006D  __bss_start
       0071  _SendBuffer
       0078  __bss_end

Area                               Start  End    Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0000   0014 =     20. bytes (abs,ovr,rom)

Files Linked      [ module(s) ]

C:\PROGRA~1\icc\lib\crtAVR.o	[ crtavr.s ]
18b20.o	[ _18b20.c ]
ADC.o	[ ADC.c ]
Sensor.o	[ Sensor.c ]
SensorEXB.o	[ SensorEXB.c ]
SPI.o	[ SPI.c ]
TimeDelay.o	[ TimeDelay.c ]
Timer.o	[ Timer.c ]
UART.o	[ UART.c ]
<library>	[ asave.s, div8u.s, emul16s.s, gpop2.s, gpop3.s, gpop4.s, gpop5.s, gpush.s, lneg.s, lpm32.s, lsl8.s, fp2int.s ]
[ fpint2fp.s, fputil.s, fpadd.s, fpdiv.s, fpcmp.s, fpemul.s, fputil2.s, fputil3.s ]

User Global Definitions

ram_end = 0x45f
hwstk_size = 0x10

User Base Address Definitions

func_lit = 0x26
data = 0x60
eeprom:1.512

