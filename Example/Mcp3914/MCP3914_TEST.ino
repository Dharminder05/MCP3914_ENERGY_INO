#include "SPI.h"

//MCP3914 Register Map
#define ADDR_BITS         0x01            //Hardware adress of ADC.

#define MCP3914_CH0       0x00
#define MCP3914_CH1       0x01
#define MCP3914_CH2       0x02
#define MCP3914_CH3       0x03
#define MCP3914_CH4       0x04
#define MCP3914_CH5       0x05
#define MCP3914_CH6       0x06
#define MCP3914_CH7       0x07

#define MCP3914_MOD       0x08
#define MCP3914_PHASE0    0x09
#define MCP3914_PHASE1    0x0A
#define MCP3914_GAIN      0x0B
#define MCP3914_STATCOM   0x0C
#define MCP3914_CONFIG0   0x0D
#define MCP3914_CONFIG1   0x0E
#define MCP3914_OFF0      0x0F
#define MCP3914_GC0       0x10
#define MCP3914_OFF1      0x11
#define MCP3914_GC1       0x12
#define MCP3914_OFF2      0x13
#define MCP3914_GC2       0x14
#define MCP3914_OFF3      0x15
#define MCP3914_GC3       0x16
#define MCP3914_OFF4      0x17
#define MCP3914_GC4       0x18
#define MCP3914_OFF5      0x19
#define MCP3914_GC5       0x1A
#define MCP3914_OFF6      0x1B
#define MCP3914_GC6       0x1C
#define MCP3914_OFF7      0x1D
#define MCP3914_GC7       0x1E
#define MCP3914_LOCK      0x1F

byte CLOCK_PIN = 8;
byte CS_PIN = 10;

struct REGISTER_SETTINGS {
  //PHASE-SETTINGS
  uint32_t PHASE0;
  uint32_t PHASE1;
  //GAIN-SETTINGS
  uint8_t PGA_CH7;
  uint8_t PGA_CH6;
  uint8_t PGA_CH5;
  uint8_t PGA_CH4;
  uint8_t PGA_CH3;
  uint8_t PGA_CH2;
  uint8_t PGA_CH1;
  uint8_t PGA_CH0;
  //STATUSCOM-SETTINGS
  uint8_t READ_;
  uint8_t WRITE_;
  uint8_t DR_HIZ;
  uint8_t DR_LINK;
  uint8_t WIDTH_CRC;
  uint8_t WIDTH_DATA;
  uint8_t EN_CRCCOM;
  uint8_t EN_INT;
  uint8_t DRSTATUS;
  //CONFIG0-SETTINGS
  uint8_t EN_OFFCAL;
  uint8_t EN_GAINCAL;
  uint8_t DITHER;
  uint8_t BOOST;
  uint8_t PRE;
  uint8_t OSR;
  uint8_t VREFCAL;
  //CONFIG1-SETTINGS
  uint8_t RESET;
  uint8_t SHUTDOWN;
  uint8_t VREFEXT;
  uint8_t CLKEXT;
};
REGISTER_SETTINGS settings;

void generate_CLK(void);
void configure(REGISTER_SETTINGS settings);
float read_chX(uint8_t channel);
long read_raw_data(uint8_t channel);
float data_to_voltage(long data_, uint8_t channel);
void enter_reset_mode();
void exit_reset_mode();
void write_offset(long offset, uint8_t channel);
uint8_t read_register(uint8_t reg);
/*.....................START_CODE....................................*/

void setup () {
  Serial.begin(9600);
  pinMode(CS_PIN, OUTPUT);    //Define CS-Pin as output
  digitalWrite(CS_PIN, HIGH);

  SPI.begin();
  //Use the maximum serial clock frequency of 20MHz, MSB first mode and
  //- SPI_MODE0 for low clock idle, output edge falling, data capture rising
  //- SPI_MODE3 for high clock idle, output edge falling, data capture rising
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  
  generate_CLK();               //Generate 4MHZ clock on CLOCK_PIN

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
  settings.READ_       = 0b10;       //Adress counter loops register types//No modulator output enabled
  settings.WRITE_      = 0b1;        //Adress counter loops entire register map
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
  
  configure(settings);      //Configure the MCP3914 with the settings above
  delay(100);
}

