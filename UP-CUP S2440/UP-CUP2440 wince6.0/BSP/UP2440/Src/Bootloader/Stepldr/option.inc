;===========================================
; NAME: OPTION.A
; DESC: Configuration options for .S files
; HISTORY:
; 02.28.2002: ver 0.0
;===========================================

;Start address of each stacks,
_STACK_BASEADDRESS	EQU 0x33ff8000  
_MMUTT_STARTADDRESS	EQU 0x33ff8000  
_ISR_STARTADDRESS	EQU 0x33ffff00

	GBLL 	PLL_ON_START  
PLL_ON_START	SETL 	{TRUE}


	GBLL	ENDIAN_CHANGE
ENDIAN_CHANGE	SETL	{FALSE}

	GBLA	ENTRY_BUS_WIDTH
ENTRY_BUS_WIDTH	SETA	16	


;BUSWIDTH = 16,32
	GBLA    BUSWIDTH	;max. bus width for the GPIO configuration
BUSWIDTH	SETA    32


	GBLA	FCLK
;FCLK		SETA	296350000
FCLK		SETA	400000000
    

    [	FCLK = 300000000	
M_MDIV	EQU	0x44	;Fin=12.0MHz Fout=300.0MHz
M_PDIV	EQU	0x1
M_SDIV	EQU	0x1

U_MDIV EQU  0x38  ;USB = 48Mhz
U_PDIV EQU  0x2
U_SDIV EQU  0x
    ]

    [	FCLK = 296350000	
M_MDIV	EQU	0x61	;Fin=16.000MHz
M_PDIV	EQU	0x1
M_SDIV	EQU	0x2

U_MDIV EQU  0x3C  ;USB = 48Mhz
U_PDIV EQU  0x4
U_SDIV EQU  0x2
	]

    [	FCLK = 399650000	
M_MDIV	EQU	0x6e	;Fin=16.00000MHz
M_PDIV	EQU	0x3
M_SDIV	EQU	0x1

U_MDIV EQU  0x3C  ;USB = 48Mhz
U_PDIV EQU  0x4
U_SDIV EQU  0x2
    ]

    [	FCLK = 400000000	
M_MDIV  EQU	0x5C	;Fin=12.00000MHz
M_PDIV	EQU	0x1
M_SDIV	EQU	0x1

U_MDIV  EQU	0x38	;USB = 48Mhz
U_PDIV  EQU	0x2
U_SDIV  EQU	0x2
    ]

	END
