/*
 * core_fw_pinout.h
 *
 * Created: 16.10.2022
 *  Author: Miroslav Rujzl
 */ 


#ifndef CORE_FW_PINOUT_H_
#define CORE_FW_PINOUT_H_

// PINOUT
#define UART_PC_RX_PIN        0
#define UART_PC_TX_PIN        1
#define UART_PWL_RX_PIN       20
#define UART_PWL_TX_PIN       21

#define I2C_SDA_PIN           18
#define I2C_SDA_PIN           19

#define SPI_SCLK_PIN          9
#define SPI_MOSI_PIN          10
#define SPI_MISO_PIN          11
#define SPI_CS_RDIG_IC_PIN    22
#define SPI_CS_AD_XY_PIN      23
#define SPI_CS_AD_ZW_PIN      24
#define SPI_CS_RDIG_1_PIN     25
#define SPI_CS_RDIG_2_PIN     12

#define KEYPAD_R1_PIN         32
#define KEYPAD_R2_PIN         33
#define KEYPAD_R3_PIN         34
#define KEYPAD_R4_PIN         35
#define KEYPAD_C1_PIN         28
#define KEYPAD_C2_PIN         29
#define KEYPAD_C3_PIN         30
#define KEYPAD_C4_PIN         31

#endif /* CORE_FW_PINOUT_H_ */
