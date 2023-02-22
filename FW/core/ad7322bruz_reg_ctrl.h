/*
 * ad7322bruz_reg_ctrl.h
 *
 * Created: 16.10.2022
 *  Author: Miroslav Rujzl
 */ 


#ifndef AD7322BRUZ_REG_CTRL_H_
#define AD7322BRUZ_REG_CTRL_H_

// Registers

#define AD7322_NONE_REG_SEL             0x0000 // 0b 0000 0000 0000 0000 - no register selected, data is ignored
#define AD7322_CONTROL_REG_SEL          0x8000 // 0b 1000 0000 0000 0000 - control register selected
#define AD7322_RANGE_REG_SEL            0xA000 // 0b 1010 0000 0000 0000 - range register selected

// Control

#define AD7322_CTRL_ADD0_V0             0x0000 // 0b0000 0000 0000 0000 - next conversion will be from channel V0
#define AD7322_CTRL_ADD0_V1             0x0400 // 0b0000 0100 0000 0000 - next conversion will be from channel V1

#define AD7322_CTRL_MODE_FULL_DIFF      0x0200 // 0b0000 0010 0000 0000 - fully differential analog inputs
#define AD7322_CTRL_MODE_PSEUDO_DIFF    0x0100 // 0b0000 0001 0000 0000 - pseudo differential analog inputs
#define AD7322_CTRL_MODE_SINGLE_ENDED   0x0000 // 0b0000 0000 0000 0000 - single-ended analog inputs

#define AD7322_CTRL_PWRMODE_FULL_SHUTDOWN        0x00C0 // 0b0000 0000 1100 0000 - full shutdown mode, all internal circuitry on the AD7322 is powered down
#define AD7322_CTRL_PWRMODE_AUTOSHUTDOWN         0x0080 // 0b0000 0000 1000 0000 - autoshutdown mode, AD7322 enters autoshutdown on the 15th SCLK rising edge when the control register is updated
#define AD7322_CTRL_PWRMODE_AUTOSTANDBY          0x0040 // 0b0000 0000 0100 0000 - autostandby mode, all internal circuitry is powered down excluding the internal reference
#define AD7322_CTRL_PWRMODE_NORMAL_MODE          0x0000 // 0b0000 0000 0000 0000 - normal mode, all internal circuitry is powered up at all times

#define AD7322_CTRL_CODING_TWO_COMP     0x0000 // 0b0000 0000 0000 0000 - output coding is twos complement and used for the next conversion
#define AD7322_CTRL_CODING_BINARY       0x0020 // 0b0000 0000 0010 0000 - output coding is straight binary and used for the next conversion

#define AD7322_CTRL_REF_INT             0x0010 // 0b0000 0000 0000 0000 - internal reference is enabled and used for the next conversion
#define AD7322_CTRL_REF_EXT             0x0000 // 0b0000 0000 0000 0000 - external reference is enabled and used for the next conversion

#define AD7322_CTRL_SEQ_NOT_USED        0x000C // 0b0000 0000 0000 1100 - channel sequencer is not used
#define AD7322_CTRL_SEQ_ENABLE          0x0008 // 0b0000 0000 0000 1000 - allows continuous conversions on a consecutive sequence of channels, from Channel 0 up to Channel 1, as selected by the channel address bits in the control register

#define AD7322_RNGE_V0_BIPOLAR_10V      0x0000 // 0b0000 0000 0000 0000 - +-10V input range for channel V0
#define AD7322_RNGE_V0_BIPOLAR_5V       0x0800 // 0b0000 1000 0000 0000 - +-5V input range for channel V0
#define AD7322_RNGE_V0_BIPOLAR_2_5V     0x1000 // 0b0001 0000 0000 0000 - +-2.5V input range for channel V0
#define AD7322_RNGE_V0_UNIPOLAR_10V     0x1800 // 0b0001 1000 0000 0000 - 0 - 10V input range for channel V0

#define AD7322_RNGE_V1_BIPOLAR_10V      0x0000 // 0b0000 0000 0000 0000 - +-10V input range for channel V1
#define AD7322_RNGE_V1_BIPOLAR_5V       0x0080 // 0b0000 0000 1000 0000 - +-5V input range for channel V1
#define AD7322_RNGE_V1_BIPOLAR_2_5V     0x0100 // 0b0000 0001 0000 0000 - +-2.5V input range for channel V1
#define AD7322_RNGE_V1_UNIPOLAR_10V     0x0180 // 0b0000 0001 1000 0000 - 0 - 10V input range for channel V1

#endif /* AD7322BRUZ_REG_CTRL_H_ */
