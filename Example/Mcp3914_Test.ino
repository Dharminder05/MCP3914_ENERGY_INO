/*
  Mcp3914_Test.ino - Basic functionality sketch for the MCP3914.
  Version 1.0
  Created by Dharminder, June 1, 2022.
*/

#include <SPI.h>
#include <arduino.h>
#include <MCP3914.h>

MCP3914 mcp3914;

int CLOCK_PIN = 9;  //Pin 9 is the OC1A-Pin of the Arduino Pro Mini and goes to OSC1-Pin of the MCP3914
int CS_PIN = 8;     //Pin 8 of the Arduino Pro Mini goes to the CS-Pin of the MCP3914

void setup() {
  Serial.begin(115200);
  
  mcp3914.begin(CLOCK_PIN, CS_PIN);     //Initialize MCP3914
  mcp3914.generate_CLK();               //Generate 4MHZ clock on CLOCK_PIN

  REGISTER_SETTINGS settings = {};
  //PHASE-SETTINGS
  settings.PHASE0     = 0;           //Phase shift between CH6/CH7 and CH5/CH4 is 0
  settings.PHASE1     = 0;           //Phase shift between CH3/CH2 and CH1/CH0 is 0
  //GAIN-SETTINGS
  settings.PGA_CH7    = 0b000;      //CH7 gain is 1
  settings.PGA_CH6    = 0b000;      //CH6 gain is 1
  settings.PGA_CH5    = 0b000;      //CH5 gain is 1
  settings.PGA_CH4    = 0b000;      //CH4 gain is 1
  settings.PGA_CH3    = 0b000;      //CH3 gain is 1
  settings.PGA_CH2    = 0b000;      //CH2 gain is 1 
  settings.PGA_CH1    = 0b000;      //CH1 gain is 1
  settings.PGA_CH0    = 0b000;      //CH0 gain is 1
  //STATUSCOM-SETTINGS
  settings.READ       = 0b10;       //Adress counter loops register types//No modulator output enabled
  settings.WRITE      = 0b1;        //Adress counter loops entire register map
  settings.DR_HIZ     = 0b1;        //DR pin state is logic high when data is not ready
  settings.DR_LINK    = 0b0;        //Data ready pulses from lagging ADC are output on DR-Pin
  settings.WIDTH_CRC  = 0b0;        //CRC-16 Format on Communications bit
  settings.WIDTH_DATA = 0b01;       //All Channel are in 24bit-mode
  settings.EN_CRCCOM  = 0b0;        //Digital offset calibration on both channels disabled
  settings.EN_INT     = 0b0;        //Group delay on both channels disabled
  settings.DRSTATUS   = 0b1;
  //CONFIG-SETTINGS
  settings.EN_OFFCAL   = 0b0;       //24-Bit Digital Offset Error Calibration on All Channels Enable bit
  settings.EN_GAINCAL  = 0b0;       //24-Bit Digital Gain Error Calibration on All Channels Enable/Disable bit
  settings.DITHER      = 0b11;      //Dithering on both channels maximal
  settings.BOOST       = 0b10;      //Current boost is 1
  settings.PRE         = 0b00;      //AMCLK = MCLK
  settings.OSR         = 0b011;     //Oversampling Ratio for Delta-Sigma A/D Conversion bits
  settings.VREFCAL     = 0b0;       //Internal Voltage Temperature Coefficient VREFCAL[7:0] Value bits
  //CONFIG-SETTINGS
  settings.RESET       = 0b00000000;     //Neither ADC in Reset mode
  settings.SHUTDOWN    = 0b00000000;     //Neither ADC in Shutdown
  settings.VREFEXT     = 0b0;            //Internal Voltage Reference Selection bit
  settings.CLKEXT      = 0b0;            //Internal Clock Selection bit
  
  mcp3914.configure(settings);      //Configure the MCP3914 with the settings above
  delay(100);
}

void loop() {
  float value0 = mcp3914.read_chX(MCP3914_CH0); //Read value of CH0
  float value1 = mcp3914.read_chX(MCP3914_CH1); //Read value of CH1
  float value2 = mcp3914.read_chX(MCP3914_CH2); //Read value of CH2
  float value3 = mcp3914.read_chX(MCP3914_CH3); //Read value of CH3
  float value4 = mcp3914.read_chX(MCP3914_CH4); //Read value of CH4
  float value5 = mcp3914.read_chX(MCP3914_CH5); //Read value of CH5
  float value6 = mcp3914.read_chX(MCP3914_CH6); //Read value of CH6
  float value7 = mcp3914.read_chX(MCP3914_CH7); //Read value of CH7
  
  Serial.print("Voltage CH0 = ");       //Print value of CH0
  Serial.print(value0, 8);
  Serial.print(" V\n");

  Serial.print("Voltage CH1 = ");       //Print value of CH1
  Serial.print(value1, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH2 = ");       //Print value of CH2
  Serial.print(value2, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH3 = ");       //Print value of CH3
  Serial.print(value3, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH4 = ");       //Print value of CH4
  Serial.print(value4, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH5 = ");       //Print value of CH5
  Serial.print(value5, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH6 = ");       //Print value of CH6
  Serial.print(value6, 8);
  Serial.print(" V\n\n");
  
  Serial.print("Voltage CH7 = ");       //Print value of CH7
  Serial.print(value7, 8);
  Serial.print(" V\n\n");

  delay(1000);
}