void loop() {

  float value0 = read_chX(MCP3914_CH0); //Read value of CH0
  float value1 = read_chX(MCP3914_CH1); //Read value of CH1
  float value2 = read_chX(MCP3914_CH2); //Read value of CH2
  float value3 = read_chX(MCP3914_CH3); //Read value of CH3
  float value4 = read_chX(MCP3914_CH4); //Read value of CH4
  float value5 = read_chX(MCP3914_CH5); //Read value of CH5
  float value6 = read_chX(MCP3914_CH6); //Read value of CH6
  float value7 = read_chX(MCP3914_CH7); //Read value of CH7
  
  Serial.print("Voltage CH0 = ");       //Print value of CH0
  Serial.print(value0, 8);
  Serial.print(" V\n\n");

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

/*...............................FUNCTIONS_DEFINATIONS........................................................*/

////Generates 4MHZ-Clock at pin 0C1A.
void generate_CLK(void)
{
  pinMode(CLOCK_PIN, OUTPUT);             //We can only toggle Output Compare Pin OC1A
  //Set CS1[2:0] in Timer1ControlRegisterB to '001' to count without a prescaler.
  //Set WGM1[2:0] in Timer1ControlRegisterB to '010' to use CTS operation mode.
  //(This means we only count up to to the value in the OCR1A-Register.)
  TCCR1B = (1 << CS10) | (1 << WGM12);
  TCCR1A = (1 << COM1A0);                 //Set COM1A[1:0] to '01' to toggle the OC1A-Register on Compare Match with OCR1A.
  TIMSK1 = 0;                             //Disable all Interrupts for Timer1
  OCR1A = 0;                              //Set Output Compare Register A to 0. This means we divide our internal clock by factor 2, generating 4 MHZ.
  delay(200);                             //Necessary delay, since it takes some time until the clock starts.
}

//Reset and configure the MCP3914 with user-defined settings
void configure(REGISTER_SETTINGS settings)
{
  //Put all ADC's into reset mode to enable a configuration in one write-cycle.
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
  SPI.transfer(0b11111111);                      //Write RESET<23:16> to '11111111' to reset all ADC's
  digitalWrite(CS_PIN, HIGH);

  //Start write-cycle and configure all necessary registers
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (MCP3914_PHASE0 << 1));     //Control Byte: Choose phase register to start cycling through all registers.
  SPI.transfer16(settings.PHASE0);                //Write PHASE0-Register
  SPI.transfer16(settings.PHASE0);
  SPI.transfer16(settings.PHASE1);      //Write PHASE1-Register
  SPI.transfer16(settings.PHASE1);

  SPI.transfer((settings.PGA_CH7 << 5) | (settings.PGA_CH6 << 2) | (settings.PGA_CH5)); //Write GAIN-Register
  SPI.transfer((settings.PGA_CH5 << 7) | (settings.PGA_CH4 << 4) | (settings.PGA_CH3 << 1) | (settings.PGA_CH2));
  SPI.transfer((settings.PGA_CH2 << 6) | (settings.PGA_CH1 << 3) | (settings.PGA_CH0));

  SPI.transfer16((settings.READ_ << 14) |        //Write STATUSCOM-Register
                 (settings.WRITE_ << 13) |
                 (settings.DR_HIZ << 12) |
                 (settings.DR_LINK << 11) |
                 (settings.WIDTH_CRC << 10) |
                 (settings.WIDTH_DATA << 8) |
                 (settings.EN_CRCCOM << 7) |
                 (settings.EN_INT << 6));
  SPI.transfer(settings.DRSTATUS);

  SPI.transfer16((settings.EN_OFFCAL << 15) | //Write CONFIG0-Register
                 (settings.EN_GAINCAL << 14) |
                 (settings.DITHER << 12) |
                 (settings.BOOST << 10) |
                 (settings.PRE << 8) |
                 (settings.OSR << 5));
  SPI.transfer(settings.VREFCAL);

  SPI.transfer(settings.RESET);     //Write CONFIG1-Register
  SPI.transfer(settings.SHUTDOWN);
  SPI.transfer((settings.VREFEXT << 7) | (settings.CLKEXT << 6));
  digitalWrite(CS_PIN, HIGH);
}

