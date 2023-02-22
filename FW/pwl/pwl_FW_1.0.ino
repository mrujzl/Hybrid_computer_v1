/*
 *    PWL_FW_1.0
 * 
 * Author: Miroslav Rujzl
 * 
 * This code is used for the piecewise linear function block MCU 
 * of the hybrid computer for simulation and modeling of nonlinear 
 * dynamic systems. It was developed at Department of Radio Electronics, 
 * Brno University of Technology within the grant Development of 
 * analog computer with hybrid analog-digital components for 
 * accurate modeling of chaotic systems. This grant is realised 
 * within the project Quality Internal Grants of BUT (KInG BUT), 
 * Reg. No. CZ.02.2.69 / 0.0 / 0.0 / 19_073 / 0016948, which is 
 * financed from the OP RDE.
 * 
 * Controlable components:
 * - UART for communication with Core MCU
 * - 1x digital potentiometers MAX5437EUD+
 * - 1x AD converters AD7322BRUZ
 * 
 */

// --- Includes ---

#include <SPI.h>

#include "pwl_fw_pinout.h"
//#include "max5437eud_reg_ctrl.h"
//#include "ad7322bruz_reg_ctrl.h"

// --- Defines ---

// Components selections
#define RDIG_PWL               0
#define AD_PWL                 1

// UART settings
#define UART_CORE_BAUDRATE     9600

// --- Typedefs ---



// --- Global variables ---



// --- Prototypes ---



// --- Functions ---

void setup()
{
  Serial.begin(UART_CORE_BAUDRATE);

  pinMode(2,OUTPUT);

}

void loop()
{
  Serial.printf("PWL_OK;");

  delay(1000);

  digitalWrite(2,LOW);

  delay(1000);
  
  digitalWrite(2,HIGH);

  delay(1000);
 
}
