/*
      CORE_FW_1.0

   Author: Miroslav Rujzl

   This code is used for the main MCU of the hybrid computer
   for simulation and modeling of nonlinear dynamic systems.
   It was developed at Department of Radio Electronics,
   Brno University of Technology within the grant Development of
   analog computer with hybrid analog-digital components for
   accurate modeling of chaotic systems. This grant is realised
   within the project Quality Internal Grants of BUT (KInG BUT),
   Reg. No. CZ.02.2.69 / 0.0 / 0.0 / 19_073 / 0016948, which is
   financed from the OP RDE.

   Information about the board (MegaCore):
   - MCU ATMega64
   - External 16MHz crystal
   - BOD 2.7V
   - EEPROM retained
   - LTO disabled
   - No bootloader
   - Arduino as ISP (MegaCore) programmer

   Controlable components:
  ´- Keyboard ADAFRUIT 3844
   - I2C display
   - UART for communication with PC
   - 3x digital potentiometers MAX5437EUD+ (2x counting potentiometers,
     1x for initial conditions settings)
   - 2x AD converters AD7322BRUZ
   - 1x PWL block (communication via UART commands)

*/

// --- Includes ---

#include <Keypad.h>
#include "U8glib.h"
#include <SPI.h>

#include "core_fw_pinout.h"
#include "max5437eud_reg_ctrl.h"
#include "ad7322bruz_reg_ctrl.h"

// --- Defines ---

// UART settings
#define UART_PC_BAUDRATE      115200
#define UART_PWL_BAUDRATE     9600

// Display settings
#define REMOTE_DIS  0
#define REMOTE_EN   1
#define RECORD_DIS  0
#define RECORD_EN   1

// STM defines
#define STM_STATE_INIT    0
#define STM_STATE_MAIN    1
#define STM_STATE_IC      2
#define STM_STATE_RDIG    3
#define STM_STATE_PWL     4
#define STM_STATE_RECORD  5
#define STM_STATE_REMOTE  6

// Remote defines
#define REMOTE_IC         1
#define REMOTE_R1         2
#define REMOTE_R2         3
#define REMOTE_PWL        4
#define REMOTE_RECSTART   5
#define REMOTE_RECSTOP    6     

// --- Typedefs ---

typedef void (*function_p)();

// --- Global variables ---

// Keypad config
const uint8_t   rows = 4;
const uint8_t   columns = 4;
char            keys[rows][columns] = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
uint8_t         pinrows[rows] = {KEYPAD_R1_PIN, KEYPAD_R2_PIN, KEYPAD_R3_PIN, KEYPAD_R4_PIN};
uint8_t         pincolumns[columns] = {KEYPAD_C1_PIN, KEYPAD_C2_PIN, KEYPAD_C3_PIN, KEYPAD_C4_PIN};

// Keypad variables
Keypad keypad_core = Keypad(makeKeymap(keys), pinrows, pincolumns, rows, columns);
char pressed_key;

// Display variables
U8GLIB_SSD1306_128X64 display_core(U8G_I2C_OPT_NONE);
const int blink_time = 3000;

//UART variables
String uart_pc_rx_buffer;
String uart_pwl_rx_buffer;

//STM variables
uint8_t stm_state = STM_STATE_INIT;

// ADC settings
uint16_t control_reg;
uint16_t range_reg;

// --- Prototypes ---

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void serialFlush(void);
void serial1Flush(void);
void remote(void);
void display_update(function_p func_to_print);
void print_init();
void print_menu();
void print_wrong_key();
void print_pwl();
void print_recstart();
void print_recstop();
void print_nonconnected();
void print_connected();
void print_disconnected();
void rdig_send(uint8_t data_to_send, uint8_t device);
void adc_config(void);
void adc_read(uint8_t device, int16_t * result_0, int16_t * result_1);
void initial_conditions();
void rdig();
void record();

// --- Functions ---

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void serialFlush(void)
{
  while(Serial.available() > 0) 
  {
    char t = Serial.read();
  }
}

void serial1Flush(void)
{
  while(Serial1.available() > 0) 
  {
    char t = Serial1.read();
  }
}