//Read chosen channel, convert it to a readable form and return it.
//Function only usable in 24-bit mode.
float read_chX(uint8_t channel)
{
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (channel << 1) | 1); //Control Byte

  byte upper = SPI.transfer(0x00);         //Read the three bytes
  byte middle = SPI.transfer(0x00);
  byte lower = SPI.transfer(0x00);

  digitalWrite(CS_PIN, HIGH);

  //Concat the three bytes.
  //When I cast the three bytes to long, they will be shifted with respect to their sign.
  //The back-shift by 8 bit at the end makes sure that the sign is at bit 32 instead of bit 24.
  long combined_value = ((((long)upper << 24) | ((long)middle << 16) | (long)lower << 8) >> 8);

  //Conversion to readable form according to datasheet
  float voltage = data_to_voltage(combined_value, channel);

  return voltage;
}


//Read chosen channel and return raw data.
//Function only usable in 24-bit mode.
long read_raw_data(uint8_t channel)
{
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (channel << 1) | 1); //Control Byte

  byte upper = SPI.transfer(0x00);         //Read the three bytes
  byte middle = SPI.transfer(0x00);
  byte lower = SPI.transfer(0x00);

  digitalWrite(CS_PIN, HIGH);

  //Concat the three bytes.
  //When I cast the three bytes to long, they will be shifted with respect to their sign.
  //The back-shift by 8 bit at the end makes sure that the sign is at bit 32 instead of bit 24.
  long combined_value = ((((long)upper << 24) | ((long)middle << 16) | (long)lower << 8) >> 8);

  return combined_value;
}


//Takes a 24-bit value and calculates voltage from it.
float data_to_voltage(long data_, uint8_t channel)
{
  uint8_t gain = 0;

  if (channel == MCP3914_CH0)
    gain = settings.PGA_CH0;
  else if (channel == MCP3914_CH1)
    gain = settings.PGA_CH1;
  else if (channel == MCP3914_CH2)
    gain = settings.PGA_CH2;
  else if (channel == MCP3914_CH3)
    gain = settings.PGA_CH3;
  else if (channel == MCP3914_CH4)
    gain = settings.PGA_CH4;
  else if (channel == MCP3914_CH5)
    gain = settings.PGA_CH5;
  else if (channel == MCP3914_CH6)
    gain = settings.PGA_CH6;
  else if (channel == MCP3914_CH7)
    gain = settings.PGA_CH7;
  else {
    Serial.println("Entered wrong register at data_to_voltage-function");
    return 0;
  }

  //Depending on which gain was chosen for the channel, the conversion is different
  switch (gain) {
    case 0b111: gain = 1;
      break;
    case 0b110: gain = 1;
      break;
    case 0b101: gain = 32;
      break;
    case 0b100: gain = 16;
      break;
    case 0b011: gain = 8;
      break;
    case 0b010: gain = 4;
      break;
    case 0b001: gain = 2;
      break;
    case 0b000: gain = 1;
      break;
    default:    gain = 0;
  }
  float voltage = (data_ * 1.2) / (8388608 * 1.5 * gain); //Conversion according to datasheet.
  return voltage;
}



//Enter reset mode on all Channels
void enter_reset_mode(void)
{
  //  uint32_t value_conf2 = read_register(MCP3914_CONFIG1); //Read configuration of MCP3914_CONFIG1, so nothing gets lost
  //Put both ADC's into reset mode
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
  //     SPI.transfer(0b11000000 | value_conf2);
  SPI.transfer(0b11111111);            //Write RESET<23:16> to '11111111' to reset both ADC's
  digitalWrite(CS_PIN, HIGH);
}

//Exit reset mode on all Channels
void exit_reset_mode(void)
{
  //  uint32_t value_conf2 = read_register(MCP3914_CONFIG1); //Read configuration of MCP3914_CONFIG1, so nothing gets lost
  //Exit both ADC's from reset mode
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
  //     SPI.transfer(0b00111111 & value_conf2);
  SPI.transfer(0b00000000);   //Write RESET<23:16> to '00000000' to exit all ADC's reset mode
  digitalWrite(CS_PIN, HIGH);
}

//Write given offset to offset register any of CH0 or CH1 or CH2 or CH3 or CH4 or CH5 or CH6 or CH7
void write_offset(long offset, uint8_t channel)
{
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (channel << 1)); //Control Byte
  SPI.transfer(offset >> 16);
  SPI.transfer(offset >> 8);
  SPI.transfer(offset >> 0);
  digitalWrite(CS_PIN, HIGH);
}

//Read a register and return its value
uint8_t read_register(uint8_t reg)
{
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(ADDR_BITS | (reg << 1) | 1); //Control Byte
  byte result = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);

  return result;
}
