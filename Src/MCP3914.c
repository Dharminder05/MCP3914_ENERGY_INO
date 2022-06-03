#include "Arduino.h"
#include "SPI.h"
#include "MCP3914.h"
MCP3914 :: MCP3914(){
//Constructor
}

//Initializes SPI and saves pin variables
void MCP3914 :: begin(int _CLOCK_PIN, int _CS_PIN)
{
	CLOCK_PIN = _CLOCK_PIN;
	CS_PIN = _CS_PIN;
	
	pinMode(CS_PIN, OUTPUT);  	//Define CS-Pin as output
	digitalWrite(CS_PIN, HIGH);
  
	SPI.begin();
	//Use the maximum serial clock frequency of 20MHz, MSB first mode and
	//- SPI_MODE0 for low clock idle, output edge falling, data capture rising
	//- SPI_MODE3 for high clock idle, output edge falling, data capture rising
	SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
}

//Generates 4MHZ-Clock at pin 0C1A. 
void MCP3914 :: generate_CLK(void)
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
void MCP3914::configure(REGISTER_SETTINGS _settings)
{
	settings = _settings;							//Load settings into private variable
	
	//Put all ADC's into reset mode to enable a configuration in one write-cycle.
	digitalWrite(CS_PIN,LOW);
	SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
	SPI.transfer(0b11111111);                      //Write RESET<23:16> to '11111111' to reset all ADC's
	digitalWrite(CS_PIN,HIGH);

	//Start write-cycle and configure all necessary registers
	digitalWrite(CS_PIN,LOW);
	SPI.transfer(ADDR_BITS | (MCP3914_PHASE0 << 1));     //Control Byte: Choose phase register to start cycling through all registers.
	SPI.transfer16(settings.PHASE0);                //Write PHASE0-Register
	SPI.transfer16(settings.PHASE0);
	SPI.transfer16(settings.PHASE1);  		//Write PHASE1-Register
	SPI.transfer16(settings.PHASE1); 
	
	SPI.transfer((settings.PGA_CH7 << 5)|(settings.PGA_CH6 << 2)|(settings.PGA_CH5)); //Write GAIN-Register      
	SPI.transfer((settings.PGA_CH5 << 7)|(settings.PGA_CH4 << 4)|(settings.PGA_CH3 << 1)|(settings.PGA_CH2));
	SPI.transfer((settings.PGA_CH2 << 6)|(settings.PGA_CH1 << 3)|(settings.PGA_CH0));
	
	SPI.transfer16((settings.READ << 14)|         //Write STATUSCOM-Register
		   	(settings.WRITE << 13)|
                   	(settings.DR_HIZ << 12)|
                   	(settings.DR_LINK << 11)|
                   	(settings.WIDTH_CRC << 10)|
                   	(settings.WIDTH_DATA << 8)|
                   	(settings.EN_CRCCOM << 7)|
                   	(settings.EN_INT << 6));  
        SPI.transfer(settings.DRSTATUS);
	
	SPI.transfer16((settings.EN_OFFCAL << 15)|	//Write CONFIG0-Register
                   	(settings.EN_GAINCAL << 14)|
                   	(settings.DITHER << 12)|
                   	(settings.BOOST << 10)|
                   	(settings.PRE << 8)|
                   	(settings.OSR << 5));
	SPI.transfer(settings.VREFCAL);
	
	SPI.transfer(settings.RESET);			//Write CONFIG1-Register
	SPI.transfer(settings.SHUTDOWN);	
	SPI.transfer((settings.VREFEXT << 7)|(settings.CLKEXT << 6));                                              	
	digitalWrite(CS_PIN,HIGH);
}

//Read chosen channel, convert it to a readable form and return it. 
//Function only usable in 24-bit mode.
float MCP3914::read_chX(uint8_t channel)
{
	digitalWrite(CS_PIN, LOW);
	SPI.transfer(ADDR_BITS | (channel << 1) | 1); //Control Byte
	
	byte upper = SPI.transfer(0x00);			   //Read the three bytes
	byte middle = SPI.transfer(0x00);
	byte lower = SPI.transfer(0x00);
	
	digitalWrite(CS_PIN, HIGH);
	
	//Concat the three bytes.
	//When I cast the three bytes to long, they will be shifted with respect to their sign. 
	//The back-shift by 8 bit at the end makes sure that the sign is at bit 32 instead of bit 24.
	long combined_value = ((((long)upper << 24) | ((long)middle << 16) | (long)lower<<8) >> 8); 
 
    //Conversion to readable form according to datasheet
	float voltage = data_to_voltage(combined_value, channel);
	
	return voltage;
}


