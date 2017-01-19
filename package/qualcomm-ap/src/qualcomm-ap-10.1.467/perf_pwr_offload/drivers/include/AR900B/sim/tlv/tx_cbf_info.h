// Copyright (c) 2012 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  This file is automatically generated
//               These definitions are tied to a particular hardware layout


#ifndef _TX_CBF_INFO_H_
#define _TX_CBF_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	chain_mask[3:0], paprd_chain_mask[7:4], bw_in_service[9:8], reserved_0a[13:10], dot11b_preamble_type[14], bf[15], pkt_type[17:16], nss[20:18], bandwidth[22:21], ndp[23], reserved_0b[29:24], channel_capture[30], warm_tx[31]
//	1	tpc_init_pwr_cfg[2:0], dpd_enable[3], tx_pwr[10:4], clpc_en[11], measure_power[12], reserved_1[31:13]
//	2	cfb_lsig[31:0]
//	3	cfb_vht_siga1[23:0], reserved_2[31:24]
//	4	cfb_vht_siga2[23:0], reserved_3[31:24]
//	5	cfb_vht_sigb[31:0]
//	6	cfb_vht_pad[15:0], reserved_5[31:16]
//	7	frame_control[15:0], duration[31:16]
//	8	addr1_31_0[31:0]
//	9	addr1_47_32[15:0], addr2_15_0[31:16]
//	10	addr2_47_16[31:0]
//	11	addr3_31_0[31:0]
//	12	addr3_47_32[15:0], sequence_control[31:16]
//	13	category[7:0], vht_action[15:8], vht_mimo_control_15_0[31:16]
//	14	vht_mimo_control_23_16[7:0], reserved_12[31:8]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_TX_CBF_INFO 15

struct tx_cbf_info {
    volatile uint32_t chain_mask                      :  4, //[3:0]
                      paprd_chain_mask                :  4, //[7:4]
                      bw_in_service                   :  2, //[9:8]
                      reserved_0a                     :  4, //[13:10]
                      dot11b_preamble_type            :  1, //[14]
                      bf                              :  1, //[15]
                      pkt_type                        :  2, //[17:16]
                      nss                             :  3, //[20:18]
                      bandwidth                       :  2, //[22:21]
                      ndp                             :  1, //[23]
                      reserved_0b                     :  6, //[29:24]
                      channel_capture                 :  1, //[30]
                      warm_tx                         :  1; //[31]
    volatile uint32_t tpc_init_pwr_cfg                :  3, //[2:0]
                      dpd_enable                      :  1, //[3]
                      tx_pwr                          :  7, //[10:4]
                      clpc_en                         :  1, //[11]
                      measure_power                   :  1, //[12]
                      reserved_1                      : 19; //[31:13]
    volatile uint32_t cfb_lsig                        : 32; //[31:0]
    volatile uint32_t cfb_vht_siga1                   : 24, //[23:0]
                      reserved_2                      :  8; //[31:24]
    volatile uint32_t cfb_vht_siga2                   : 24, //[23:0]
                      reserved_3                      :  8; //[31:24]
    volatile uint32_t cfb_vht_sigb                    : 32; //[31:0]
    volatile uint32_t cfb_vht_pad                     : 16, //[15:0]
                      reserved_5                      : 16; //[31:16]
    volatile uint32_t frame_control                   : 16, //[15:0]
                      duration                        : 16; //[31:16]
    volatile uint32_t addr1_31_0                      : 32; //[31:0]
    volatile uint32_t addr1_47_32                     : 16, //[15:0]
                      addr2_15_0                      : 16; //[31:16]
    volatile uint32_t addr2_47_16                     : 32; //[31:0]
    volatile uint32_t addr3_31_0                      : 32; //[31:0]
    volatile uint32_t addr3_47_32                     : 16, //[15:0]
                      sequence_control                : 16; //[31:16]
    volatile uint32_t category                        :  8, //[7:0]
                      vht_action                      :  8, //[15:8]
                      vht_mimo_control_15_0           : 16; //[31:16]
    volatile uint32_t vht_mimo_control_23_16          :  8, //[7:0]
                      reserved_12                     : 24; //[31:8]
};

