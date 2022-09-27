//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Header: s3c2440a_usbd.h
//
//  Defines the USB device controller CPU register layout and definitions.
//
#ifndef __S3C2440A_USBD_H
#define __S3C2440A_USBD_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C2440A_USBD_REG    
//
//  Defines the USB device control register block. This register bank is
//  located by the constant S3C2440A_BASE_REG_PA_USBD in configuration file 
//  s3c2440_base_reg_cfg.h.
//

struct FUNC_ADDR_REG   // Function Address Register
{
    UINT8 func_addr       :7;    // function_address
    UINT8 addr_up         :1;    // addr_update
};

struct PWR_REG         // Power Management Register
{
    UINT8 sus_en          :1;    // suspend_en
    UINT8 sus_mo          :1;    // suspend_mode
    UINT8 mcu_res         :1;    // mcu_resume
    UINT8 usb_re          :1;    // usb_reset
    UINT8 rsvd1           :3;     
    UINT8 iso_up          :1;    // iso_update
};

struct EP_INT_REG      // Endpoint Interrupt register
{
    UINT8 ep0_int         :1;    // ep0_interrupt
    UINT8 ep1_int         :1;    // ep1_interrupt
    UINT8 ep2_int         :1;    // ep2_interrupt
    UINT8 ep3_int         :1;    // ep3_interrupt
    UINT8 ep4_int         :1;    // ep4_interrupt
    UINT8 rsvd0           :3;
};

struct USB_INT_REG     // USB Interrupt Register
{
    UINT8 sus_int         :1;    // suspend inaterrupt
    UINT8 resume_int      :1;    // resume interrupt
    UINT8 reset_int       :1;    // reset interrupt
    UINT8 rsvd0           :5;
};

struct EP_INT_EN_REG   // Endpoint Interrupt Mask Register
{
    UINT8 ep0_int_en      :1;    // ep1_int_reg
    UINT8 ep1_int_en      :1;    // ep1_int_reg
    UINT8 ep2_int_en      :1;    // ep2_int_reg
    UINT8 ep3_int_en      :1;    // ep3_int_reg
    UINT8 ep4_int_en      :1;    // ep4_int_reg
    UINT8 rsvd0           :3;
};

struct USB_INT_EN_REG  // USB Interrupt Mask Register
{
    UINT8 sus_int_en      :1;    // suspend_int_en
    UINT8 rsvd1           :1;     
    UINT8 reset_int_en    :1;    // reset_enable_reg
    UINT8 rsvd0           :5;
};

struct FRAME_NUM1_REG  // Frame Number 1 Register
{
    UINT8 fr_n1           :8;    // frame_num1_reg
};

struct FRAME_NUM2_REG  // Frame Number 2 Register
{
    UINT8 fr_n2           :8;    // frame_num2_reg
};

struct INDEX_REG       // Index Register
{
    UINT8 index           :8;    // index_reg
};

// TODO - clean this up.

struct EP0ICSR1Bits             // EP0 & ICSR1 shared
{
    UINT8 opr_ipr       :1;
    UINT8 ipr_       	:1;
    UINT8 sts_     		:1;
    UINT8 de_ff      	:1;
    UINT8 se_sds     	:1;
    UINT8 sds_sts       :1;
    UINT8 sopr_cdt      :1;
    UINT8 sse_       	:1;
};

struct ICSR2Bits
{              // in csr2 areg
    UINT8 rsvd1      	:4;
    UINT8 in_dma_int_en :1;     // in_dma_int_en
    UINT8 mode_in       :1;     // mode_in
    UINT8 iso        	:1;         // iso/bulk mode
    UINT8 auto_set      :1;     // auto_set
};

struct OCSR1Bits
{              // out csr1 reg
    UINT8 out_pkt_rdy   :1;     // out packet reday
    UINT8 rsvd0      	:3;
    UINT8 fifo_flush 	:1;         // fifo_flush
    UINT8 send_stall 	:1;         // send_stall
    UINT8 sent_stall 	:1;             // sent_stall
    UINT8 clr_data_tog  :1;     // clear data toggle
};