void remote(void)
{
  uint8_t remote_ctrl = REMOTE_DIS;
  uint8_t record_ctrl = RECORD_DIS;
  float   setval      = 0;
  uint8_t spi_val     = 0;
  String  setval_str;

  unsigned long last_rec_time = micros();
  unsigned long period = 20;

  int16_t ad_value[4];

  display_update(print_nonconnected);

  serialFlush();
  
  while(remote_ctrl == REMOTE_DIS)
  {
    if(Serial.available() > 0)
    {
      uart_pc_rx_buffer = Serial.readStringUntil('\n');
    }
    
    if(uart_pc_rx_buffer.equals("CON"))
    {
      Serial.println("OK");
      remote_ctrl = REMOTE_EN;
    }
  }

  display_update(print_connected);

  while(remote_ctrl == REMOTE_EN)
  {
    if(Serial.available() > 0)
    {
      uart_pc_rx_buffer = Serial.readStringUntil('\n');

      if(uart_pc_rx_buffer.substring(0,2) == "IC")
      {
        setval_str = uart_pc_rx_buffer.substring(3);
        setval = setval_str.toFloat();
        setval = constrain(setval,-15,15);
        setval = mapfloat(setval,-15,15,0,127);
        spi_val = (uint8_t)setval;
        rdig_send(spi_val, SPI_CS_RDIG_IC_PIN);
        Serial.println("OK");
      }
      else if(uart_pc_rx_buffer.substring(0,2) == "R1")
      {
        setval_str = uart_pc_rx_buffer.substring(3);
        setval = setval_str.toFloat();
        setval = constrain(setval,0,1);
        setval = mapfloat(setval,0,1,0,127);
        spi_val = (uint8_t)setval;
        rdig_send(spi_val, SPI_CS_RDIG_1_PIN);
        Serial.println("OK");
      }
      else if(uart_pc_rx_buffer.substring(0,2) == "R2")
      {
        setval_str = uart_pc_rx_buffer.substring(3);
        setval = setval_str.toFloat();
        setval = constrain(setval,0,1);
        setval = mapfloat(setval,0,1,0,127);
        spi_val = (uint8_t)setval;
        rdig_send(spi_val, SPI_CS_RDIG_2_PIN);
        Serial.println("OK");  
      }
      else if(uart_pc_rx_buffer.substring(0,3) == "PWL")
      {
        Serial1.println(uart_pc_rx_buffer.substring(3));
        uart_pwl_rx_buffer = Serial1.readStringUntil('\n');

        
      }
      else if(uart_pc_rx_buffer.substring(0,8) == "RECSTART")
      {
        setval_str = uart_pc_rx_buffer.substring(9);
        setval = setval_str.toFloat();
        period = (unsigned long)((1/setval) * 1000000);
        record_ctrl = RECORD_EN;
        //Serial.println(period);
        Serial.println("OK");
        delay(500);
      }
      else if(uart_pc_rx_buffer == "RECSTOP")
      {
        record_ctrl = RECORD_DIS;
        Serial.println("OK");
        delay(500);
      }
      else if(uart_pc_rx_buffer == "DIS")
      {
        remote_ctrl = REMOTE_DIS;
        Serial.println("OK");
      }
    }

    if((record_ctrl == RECORD_EN) && ((micros() - last_rec_time) > period))
    {
      adc_read(SPI_CS_AD_XY_PIN, &ad_value[0], &ad_value[1]);
      adc_read(SPI_CS_AD_ZW_PIN, &ad_value[2], &ad_value[3]);

      Serial.print(ad_value[0]);
      Serial.print(',');
      Serial.print(ad_value[1]);
      Serial.print(',');
      Serial.print(ad_value[2]);
      Serial.print(',');
      Serial.println(ad_value[3]);

      last_rec_time = micros();
    }
  }

  serialFlush();

  display_update(print_disconnected);
  delay(blink_time);
}

void display_update(function_p func_to_print)
{
  display_core.firstPage();
  do {
    func_to_print();
  }while(display_core.nextPage());
}

void print_init()
{
  display_core.setFont(u8g_font_courB14);
  display_core.drawStr(20,15," Hybrid ");
  display_core.drawStr(20,35,"computer");
  display_core.drawStr(30,55," v1.0");  
}

void print_menu()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(25,13,"Main menu"); 
  display_core.setFont(u8g_font_courR08);
  display_core.drawStr(5,23,"A - set init. cond.");
  display_core.drawStr(5,33,"B - set dig. pot.");
  display_core.drawStr(5,43,"C - PWL block");
  display_core.drawStr(5,53,"D - activate record");
  display_core.drawStr(5,63,"0 - connect with PC");
}