/*

chain_mask
			
			Chain mask to support up to 4 antennas.  <legal: 0-3>
			
			Set the value from register
			MAC_PCU_SELF_GEN_ANTENNA_MASK.

paprd_chain_mask
			
			Chain mask for the PA predistortion function.  <legal:
			0>  (tied to 0)

bw_in_service
			
			Specifies the bandwidth mode in the service field for
			legacy control packets:
			
			<enum 0     no_bw> service[6:0] is 7 random bits filled
			in by PHY
			
			<enum 1     bw_only> service[6:5] is BW where 0 means 20
			MHz, 1 means 40 MHz, 2 means 80 MHz, and 3 means 160 MHz
			(160 MHz is not supported); and service[4:0] is 5 random
			bits filled by PHY
			
			<enum 2     bw_dynamic> service[6:5] is BW, service[4]
			if set indicates dynamic BW operation or if clear indicates
			static BW operation, service[3:0] is 4 random bits filled by
			PHY
			
			<legal 0> (tied to 0)

reserved_0a
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>

dot11b_preamble_type
			
			Valid for 802.11b packets only.
			
			<enum_typedef tx_phy_desc dot11b_preamble_type>
			
			
			
			<enum 0     short_preamble>
			
			<enum 1     long_preamble>
			
			<legal 0> (tied to 0)

bf
			
			This field is set if beamforming is enabled.  <legal 0>
			(tied to 0)

pkt_type
			
			Packet type:
			
			<enum_typedef tx_phy_desc pkt_type>
			
			<enum 0     dot11a>
			
			
			
			<enum 1     dot11b>
			
			<enum 2     dot11n_mm>
			
			<enum 3     dot11ac>
			
			Note: HT Greenfield packet type is not supported
			
			<legal 3> (tied to 3(vht))

nss
			
			Number of spatial streams.
			
			
			
			<enum 0     1_spatial_stream>
			
			<enum 1     2_spatial_streams>
			
			<enum 2     3_spatial_streams>
			
			<legal 0> (tied to 0)

bandwidth
			
			Packet bandwidth:
			
			<enum_typedef tx_phy_desc bandwidth>
			
			
			
			<enum 0     20_mhz>
			
			<enum 1     40_mhz>
			
			<enum 2     80_mhz>
			
			<enum 3     160_mhz> Not supported
			
			<legal 0-2>

ndp
			
			Flag to define NDP transmission  <legal 0>
			
			(tied to 0)

reserved_0b
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>

channel_capture
			
			Indicates that the BB should be armed to capture the
			channel on the next received packet. This channel estimate
			is passed to the MAC if the packet is successfully received.
			<legal 0> (tied to 0)

warm_tx
			
			Indicates that this PPDU is being transmitted early in
			warm_tx mode. <legal 0-1>
			
			Filled by TX PCU after TX PCU evaluation. 

tpc_init_pwr_cfg
			
			Initial analog gain and PA configuration setting where 0
			has the highest power consumption and best EVM,  and 4 has
			the lowest power consumption and worst EVM <legal 0> (tied
			to 0)

dpd_enable
			
			<enum 0     dpd_off> DPD profile not applied to current
			packet
			
			<enum 1     dpd_on> DPD profile applied to current
			packet if available
			
			<legal 0> (tied to 0)

tx_pwr
			
			Transmit Power (signed value) in units of 0.5 dBm <legal
			all> (from MAC_PCU_TPC_TXBF_FB_PWR)

clpc_en
			
			This field enables closed-loop TPC operation by enabling
			CLPC adjustment of DAC gain for the next packet.
			
			<enum 0     clpc_off> TPC error update disabled
			
			<enum 1     clpc_on> TPC error will be applied to DAC
			gain setting for the next packet
			
			<legal 0> (tied to 0)

measure_power
			
			This field enables the TPC to use power measurement for
			current packet in CLPC updates.
			
			<enum 0     measure_dis> TPC will not latch power
			measurement result for current packet
			
			<enum 1     measure_en> TPC will latch power measurement
			result for current packet
			
			<legal 0> (tied to 0)

reserved_1
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>

cfb_lsig
			
			Parameters related to L-SIG Preamble for the CBF frame.

cfb_vht_siga1
			
			Parameters related to VHT-SIGA1 Preamble for the CBF
			frame.

reserved_2
			
			Reserved: HW should fill with 0

cfb_vht_siga2
			
			Parameters related to VHT-SIGA2 Preamble for the CBF
			frame.

reserved_3
			
			Reserved: HW should fill with 0

cfb_vht_sigb
			
			Parameters related to VHT-SIG B Preamble for the CBF
			frame.

cfb_vht_pad
			
			Valid with rx_pcu_frame_received.
			
			Parameters related to VHT Pad for the CBF frame.

reserved_5
			
			Reserved: HW should fill with 0

frame_control
			
			CBF frame control

duration
			
			CBF duration (*)

addr1_31_0
			
			CBF address1[31:0]

addr1_47_32
			
			CBF address1[47:32]

addr2_15_0
			
			CBF address2[15:0]

addr2_47_16
			
			CBF address2[47:16]

addr3_31_0
			
			CBF address3[31:0]

addr3_47_32
			
			CBF address3[47:16]

sequence_control
			
			CBF sequence control (*)

category
			
			VHT category:  set to 21

vht_action
			
			VHT action: set to 0

vht_mimo_control_15_0
			
			VHT MIMO control[15:0] (**)

vht_mimo_control_23_16
			
			VHT MIMO control[23:16]

reserved_12
			
			Reserved: HW should fill with 0
*/