struct OCSR2Bits
{              // out csr2 reg
    UINT8 rsvd0      		:5;
    UINT8 out_dma_int_en 	:1;     // out_dma_int_en
    UINT8 iso        		:1;     // iso/bulk mode
    UINT8 auto_clr       	:1;     // auto_clr
};

struct EP0FBits
{               // ep0 fifo reg
    UINT8 fifo_data      :8;     // fifo data
};

struct EP1FBits
{               // ep0 fifo reg
    UINT8 fifo_data      :8;     // fifo data
};

struct EP2FBits
{               // ep0 fifo reg
    UINT8 fifo_data      :8;     // fifo data
};

struct EP3FBits
{               // ep0 fifo reg
    UINT8 fifo_data      :8;     // fifo data
};

struct EP4FBits
{               // ep0 fifo reg
    UINT8 fifo_data      :8;     // fifo data
};

struct MAXPBits
{
    UINT8 maxp       :4;     // max packet reg
    UINT8 rsvd0      :4;
};

struct OFCR1Bits
{              // out_fifo_cnt1_reg
    UINT8 out_cnt_low    :8;     // out_cnt_low
};

struct OFCR2Bits
{              // out_fifo_cnt2_reg
    UINT8 out_cnt_high   :8;     // out_cnt_high
};

struct EP1DCBits
{              // ep1 dma interface control
    UINT8 dma_mo_en      :1;     // dma_mode_en
    UINT8 in_dma_run     :1;     // in_dma_run
    UINT8 orb_odr        :1;     // out_run_ob/out_dma_run
    UINT8 demand_mo      :1;     // demand_mode
    UINT8 state      	 :3;     // state
    UINT8 in_run_ob      :1;     // in_run_ob
};

struct EP2DCBits
{              // ep2 dma interface control
    UINT8 dma_mo_en      :1;     // dma_mode_en
    UINT8 in_dma_run     :1;     // in_dma_run
    UINT8 orb_odr        :1;     // out_run_ob/out_dma_run
    UINT8 demand_mo      :1;     // demand_mode
    UINT8 state      	 :3;     // state
    UINT8 in_run_ob      :1;     // in_run_ob
};

struct EP3DCBits
{              // ep3 dma interface control
    UINT8 dma_mo_en      :1;     // dma_mode_en
    UINT8 in_dma_run     :1;     // in_dma_run
    UINT8 orb_odr        :1;     // out_run_ob/out_dma_run
    UINT8 demand_mo      :1;     // demand_mode
    UINT8 state      	 :3;     // state
    UINT8 in_run_ob      :1;     // in_run_ob
};

struct EP4DCBits
{              // ep4 dma interface control
    UINT8 dma_mo_en      :1;     // dma_mode_en
    UINT8 in_dma_run     :1;     // in_dma_run
    UINT8 orb_odr        :1;     // out_run_ob/out_dma_run
    UINT8 demand_mo      :1;     // demand_mode
    UINT8 state      	 :3;     // state
    UINT8 in_run_ob      :1;     // in_run_ob
};

struct EP1DUBits
{
    UINT8 ep1_unit_cnt   :8;     // ep0_unit_cnt
};

struct EP2DUBits
{
    UINT8 ep2_unit_cnt   :8;     // ep0_unit_cnt
};

struct EP3DUBits
{
    UINT8 ep3_unit_cnt   :8;     // ep0_unit_cnt
};

struct EP4DUBits
{
    UINT8 ep4_unit_cnt   :8;     // ep0_unit_cnt
};

struct EP1DFBits
{
    UINT8 ep1_fifo_cnt   :8;
};

struct EP2DFBits
{
    UINT8 ep2_fifo_cnt   :8;
};

struct EP3DFBits
{
    UINT8 ep3_fifo_cnt   :8;
};

struct EP4DFBits
{
    UINT8 ep4_fifo_cnt   :8;
};

struct EP1DTLBits
{
    UINT8 ep1_ttl_l      :8;
};

struct EP1DTMBits
{
    UINT8 ep1_ttl_m      :8;
};

struct EP1DTHBits
{
    UINT8 ep1_ttl_h      :8;
};

struct EP2DTLBits
{
    UINT8 ep2_ttl_l      :8;
};

struct EP2DTMBits
{
    UINT8 ep2_ttl_m      :8;
};