void print_wrong_key()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(40,25,"Wrong");
  display_core.drawStr(40,40," key");
}

void print_pwl()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(10,20,"PWL block is");
  display_core.drawStr(10,35,"available in");
  display_core.drawStr(10,50,"  PC app.");
}

void print_recstart()
{
  display_core.setFont(u8g_font_courB08);
  display_core.drawStr(25,10,"Recording");

  display_core.setFont(u8g_font_courB08);
  display_core.setPrintPos(25,30);
  display_core.print("  ACTIVE");

  display_core.setFont(u8g_font_u8glib_4);
  display_core.setPrintPos(5,55);
  display_core.drawStr(5,55,"* - START, # - STOP");
  display_core.drawStr(5,60,"A - back to menu");
}

void print_recstop()
{
  display_core.setFont(u8g_font_courB08);
  display_core.drawStr(25,10,"Recording");
  
  display_core.setFont(u8g_font_courB08);
  display_core.setPrintPos(25,30);
  display_core.print("INACTIVE");

  display_core.setFont(u8g_font_u8glib_4);
  display_core.setPrintPos(5,55);
  display_core.drawStr(5,55,"* - START, # - STOP");
  display_core.drawStr(5,60,"A - back to menu");
}

void print_nonconnected()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(7,25," Waiting for");
  display_core.drawStr(7,40,"PC response...");
}

void print_connected()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(20,35,"Connected.");
}

void print_disconnected()
{
  display_core.setFont(u8g_font_courB10);
  display_core.drawStr(10,35,"Disconnected.");
}

void rdig_send(uint8_t data_to_send, uint8_t device)
{
  digitalWrite(device, LOW);
  
  SPI.transfer(data_to_send);
  
  digitalWrite(device, HIGH);
}

void adc_config(void)
{
  control_reg = AD7322_CONTROL_REG_SEL | AD7322_CTRL_ADD0_V1 | AD7322_CTRL_MODE_SINGLE_ENDED
               | AD7322_CTRL_PWRMODE_NORMAL_MODE | AD7322_CTRL_CODING_TWO_COMP | AD7322_CTRL_REF_INT | AD7322_CTRL_SEQ_ENABLE;
  range_reg = AD7322_RANGE_REG_SEL | AD7322_RNGE_V0_BIPOLAR_10V | AD7322_RNGE_V1_BIPOLAR_10V;

  digitalWrite(SPI_CS_AD_XY_PIN, LOW);
//pressed_key = keypad_core.waitForKey();
  SPI.transfer(range_reg);
//pressed_key = keypad_core.waitForKey();
  SPI.transfer(control_reg);
//pressed_key = keypad_core.waitForKey();
  digitalWrite(SPI_CS_AD_XY_PIN, HIGH);
//pressed_key = keypad_core.waitForKey();
  digitalWrite(SPI_CS_AD_ZW_PIN, LOW);
//pressed_key = keypad_core.waitForKey();
  SPI.transfer(range_reg);
//pressed_key = keypad_core.waitForKey();
  SPI.transfer(control_reg);
//pressed_key = keypad_core.waitForKey();
  digitalWrite(SPI_CS_AD_ZW_PIN, HIGH);
//pressed_key = keypad_core.waitForKey();
}

void adc_read(uint8_t device, int16_t * result_0, int16_t * result_1)
{
  uint16_t res_0;
  uint16_t res_1;
  
  digitalWrite(device, LOW);

  res_0 = SPI.transfer(AD7322_NONE_REG_SEL);
  res_1 = SPI.transfer(AD7322_NONE_REG_SEL);

  digitalWrite(device, HIGH);

  *result_0 = (int16_t)(res_0 << 4);
  *result_1 = (int16_t)(res_1 << 4);

  *result_0 = *result_0 / 16;
  *result_0 = *result_1 / 16;
}