/* Description		TX_CBF_INFO_0_CHAIN_MASK
			
			Chain mask to support up to 4 antennas.  <legal: 0-3>
			
			Set the value from register
			MAC_PCU_SELF_GEN_ANTENNA_MASK.
*/
#define TX_CBF_INFO_0_CHAIN_MASK_OFFSET                              0x00000000
#define TX_CBF_INFO_0_CHAIN_MASK_LSB                                 0
#define TX_CBF_INFO_0_CHAIN_MASK_MASK                                0x0000000f

/* Description		TX_CBF_INFO_0_PAPRD_CHAIN_MASK
			
			Chain mask for the PA predistortion function.  <legal:
			0>  (tied to 0)
*/
#define TX_CBF_INFO_0_PAPRD_CHAIN_MASK_OFFSET                        0x00000000
#define TX_CBF_INFO_0_PAPRD_CHAIN_MASK_LSB                           4
#define TX_CBF_INFO_0_PAPRD_CHAIN_MASK_MASK                          0x000000f0

/* Description		TX_CBF_INFO_0_BW_IN_SERVICE
			
			Specifies the bandwidth mode in the service field for
			legacy control packets:
			
			<enum 0     no_bw> service[6:0] is 7 random bits filled
			in by PHY
			
			<enum 1     bw_only> service[6:5] is BW where 0 means 20
			MHz, 1 means 40 MHz, 2 means 80 MHz, and 3 means 160 MHz
			(160 MHz is not supported); and service[4:0] is 5 random
			bits filled by PHY
			
			<enum 2     bw_dynamic> service[6:5] is BW, service[4]
			if set indicates dynamic BW operation or if clear indicates
			static BW operation, service[3:0] is 4 random bits filled by
			PHY
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_0_BW_IN_SERVICE_OFFSET                           0x00000000
#define TX_CBF_INFO_0_BW_IN_SERVICE_LSB                              8
#define TX_CBF_INFO_0_BW_IN_SERVICE_MASK                             0x00000300

/* Description		TX_CBF_INFO_0_RESERVED_0A
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/
#define TX_CBF_INFO_0_RESERVED_0A_OFFSET                             0x00000000
#define TX_CBF_INFO_0_RESERVED_0A_LSB                                10
#define TX_CBF_INFO_0_RESERVED_0A_MASK                               0x00003c00

/* Description		TX_CBF_INFO_0_DOT11B_PREAMBLE_TYPE
			
			Valid for 802.11b packets only.
			
			<enum_typedef tx_phy_desc dot11b_preamble_type>
			
			
			
			<enum 0     short_preamble>
			
			<enum 1     long_preamble>
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_0_DOT11B_PREAMBLE_TYPE_OFFSET                    0x00000000
#define TX_CBF_INFO_0_DOT11B_PREAMBLE_TYPE_LSB                       14
#define TX_CBF_INFO_0_DOT11B_PREAMBLE_TYPE_MASK                      0x00004000

/* Description		TX_CBF_INFO_0_BF
			
			This field is set if beamforming is enabled.  <legal 0>
			(tied to 0)
*/
#define TX_CBF_INFO_0_BF_OFFSET                                      0x00000000
#define TX_CBF_INFO_0_BF_LSB                                         15
#define TX_CBF_INFO_0_BF_MASK                                        0x00008000

