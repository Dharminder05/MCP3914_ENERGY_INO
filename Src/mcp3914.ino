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