void initial_conditions()
{
  bool    quit        = false;
  bool    set         = false;
  float   setval      = 0;
  uint8_t spi_val     = 0;
  String  setval_str;
  char    p_ic        = 0;

  do{

    display_core.firstPage();
    do {
      display_core.setFont(u8g_font_courB08);
      display_core.drawStr(13,10,"Initial conditions");

      display_core.setFont(u8g_font_courR08);
      display_core.setPrintPos(10,30);
      display_core.print("Value: ");
      display_core.setFont(u8g_font_courB08);
      display_core.print(setval_str);
      display_core.print(" V");

      if(set)
      {
        display_core.setFont(u8g_font_courB08);
        display_core.setPrintPos(50,45);
        display_core.print(" SET");
      }
      else
      {
        display_core.setFont(u8g_font_courB08);
        display_core.setPrintPos(50,45);
        display_core.print("UNSET");
      }

      display_core.setFont(u8g_font_u8glib_4);
      display_core.setPrintPos(5,55);
      display_core.drawStr(5,55,"* - set value, # - ., B - clear,");
      display_core.drawStr(5,60,"A - back to menu, D - minus");
     
    }while(display_core.nextPage());

    if(set == true)
    {
      setval_str = String("");
      set = false;
    }

    pressed_key = keypad_core.waitForKey();
    set = false;

    switch(pressed_key)
    {
      case'*':
        set = true;

        setval = setval_str.toFloat();
        setval = constrain(setval,-15,15);
        setval_str = String(setval, 3);
        setval = mapfloat(setval,-15,15,0,127);
        spi_val = (uint8_t)setval;

        rdig_send(spi_val, SPI_CS_RDIG_IC_PIN);
      break;

      case'#':
        setval_str = String(setval_str + '.');
      break;

      case'A':
        quit = true;
      break;

      case'B':
        setval_str = String("");
      break;
      
      case'C':
        display_update(print_wrong_key);
        delay(blink_time);
      break;
      
      case'D':
        setval_str = String('-' + setval_str); 
      break;

      default:
        setval_str = String(setval_str + pressed_key);
      break;
    }
    
  }while(!quit);
}

void rdig()
{
  bool    quit        = false;
  bool    set         = false;
  float   setval      = 0;
  uint8_t spi_val     = 0;
  String  setval_str;
  char    p_setval    = 0;
  uint8_t choose_pot  = 0;
  uint8_t choose_pot_print  = 0;

  do {

    display_core.firstPage();
    do {
      display_core.setFont(u8g_font_courB08);
      display_core.setPrintPos(10,10);
      display_core.print("Dig. potentiometer");
      display_core.print("s");

      display_core.setFont(u8g_font_courR08);
      display_core.drawStr(25,25,"  Select the");
      display_core.drawStr(25,35,"potentiometer:");
      display_core.drawStr(5,50,"A - RDIG1");
      display_core.drawStr(70,50,"B - RDIG2");
      
    }while(display_core.nextPage());

    pressed_key = keypad_core.waitForKey();

    if(pressed_key == 'A')
    {
      choose_pot = SPI_CS_RDIG_1_PIN;
      choose_pot_print  = 1;
      quit = true;
    }
    else if(pressed_key == 'B')
    {
      choose_pot = SPI_CS_RDIG_2_PIN;
      choose_pot_print  = 2;
      quit = true;
    }
    else
    {
      display_update(print_wrong_key);
      delay(blink_time);
    }
  }while(!quit);

  quit = false;

  do{

    display_core.firstPage();
    do{
      display_core.setFont(u8g_font_courB08);
      display_core.setPrintPos(7,10);
      display_core.print("Dig. potentiometer");
      display_core.print(" ");
      display_core.print(choose_pot_print);
      

      display_core.setFont(u8g_font_courR08);
      display_core.setPrintPos(10,30);
      display_core.print("Value: ");
      display_core.setFont(u8g_font_courB08);
      display_core.print(setval_str);
      
      if(set)
      {
        display_core.setFont(u8g_font_courB08);
        display_core.setPrintPos(50,45);
        display_core.print(" SET");
      }
      else
      {
        display_core.setFont(u8g_font_courB08);
        display_core.setPrintPos(50,45);
        display_core.print("UNSET");
      }

      display_core.setFont(u8g_font_u8glib_4);
      display_core.setPrintPos(5,55);
      display_core.drawStr(5,55,"* - set value, # - ., B - clear,");
      display_core.drawStr(5,60,"A - back to menu, D - minus");
     
    }while(display_core.nextPage());

    if(set == true)
    {
      setval_str = String("");
      set = false;
    }

    pressed_key = keypad_core.waitForKey();
    set = false;

    switch(pressed_key)
    {
      case'*':
        set = true;

        setval = setval_str.toFloat();
        setval = constrain(setval,0,1);
        setval_str = String(setval, 3);
        setval = mapfloat(setval,0,1,0,127);
        spi_val = (uint8_t)setval;

        rdig_send(spi_val, choose_pot);
      break;

      case'#':
        setval_str = String(setval_str + '.');
      break;

      case'A':
        quit = true;
      break;

      case'B':
        setval_str = String("");
      break;
      
      case'C':
        display_update(print_wrong_key);
        delay(blink_time);
      break;
      
      case'D':
        setval_str = String('-' + setval_str); 
      break;

      default:
        setval_str = String(setval_str + pressed_key);
      break;
    }
    
  }while(!quit);
}