/* Description		TX_CBF_INFO_0_PKT_TYPE
			
			Packet type:
			
			<enum_typedef tx_phy_desc pkt_type>
			
			<enum 0     dot11a>
			
			
			
			<enum 1     dot11b>
			
			<enum 2     dot11n_mm>
			
			<enum 3     dot11ac>
			
			Note: HT Greenfield packet type is not supported
			
			<legal 3> (tied to 3(vht))
*/
#define TX_CBF_INFO_0_PKT_TYPE_OFFSET                                0x00000000
#define TX_CBF_INFO_0_PKT_TYPE_LSB                                   16
#define TX_CBF_INFO_0_PKT_TYPE_MASK                                  0x00030000

/* Description		TX_CBF_INFO_0_NSS
			
			Number of spatial streams.
			
			
			
			<enum 0     1_spatial_stream>
			
			<enum 1     2_spatial_streams>
			
			<enum 2     3_spatial_streams>
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_0_NSS_OFFSET                                     0x00000000
#define TX_CBF_INFO_0_NSS_LSB                                        18
#define TX_CBF_INFO_0_NSS_MASK                                       0x001c0000

/* Description		TX_CBF_INFO_0_BANDWIDTH
			
			Packet bandwidth:
			
			<enum_typedef tx_phy_desc bandwidth>
			
			
			
			<enum 0     20_mhz>
			
			<enum 1     40_mhz>
			
			<enum 2     80_mhz>
			
			<enum 3     160_mhz> Not supported
			
			<legal 0-2>
*/
#define TX_CBF_INFO_0_BANDWIDTH_OFFSET                               0x00000000
#define TX_CBF_INFO_0_BANDWIDTH_LSB                                  21
#define TX_CBF_INFO_0_BANDWIDTH_MASK                                 0x00600000

/* Description		TX_CBF_INFO_0_NDP
			
			Flag to define NDP transmission  <legal 0>
			
			(tied to 0)
*/
#define TX_CBF_INFO_0_NDP_OFFSET                                     0x00000000
#define TX_CBF_INFO_0_NDP_LSB                                        23
#define TX_CBF_INFO_0_NDP_MASK                                       0x00800000

/* Description		TX_CBF_INFO_0_RESERVED_0B
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/
#define TX_CBF_INFO_0_RESERVED_0B_OFFSET                             0x00000000
#define TX_CBF_INFO_0_RESERVED_0B_LSB                                24
#define TX_CBF_INFO_0_RESERVED_0B_MASK                               0x3f000000

/* Description		TX_CBF_INFO_0_CHANNEL_CAPTURE
			
			Indicates that the BB should be armed to capture the
			channel on the next received packet. This channel estimate
			is passed to the MAC if the packet is successfully received.
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_0_CHANNEL_CAPTURE_OFFSET                         0x00000000
#define TX_CBF_INFO_0_CHANNEL_CAPTURE_LSB                            30
#define TX_CBF_INFO_0_CHANNEL_CAPTURE_MASK                           0x40000000

/* Description		TX_CBF_INFO_0_WARM_TX
			
			Indicates that this PPDU is being transmitted early in
			warm_tx mode. <legal 0-1>
			
			Filled by TX PCU after TX PCU evaluation. 
*/
#define TX_CBF_INFO_0_WARM_TX_OFFSET                                 0x00000000
#define TX_CBF_INFO_0_WARM_TX_LSB                                    31
#define TX_CBF_INFO_0_WARM_TX_MASK                                   0x80000000

