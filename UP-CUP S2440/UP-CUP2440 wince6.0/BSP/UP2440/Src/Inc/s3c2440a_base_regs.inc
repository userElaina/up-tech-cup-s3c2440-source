;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;;
;;  Header: s3c2440a_base_regs.inc
;;
;;  This header file defines the Physical Addresses (PA) of 
;;  the base registers for the System on Chip (SoC) components.
;;

;;
;;  NAMING CONVENTIONS
;;
;;  CPU_BASE_REG_ is the standard prefix for CPU base registers.
;;
;;  Memory ranges are accessed using physical, uncached, or cached addresses,
;;  depending on the system state. The following abbreviations are used for
;;  each addressing type:
;;
;;      PA - physical address
;;      UA - uncached virtual address
;;      CA - cached virtual address
;;
;;  The naming convention for CPU base registers is:
;;
;;      CPU_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
;;
;;

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_MEMCTRL
;;
;;  Locates the memory controller register block.
;;

S3C2440A_BASE_REG_PA_MEMCTRL    EQU            (0x48000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_USB
;;
;;  Locates the USB controller register block.
;;
S3C2440A_BASE_REG_PA_USB    EQU                (0x49000000)

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_INTR
;;
;;  Locates the interrupt (INTR) controller register block.
;;
S3C2440A_BASE_REG_PA_INTR    EQU               (0x4A000000)

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_DMA
;;
;;  Locates the DMA controller register block.
;;
S3C2440A_BASE_REG_PA_DMA    EQU                (0x4B000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_CLOCK_POWER
;;
;;  Locates the clock and power register block
;;
S3C2440A_BASE_REG_PA_CLOCK_POWER    EQU        (0x4C000000)

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_LCD
;;
;;  Locates the LCD controller register block.
;;
S3C2440A_BASE_REG_PA_LCD    EQU                (0x4D000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_NAND
;;
;;  Locates the NAND controller register block.
;;

S3C2440A_BASE_REG_PA_NAND    EQU               (0x4E000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_UART0/1/2
;;
;;  Locates the UART register blocks.
;;
S3C2440A_BASE_REG_PA_UART0    EQU              (0x50000000)            
S3C2440A_BASE_REG_PA_UART1    EQU              (0x50004000)            
S3C2440A_BASE_REG_PA_UART2    EQU              (0x50008000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_PWM
;;
;;  Locates the PWM timer controller register block.
;;

S3C2440A_BASE_REG_PA_PWM    EQU                (0x51000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_USBD
;;
;;  Locates the USB device controller register block.
;;

S3C2440A_BASE_REG_PA_USBD    EQU               (0x52000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_WATCHDOG
;;
;;  Locates the watchdog timer register block.
;;

S3C2440A_BASE_REG_PA_WATCHDOG    EQU           (0x53000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_IICBUS
;;
;;  Locates the IIC Bus register block.
;;
S3C2440A_BASE_REG_PA_IICBUS    EQU             (0x54000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_IISBUS
;;
;;  Locates the IIS Bus register block.
;;
S3C2440A_BASE_REG_PA_IISBUS    EQU             (0x55000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_IOPORT
;;
;;  Locates the IO port controller register block.
;;
S3C2440A_BASE_REG_PA_IOPORT    EQU             (0x56000000)            

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_RTC
;;
;;  Locates the real time clock (RTC) register block.
;;
S3C2440A_BASE_REG_PA_RTC    EQU                (0x57000000)

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_ADC
;;
;;  Locates the A/D converter register block.
;;
S3C2440A_BASE_REG_PA_ADC    EQU                (0x58000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_SPI
;;
;;  Locates the SPI register block.
;;
S3C2440A_BASE_REG_PA_SPI    EQU                (0x59000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_SDI
;;
;;  Locates the SDI register block.
;;
S3C2440A_BASE_REG_PA_SDI    EQU                (0x5A000000)           

;;
;;
;;  Define:  S3C2440A_BASE_REG_PA_CAM
;;
;;  Locates the SDI register block.
;;
S3C2440A_BASE_REG_PA_CAM    EQU                (0x4F000000)           


    END