void record()
{
  bool quit = false;
  bool set = false;
  int16_t ad_value[4];

  unsigned long last_rec_time = micros();
  
  display_update(print_recstop);

  while(!quit)
  {
    pressed_key = keypad_core.getKey();

    if(pressed_key != NO_KEY)
    {
      if(pressed_key == '*')
      {
        set = true;
        display_update(print_recstart);
      }
      else if(pressed_key == '#')
      {
        set = false;
        display_update(print_recstop);
      }
      else if(pressed_key == 'A')
      {
        set = false;
        quit = true;
      }
    }

    if(set && ((micros() - last_rec_time) > 20))
    {
      adc_read(SPI_CS_AD_XY_PIN, &ad_value[0], &ad_value[1]);
      adc_read(SPI_CS_AD_ZW_PIN, &ad_value[2], &ad_value[3]);

      Serial.print(ad_value[0]);
      Serial.print(',');
      Serial.print(ad_value[1]);
      Serial.print(',');
      Serial.print(ad_value[2]);
      Serial.print(',');
      Serial.println(ad_value[3]);

      last_rec_time = micros();
    }
  }
}

void setup()
{
  display_update(print_init);
  
  Serial.begin(UART_PC_BAUDRATE);
  //Serial1.begin(UART_PWL_BAUDRATE);

  Serial.setTimeout(1000);
  //Serial1.setTimeout(1000);

  serialFlush();
  //serial1Flush();

  SPI.begin();

  pinMode(SPI_CS_RDIG_IC_PIN, OUTPUT);
  pinMode(SPI_CS_RDIG_1_PIN,  OUTPUT);
  pinMode(SPI_CS_RDIG_2_PIN,  OUTPUT);
  pinMode(SPI_CS_AD_XY_PIN,   OUTPUT);
  pinMode(SPI_CS_AD_ZW_PIN,   OUTPUT);

  digitalWrite(SPI_CS_RDIG_IC_PIN,  HIGH);
  digitalWrite(SPI_CS_RDIG_1_PIN,   HIGH);
  digitalWrite(SPI_CS_RDIG_2_PIN,   HIGH);
  digitalWrite(SPI_CS_AD_XY_PIN,    HIGH);
  digitalWrite(SPI_CS_AD_ZW_PIN,    HIGH);

  adc_config();

  stm_state = STM_STATE_INIT;
}

void loop()
{  
  switch (stm_state)
  {
    case STM_STATE_INIT:
      
      /*do
      {
        if(Serial1.available() > 0)
        {
          uart_pwl_rx_buffer = Serial1.readStringUntil(';');
        } 
      }while(!uart_pwl_rx_buffer.equals("PWL_OK;"));*/

      delay(blink_time);

      stm_state = STM_STATE_MAIN;

    break;

    case STM_STATE_MAIN:

      display_update(print_menu);

      pressed_key = keypad_core.waitForKey();

      switch(pressed_key)
      {
        case 'A':
          stm_state = STM_STATE_IC;
        break;

        case 'B':
          stm_state = STM_STATE_RDIG;
        break;

        case 'C':
          stm_state = STM_STATE_PWL;
        break;

        case 'D':
          stm_state = STM_STATE_RECORD;
        break;

        case '0':
          stm_state = STM_STATE_REMOTE;
        break;

        default:
          display_update(print_wrong_key);
          delay(blink_time);
        break;
      }

    break;

    case STM_STATE_IC:
      initial_conditions();
      stm_state = STM_STATE_MAIN;
    break;

    case STM_STATE_RDIG:
      rdig();
      stm_state = STM_STATE_MAIN;
    break;

    case STM_STATE_PWL:
      display_update(print_pwl);
      delay(blink_time);
      stm_state = STM_STATE_MAIN;
    break;

    case STM_STATE_RECORD:
      record();
      stm_state = STM_STATE_MAIN;
    break;
    
    case STM_STATE_REMOTE:
      remote();      
      stm_state = STM_STATE_MAIN;
    break;

    default:

    break;
  }
}