//Read chosen channel and return raw data. 
//Function only usable in 24-bit mode.
long MCP3914::read_raw_data(uint8_t channel)
{
	digitalWrite(CS_PIN, LOW);
	SPI.transfer(ADDR_BITS | (channel << 1) | 1); //Control Byte
	
	byte upper = SPI.transfer(0x00);			   //Read the three bytes
	byte middle = SPI.transfer(0x00);
	byte lower = SPI.transfer(0x00);
	
	digitalWrite(CS_PIN, HIGH);
	
	//Concat the three bytes.
	//When I cast the three bytes to long, they will be shifted with respect to their sign. 
	//The back-shift by 8 bit at the end makes sure that the sign is at bit 32 instead of bit 24.
	long combined_value = ((((long)upper << 24) | ((long)middle << 16) | (long)lower<<8) >> 8); 
	
	return combined_value;
}


//Takes a 24-bit value and calculates voltage from it.
float MCP3914::data_to_voltage(long data, uint8_t channel)
{
	uint8_t gain = 0;
	
	if(channel == MCP3914_CH0)
		gain = settings.PGA_CH0;
	else if(channel == MCP3914_CH1)
		gain = settings.PGA_CH1;
	else if(channel == MCP3914_CH2)
		gain = settings.PGA_CH2;
	else if(channel == MCP3914_CH3)
		gain = settings.PGA_CH3;
	else if(channel == MCP3914_CH4)
		gain = settings.PGA_CH4;
	else if(channel == MCP3914_CH5)
		gain = settings.PGA_CH5;
	else if(channel == MCP3914_CH6)
		gain = settings.PGA_CH6;
	else if(channel == MCP3914_CH7)
		gain = settings.PGA_CH7;
	else{
		Serial.println("Entered wrong register at data_to_voltage-function");
		return 0;
	}
	
	//Depending on which gain was chosen for the channel, the conversion is different
	switch(gain){
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
	float voltage = (data * 1.2)/(8388608*1.5*gain);  	//Conversion according to datasheet.
	return voltage;
}



//Enter reset mode on all Channels
void MCP3914::enter_reset_mode(void)
{
// 	uint32_t value_conf2 = read_register(MCP3914_CONFIG1); //Read configuration of MCP3914_CONFIG1, so nothing gets lost
	//Put both ADC's into reset mode
    digitalWrite(CS_PIN,LOW);
    SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
//     SPI.transfer(0b11000000 | value_conf2);
    SPI.transfer(0b11111111);            //Write RESET<23:16> to '11111111' to reset both ADC's
    digitalWrite(CS_PIN,HIGH);
}

//Exit reset mode on all Channels
void MCP3914::exit_reset_mode(void)
{
// 	uint32_t value_conf2 = read_register(MCP3914_CONFIG1); //Read configuration of MCP3914_CONFIG1, so nothing gets lost	
	//Exit both ADC's from reset mode
    digitalWrite(CS_PIN,LOW);
    SPI.transfer(ADDR_BITS | (MCP3914_CONFIG1 << 1));  //Control Byte: Choose MCP3914_CONFIG1
//     SPI.transfer(0b00111111 & value_conf2); 
    SPI.transfer(0b00000000);		//Write RESET<23:16> to '00000000' to exit all ADC's reset mode
    digitalWrite(CS_PIN,HIGH);
}

//Write given offset to offset register any of CH0 or CH1 or CH2 or CH3 or CH4 or CH5 or CH6 or CH7
void MCP3914::write_offset(long offset, uint8_t channel)
{
	digitalWrite(CS_PIN, LOW);
	SPI.transfer(ADDR_BITS | (channel << 1)); //Control Byte
	SPI.transfer(offset >> 16);
	SPI.transfer(offset >> 8);
	SPI.transfer(offset >> 0);
	digitalWrite(CS_PIN, HIGH);
}	

//Read a register and return its value
uint8_t MCP3914::read_register(uint8_t reg)
{
	digitalWrite(CS_PIN, LOW);
	SPI.transfer(ADDR_BITS | (reg << 1) | 1); //Control Byte
	byte result = SPI.transfer(0x00);			   
	digitalWrite(CS_PIN, HIGH);
	
	return result;
}