struct EP2DTHBits
{
    UINT8 ep2_ttl_h      :8;
};

struct EP3DTLBits
{
    UINT8 ep3_ttl_l      :8;
};

struct EP3DTMBits
{
    UINT8 ep3_ttl_m      :8;
};

struct EP3DTHBits
{
    UINT8 ep3_ttl_h      :8;
};

struct EP4DTLBits
{
    UINT8 ep4_ttl_l      :8;
};

struct EP4DTMBits
{
    UINT8 ep4_ttl_m      :8;
};

struct EP4DTHBits
{
    UINT8 ep4_ttl_h      :8;
};

typedef struct
{                     // PHY BASE : 0x52000140(Little Endian)
    struct FUNC_ADDR_REG   udcFAR;         // 0x140
    UINT8            rsvd0;              // 0x141
    UINT8            rsvd1;              // 0x142
    UINT8            rsvd2;              // 0x143
    struct PWR_REG      PMR;            // 0x144
    UINT8            rsvd3;              // 0x145
    UINT8            rsvd4;              // 0x146
    UINT8            rsvd5;              // 0x147
    struct EP_INT_REG      EIR;                // 0x148
    UINT8            rsvd6;              // 0x149
    UINT8            rsvd7;              // 0x14a
    UINT8            rsvd8;              // 0x14b
    UINT8            rsvd9;              // 0x14C
    UINT8            rsvd10;             // 0x14d
    UINT8            rsvd11;             // 0x14e
    UINT8            rsvd12;             // 0x14f

    UINT8            rsvd13;             // 0x150
    UINT8            rsvd14;             // 0x151
    UINT8            rsvd15;             // 0x152
    UINT8            rsvd16;             // 0x153
    UINT8            rsvd17;             // 0x154
    UINT8            rsvd18;             // 0x155
    UINT8            rsvd19;             // 0x156
    UINT8            rsvd20;             // 0x157
    struct USB_INT_REG      UIR;                // 0x158
    UINT8            rsvd21;             // 0x159
    UINT8            rsvd22;             // 0x15a
    UINT8            rsvd23;             // 0x15b
    struct EP_INT_EN_REG     EIER;           // 0x15C
    UINT8            rsvd24;             // 0x15d
    UINT8            rsvd25;             // 0x15e
    UINT8            rsvd26;             // 0x15f

    UINT8            rsvd27;             // 0x160
    UINT8            rsvd28;             // 0x161
    UINT8            rsvd29;             // 0x162
    UINT8            rsvd30;             // 0x163
    UINT8            rsvd31;             // 0x164
    UINT8            rsvd32;             // 0x165
    UINT8            rsvd33;             // 0x166
    UINT8            rsvd34;             // 0x167
    UINT8            rsvd35;             // 0x168
    UINT8            rsvd36;             // 0x169
    UINT8            rsvd37;             // 0x16a
    UINT8            rsvd38;         // 0x16b
    struct USB_INT_EN_REG     UIER;               // 0x16c
    UINT8            rsvd39;             // 0x16d
    UINT8            rsvd40;             // 0x16e
    UINT8            rsvd41;             // 0x16f

    struct FRAME_NUM1_REG     FNR1;           // 0x170
    UINT8            rsvd42;             // 0x171
    UINT8            rsvd43;             // 0x172
    UINT8            rsvd44;             // 0x173
    struct FRAME_NUM2_REG     FNR2;           // 0x174
    UINT8            rsvd45;             // 0x175
    UINT8            rsvd46;             // 0x176
    UINT8            rsvd47;             // 0x177
    struct INDEX_REG    INDEX;              // 0x178
    UINT8            rsvd48;         // 0x179
    UINT8            rsvd49;             // 0x17a
    UINT8            rsvd50;             // 0x17b
    UINT8            rsvd51;             // 0x17C
    UINT8            rsvd52;             // 0x17d
    UINT8            rsvd53;             // 0x17e
    UINT8            rsvd54;             // 0x17f

    struct MAXPBits     MAXP;           // 0x180
    UINT8            rsvd56;             // 0x181
    UINT8            rsvd57;             // 0x182
    UINT8            rsvd58;             // 0x183
    struct EP0ICSR1Bits EP0ICSR1;       // 0x184
    // struct ICSR1Bits	ICSR1;			// 0x184
    // struct EP0CSRBits	EP0CSR;			// mapped to the in_csr1 reg 
    UINT8            rsvd59;             // 0x185
    UINT8            rsvd60;             // 0x186
    UINT8            rsvd61;             // 0x187
    struct ICSR2Bits    ICSR2;          // 0x188
    UINT8            rsvd63;             // 0x189
    UINT8            rsvd64;             // 0x18a
    UINT8            rsvd65;             // 0x18b
    UINT8            rsvd55;             // 0x180
    UINT8            rsvd66;             // 0x18d
    UINT8            rsvd67;             // 0x18e
    UINT8            rsvd68;             // 0x18f

    struct OCSR1Bits    OCSR1;          // 0x190
    UINT8            rsvd69;             // 0x191
    UINT8            rsvd70;             // 0x192
    UINT8            rsvd71;             // 0x193
    struct OCSR2Bits    OCSR2;          // 0x194
    UINT8            rsvd73;             // 0x195
    UINT8            rsvd74;             // 0x196
    UINT8            rsvd75;             // 0x197
    struct OFCR1Bits    OFCR1;          // 0x198
    UINT8            rsvd76;             // 0x199
    UINT8            rsvd77;             // 0x19a
    UINT8            rsvd78;             // 0x19b
    struct OFCR2Bits    OFCR2;          // 0x19C
    UINT8            rsvd79;             // 0x19d
    UINT8            rsvd80;             // 0x19e
    UINT8            rsvd81;             // 0x19f

    UINT8            rsvd82;             // 0x1A0	
    UINT8            rsvd83;             // 0x1a1
    UINT8            rsvd84;             // 0x1a2
    UINT8            rsvd85;             // 0x1a3
    UINT8            rsvd86;             // 0x1A4
    UINT8            rsvd87;             // 0x1a5
    UINT8            rsvd88;             // 0x1a6
    UINT8            rsvd89;             // 0x1a7
    UINT8            rsvd90;             // 0x1A8
    UINT8            rsvd91;             // 0x1a9
    UINT8            rsvd92;             // 0x1aa
    UINT8            rsvd93;             // 0x1ab
    UINT8            rsvd94;             // 0x1AC
    UINT8            rsvd95;             // 0x1ad
    UINT8            rsvd96;             // 0x1ae
    UINT8            rsvd97;             // 0x1af

    UINT8            rsvd98;             // 0x1B0
    UINT8            rsvd99;             // 0x1B1
    UINT8            rsvd100;                // 0x1B2
    UINT8            rsvd101;                // 0x1B3
    UINT8            rsvd102;                // 0x1B4
    UINT8            rsvd103;                // 0x1B5
    UINT8            rsvd104;                // 0x1B6
    UINT8            rsvd105;                // 0x1B7
    UINT8            rsvd106;                // 0x1B8
    UINT8            rsvd107;                // 0x1B9
    UINT8            rsvd108;                // 0x1Ba
    UINT8            rsvd109;                // 0x1Bb
    UINT8            rsvd110;                // 0x1BC
    UINT8            rsvd111;                // 0x1Bd
    UINT8            rsvd112;                // 0x1Be
    UINT8            rsvd113;                // 0x1Bf

    struct EP0FBits     EP0F;               // 0x1C0
    UINT8            rsvd114;                // 0x1c1
    UINT8            rsvd115;                // 0x1c2
    UINT8            rsvd116;                // 0x1c3
    struct EP1FBits     EP1F;               // 0x1C4
    UINT8            rsvd117;                // 0x1c5
    UINT8            rsvd118;                // 0x1c6
    UINT8            rsvd119;                // 0x1c7
    struct EP2FBits     EP2F;               // 0x1C8
    UINT8            rsvd120;                // 0x1c9
    UINT8            rsvd121;                // 0x1ca
    UINT8            rsvd122;                // 0x1cb
    struct EP3FBits     EP3F;               // 0x1CC
    UINT8            rsvd123;                // 0x1cd
    UINT8            rsvd124;                // 0x1ce
    UINT8            rsvd125;                // 0x1cf

    struct EP4FBits     EP4F;               // 0x1D0
    UINT8            rsvd126;                // 0x1d1
    UINT8            rsvd127;                // 0x1d2
    UINT8            rsvd128;                // 0x1d3
    UINT8            rsvd169;                // 0x1D4
    UINT8            rsvd170;                // 0x1d5
    UINT8            rsvd171;                // 0x1d6
    UINT8            rsvd172;                // 0x1d7
    UINT8            rsvd173;                // 0x1D8
    UINT8            rsvd174;                // 0x1d9
    UINT8            rsvd175;                // 0x1da
    UINT8            rsvd176;                // 0x1db
    UINT8            rsvd177;                // 0x1DC
    UINT8            rsvd178;                // 0x1dd
    UINT8            rsvd179;                // 0x1de
    UINT8            rsvd180;                // 0x1df

    UINT8            rsvd1810;                // 0x1E0
    UINT8            rsvd1820;                // 0x1E1
    UINT8            rsvd1830;                // 0x1E2
    UINT8            rsvd1840;                // 0x1E3
    UINT8            rsvd1850;                // 0x1E4
    UINT8            rsvd1860;                // 0x1E5
    UINT8            rsvd1870;                // 0x1E6
    UINT8            rsvd1880;                // 0x1E7
    UINT8            rsvd1890;                // 0x1E8
    UINT8            rsvd1900;                // 0x1E9
    UINT8            rsvd1910;                // 0x1Ea
    UINT8            rsvd1920;                // 0x1Eb
    UINT8            rsvd1930;                // 0x1EC
    UINT8            rsvd1940;                // 0x1Ed
    UINT8            rsvd1950;                // 0x1Ee
    UINT8            rsvd1960;                // 0x1Ef

    UINT8            rsvd181;                // 0x1F0
    UINT8            rsvd182;                // 0x1F1
    UINT8            rsvd183;                // 0x1F2
    UINT8            rsvd184;                // 0x1F3
    UINT8            rsvd185;                // 0x1F4
    UINT8            rsvd186;                // 0x1F5
    UINT8            rsvd187;                // 0x1F6
    UINT8            rsvd188;                // 0x1F7
    UINT8            rsvd189;                // 0x1F8
    UINT8            rsvd190;                // 0x1F9
    UINT8            rsvd191;                // 0x1Fa
    UINT8            rsvd192;                // 0x1Fb
    UINT8            rsvd193;                // 0x1FC
    UINT8            rsvd194;                // 0x1Fd
    UINT8            rsvd195;                // 0x1Fe
    UINT8            rsvd196;                // 0x1Ff

    struct EP1DCBits    EP1DC;              // 0x200
    UINT8            rsvd197;                // 0x201
    UINT8            rsvd198;                // 0x202
    UINT8            rsvd199;                // 0x203
    struct EP1DUBits    EP1DU;              // 0x204
    UINT8            rsvd200;                // 0x205
    UINT8            rsvd201;                // 0x206
    UINT8            rsvd202;                // 0x207
    struct EP1DFBits    EP1DF;              // 0x208
    UINT8            rsvd203;                // 0x209
    UINT8            rsvd204;                // 0x20a
    UINT8            rsvd205;                // 0x20b
    struct EP1DTLBits   EP1DTL;             // 0x20C
    UINT8            rsvd206;                // 0x20d
    UINT8            rsvd207;                // 0x20e
    UINT8            rsvd208;                // 0x20f

    struct EP1DTMBits   EP1DTM;             // 0x210
    UINT8            rsvd209;                // 0x211
    UINT8            rsvd210;                // 0x212
    UINT8            rsvd211;                // 0x213
    struct EP1DTHBits   EP1DTH;             // 0x214
    UINT8            rsvd212;                // 0x215
    UINT8            rsvd213;                // 0x216
    UINT8            rsvd214;                // 0x217
    struct EP2DCBits    EP2DC;              // 0x218
    UINT8            rsvd215;                // 0x219
    UINT8            rsvd216;                // 0x21a
    UINT8            rsvd217;                // 0x21b
    struct EP2DUBits    EP2DU;              // 0x21C
    UINT8            rsvd218;                // 0x21d
    UINT8            rsvd219;                // 0x21e
    UINT8            rsvd220;                // 0x21f

    struct EP2DFBits    EP2DF;              // 0x220
    UINT8            rsvd221;                // 0x221
    UINT8            rsvd222;                // 0x222
    UINT8            rsvd223;                // 0x223
    struct EP2DTLBits   EP2DTL;             // 0x224
    UINT8            rsvd224;                // 0x225
    UINT8            rsvd225;                // 0x226
    UINT8            rsvd226;                // 0x227
    struct EP2DTMBits   EP2DTM;             // 0x228
    UINT8            rsvd227;                // 0x229
    UINT8            rsvd228;                // 0x22a
    UINT8            rsvd229;                // 0x22b
    struct EP2DTHBits   EP2DTH;             // 0x22C
    UINT8            rsvd230;                // 0x22d
    UINT8            rsvd231;                // 0x22e
    UINT8            rsvd232;                // 0x22f

    UINT8            rsvd233;                // 0x230
    UINT8            rsvd234;                // 0x231
    UINT8            rsvd235;                // 0x232
    UINT8            rsvd236;                // 0x233
    UINT8            rsvd237;                // 0x234
    UINT8            rsvd238;                // 0x235
    UINT8            rsvd239;                // 0x236
    UINT8            rsvd240;                // 0x237
    UINT8            rsvd241;                // 0x238
    UINT8            rsvd242;                // 0x239
    UINT8            rsvd243;                // 0x23a
    UINT8            rsvd244;                // 0x23b
    UINT8            rsvd245;                // 0x23C
    UINT8            rsvd246;                // 0x23d
    UINT8            rsvd247;                // 0x23e
    UINT8            rsvd248;                // 0x23f

    struct EP3DCBits    EP3DC;              // 0x240
    UINT8            rsvd249;                // 0x241
    UINT8            rsvd250;                // 0x242
    UINT8            rsvd251;                // 0x243
    struct EP3DUBits    EP3DU;              // 0x244
    UINT8            rsvd252;                // 0x245
    UINT8            rsvd253;                // 0x246
    UINT8            rsvd254;                // 0x247
    struct EP3DFBits    EP3DF;              // 0x248
    UINT8            rsvd255;                // 0x249
    UINT8            rsvd256;                // 0x24a
    UINT8            rsvd257;                // 0x24b
    struct EP3DTLBits   EP3DTL;             // 0x24C
    UINT8            rsvd258;                // 0x24d
    UINT8            rsvd259;                // 0x24e
    UINT8            rsvd260;                // 0x24f

    struct EP3DTMBits   EP3DTM;             // 0x250
    UINT8            rsvd261;                // 0x251
    UINT8            rsvd262;                // 0x252
    UINT8            rsvd263;                // 0x253
    struct EP3DTHBits   EP3DTH;             // 0x254
    UINT8            rsvd264;                // 0x255
    UINT8            rsvd265;                // 0x256
    UINT8            rsvd266;                // 0x257
    struct EP4DCBits    EP4DC;              // 0x258
    UINT8            rsvd267;                // 0x259
    UINT8            rsvd268;                // 0x25a
    UINT8            rsvd269;                // 0x25b
    struct EP4DUBits    EP4DU;              // 0x25C
    UINT8            rsvd270;                // 0x25d
    UINT8            rsvd271;                // 0x25e
    UINT8            rsvd272;                // 0x25f

    struct EP4DFBits    EP4DF;              // 0x260
    UINT8            rsvd273;                // 0x261
    UINT8            rsvd274;                // 0x262
    UINT8            rsvd275;                // 0x263
    struct EP4DTLBits   EP4DTL;             // 0x264
    UINT8            rsvd276;                // 0x265
    UINT8            rsvd277;                // 0x266
    UINT8            rsvd278;                // 0x267
    struct EP4DTMBits   EP4DTM;             // 0x268
    UINT8            rsvd279;                // 0x269
    UINT8            rsvd280;                // 0x26a
    UINT8            rsvd281;                // 0x26b
    struct EP4DTHBits   EP4DTH;             // 0x26C

} S3C2440A_USBD_REG, *PS3C2440A_USBD_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