/* Description		TX_CBF_INFO_1_TPC_INIT_PWR_CFG
			
			Initial analog gain and PA configuration setting where 0
			has the highest power consumption and best EVM,  and 4 has
			the lowest power consumption and worst EVM <legal 0> (tied
			to 0)
*/
#define TX_CBF_INFO_1_TPC_INIT_PWR_CFG_OFFSET                        0x00000004
#define TX_CBF_INFO_1_TPC_INIT_PWR_CFG_LSB                           0
#define TX_CBF_INFO_1_TPC_INIT_PWR_CFG_MASK                          0x00000007

/* Description		TX_CBF_INFO_1_DPD_ENABLE
			
			<enum 0     dpd_off> DPD profile not applied to current
			packet
			
			<enum 1     dpd_on> DPD profile applied to current
			packet if available
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_1_DPD_ENABLE_OFFSET                              0x00000004
#define TX_CBF_INFO_1_DPD_ENABLE_LSB                                 3
#define TX_CBF_INFO_1_DPD_ENABLE_MASK                                0x00000008

/* Description		TX_CBF_INFO_1_TX_PWR
			
			Transmit Power (signed value) in units of 0.5 dBm <legal
			all> (from MAC_PCU_TPC_TXBF_FB_PWR)
*/
#define TX_CBF_INFO_1_TX_PWR_OFFSET                                  0x00000004
#define TX_CBF_INFO_1_TX_PWR_LSB                                     4
#define TX_CBF_INFO_1_TX_PWR_MASK                                    0x000007f0

/* Description		TX_CBF_INFO_1_CLPC_EN
			
			This field enables closed-loop TPC operation by enabling
			CLPC adjustment of DAC gain for the next packet.
			
			<enum 0     clpc_off> TPC error update disabled
			
			<enum 1     clpc_on> TPC error will be applied to DAC
			gain setting for the next packet
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_1_CLPC_EN_OFFSET                                 0x00000004
#define TX_CBF_INFO_1_CLPC_EN_LSB                                    11
#define TX_CBF_INFO_1_CLPC_EN_MASK                                   0x00000800

/* Description		TX_CBF_INFO_1_MEASURE_POWER
			
			This field enables the TPC to use power measurement for
			current packet in CLPC updates.
			
			<enum 0     measure_dis> TPC will not latch power
			measurement result for current packet
			
			<enum 1     measure_en> TPC will latch power measurement
			result for current packet
			
			<legal 0> (tied to 0)
*/
#define TX_CBF_INFO_1_MEASURE_POWER_OFFSET                           0x00000004
#define TX_CBF_INFO_1_MEASURE_POWER_LSB                              12
#define TX_CBF_INFO_1_MEASURE_POWER_MASK                             0x00001000

/* Description		TX_CBF_INFO_1_RESERVED_1
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/
#define TX_CBF_INFO_1_RESERVED_1_OFFSET                              0x00000004
#define TX_CBF_INFO_1_RESERVED_1_LSB                                 13
#define TX_CBF_INFO_1_RESERVED_1_MASK                                0xffffe000

/* Description		TX_CBF_INFO_2_CFB_LSIG
			
			Parameters related to L-SIG Preamble for the CBF frame.
*/
#define TX_CBF_INFO_2_CFB_LSIG_OFFSET                                0x00000008
#define TX_CBF_INFO_2_CFB_LSIG_LSB                                   0
#define TX_CBF_INFO_2_CFB_LSIG_MASK                                  0xffffffff

/* Description		TX_CBF_INFO_3_CFB_VHT_SIGA1
			
			Parameters related to VHT-SIGA1 Preamble for the CBF
			frame.
*/
#define TX_CBF_INFO_3_CFB_VHT_SIGA1_OFFSET                           0x0000000c
#define TX_CBF_INFO_3_CFB_VHT_SIGA1_LSB                              0
#define TX_CBF_INFO_3_CFB_VHT_SIGA1_MASK                             0x00ffffff

/* Description		TX_CBF_INFO_3_RESERVED_2
			
			Reserved: HW should fill with 0
*/
#define TX_CBF_INFO_3_RESERVED_2_OFFSET                              0x0000000c
#define TX_CBF_INFO_3_RESERVED_2_LSB                                 24
#define TX_CBF_INFO_3_RESERVED_2_MASK                                0xff000000

/* Description		TX_CBF_INFO_4_CFB_VHT_SIGA2
			
			Parameters related to VHT-SIGA2 Preamble for the CBF
			frame.
*/
#define TX_CBF_INFO_4_CFB_VHT_SIGA2_OFFSET                           0x00000010
#define TX_CBF_INFO_4_CFB_VHT_SIGA2_LSB                              0
#define TX_CBF_INFO_4_CFB_VHT_SIGA2_MASK                             0x00ffffff

/* Description		TX_CBF_INFO_4_RESERVED_3
			
			Reserved: HW should fill with 0
*/
#define TX_CBF_INFO_4_RESERVED_3_OFFSET                              0x00000010
#define TX_CBF_INFO_4_RESERVED_3_LSB                                 24
#define TX_CBF_INFO_4_RESERVED_3_MASK                                0xff000000

/* Description		TX_CBF_INFO_5_CFB_VHT_SIGB
			
			Parameters related to VHT-SIG B Preamble for the CBF
			frame.
*/
#define TX_CBF_INFO_5_CFB_VHT_SIGB_OFFSET                            0x00000014
#define TX_CBF_INFO_5_CFB_VHT_SIGB_LSB                               0
#define TX_CBF_INFO_5_CFB_VHT_SIGB_MASK                              0xffffffff

/* Description		TX_CBF_INFO_6_CFB_VHT_PAD
			
			Valid with rx_pcu_frame_received.
			
			Parameters related to VHT Pad for the CBF frame.
*/
#define TX_CBF_INFO_6_CFB_VHT_PAD_OFFSET                             0x00000018
#define TX_CBF_INFO_6_CFB_VHT_PAD_LSB                                0
#define TX_CBF_INFO_6_CFB_VHT_PAD_MASK                               0x0000ffff

/* Description		TX_CBF_INFO_6_RESERVED_5
			
			Reserved: HW should fill with 0
*/
#define TX_CBF_INFO_6_RESERVED_5_OFFSET                              0x00000018
#define TX_CBF_INFO_6_RESERVED_5_LSB                                 16
#define TX_CBF_INFO_6_RESERVED_5_MASK                                0xffff0000

/* Description		TX_CBF_INFO_7_FRAME_CONTROL
			
			CBF frame control
*/
#define TX_CBF_INFO_7_FRAME_CONTROL_OFFSET                           0x0000001c
#define TX_CBF_INFO_7_FRAME_CONTROL_LSB                              0
#define TX_CBF_INFO_7_FRAME_CONTROL_MASK                             0x0000ffff

/* Description		TX_CBF_INFO_7_DURATION
			
			CBF duration (*)
*/
#define TX_CBF_INFO_7_DURATION_OFFSET                                0x0000001c
#define TX_CBF_INFO_7_DURATION_LSB                                   16
#define TX_CBF_INFO_7_DURATION_MASK                                  0xffff0000

/* Description		TX_CBF_INFO_8_ADDR1_31_0
			
			CBF address1[31:0]
*/
#define TX_CBF_INFO_8_ADDR1_31_0_OFFSET                              0x00000020
#define TX_CBF_INFO_8_ADDR1_31_0_LSB                                 0
#define TX_CBF_INFO_8_ADDR1_31_0_MASK                                0xffffffff

/* Description		TX_CBF_INFO_9_ADDR1_47_32
			
			CBF address1[47:32]
*/
#define TX_CBF_INFO_9_ADDR1_47_32_OFFSET                             0x00000024
#define TX_CBF_INFO_9_ADDR1_47_32_LSB                                0
#define TX_CBF_INFO_9_ADDR1_47_32_MASK                               0x0000ffff

/* Description		TX_CBF_INFO_9_ADDR2_15_0
			
			CBF address2[15:0]
*/
#define TX_CBF_INFO_9_ADDR2_15_0_OFFSET                              0x00000024
#define TX_CBF_INFO_9_ADDR2_15_0_LSB                                 16
#define TX_CBF_INFO_9_ADDR2_15_0_MASK                                0xffff0000

/* Description		TX_CBF_INFO_10_ADDR2_47_16
			
			CBF address2[47:16]
*/
#define TX_CBF_INFO_10_ADDR2_47_16_OFFSET                            0x00000028
#define TX_CBF_INFO_10_ADDR2_47_16_LSB                               0
#define TX_CBF_INFO_10_ADDR2_47_16_MASK                              0xffffffff

/* Description		TX_CBF_INFO_11_ADDR3_31_0
			
			CBF address3[31:0]
*/
#define TX_CBF_INFO_11_ADDR3_31_0_OFFSET                             0x0000002c
#define TX_CBF_INFO_11_ADDR3_31_0_LSB                                0
#define TX_CBF_INFO_11_ADDR3_31_0_MASK                               0xffffffff

/* Description		TX_CBF_INFO_12_ADDR3_47_32
			
			CBF address3[47:16]
*/
#define TX_CBF_INFO_12_ADDR3_47_32_OFFSET                            0x00000030
#define TX_CBF_INFO_12_ADDR3_47_32_LSB                               0
#define TX_CBF_INFO_12_ADDR3_47_32_MASK                              0x0000ffff

/* Description		TX_CBF_INFO_12_SEQUENCE_CONTROL
			
			CBF sequence control (*)
*/
#define TX_CBF_INFO_12_SEQUENCE_CONTROL_OFFSET                       0x00000030
#define TX_CBF_INFO_12_SEQUENCE_CONTROL_LSB                          16
#define TX_CBF_INFO_12_SEQUENCE_CONTROL_MASK                         0xffff0000

/* Description		TX_CBF_INFO_13_CATEGORY
			
			VHT category:  set to 21
*/
#define TX_CBF_INFO_13_CATEGORY_OFFSET                               0x00000034
#define TX_CBF_INFO_13_CATEGORY_LSB                                  0
#define TX_CBF_INFO_13_CATEGORY_MASK                                 0x000000ff

/* Description		TX_CBF_INFO_13_VHT_ACTION
			
			VHT action: set to 0
*/
#define TX_CBF_INFO_13_VHT_ACTION_OFFSET                             0x00000034
#define TX_CBF_INFO_13_VHT_ACTION_LSB                                8
#define TX_CBF_INFO_13_VHT_ACTION_MASK                               0x0000ff00

/* Description		TX_CBF_INFO_13_VHT_MIMO_CONTROL_15_0
			
			VHT MIMO control[15:0] (**)
*/
#define TX_CBF_INFO_13_VHT_MIMO_CONTROL_15_0_OFFSET                  0x00000034
#define TX_CBF_INFO_13_VHT_MIMO_CONTROL_15_0_LSB                     16
#define TX_CBF_INFO_13_VHT_MIMO_CONTROL_15_0_MASK                    0xffff0000

/* Description		TX_CBF_INFO_14_VHT_MIMO_CONTROL_23_16
			
			VHT MIMO control[23:16]
*/
#define TX_CBF_INFO_14_VHT_MIMO_CONTROL_23_16_OFFSET                 0x00000038
#define TX_CBF_INFO_14_VHT_MIMO_CONTROL_23_16_LSB                    0
#define TX_CBF_INFO_14_VHT_MIMO_CONTROL_23_16_MASK                   0x000000ff

/* Description		TX_CBF_INFO_14_RESERVED_12
			
			Reserved: HW should fill with 0
*/
#define TX_CBF_INFO_14_RESERVED_12_OFFSET                            0x00000038
#define TX_CBF_INFO_14_RESERVED_12_LSB                               8
#define TX_CBF_INFO_14_RESERVED_12_MASK                              0xffffff00


#endif // _TX_CBF_INFO_H_
