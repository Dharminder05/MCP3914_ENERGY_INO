/*
  MCP3914.h - Library for the MCP3914.
  Version 1.0
  Created by Dharminder, May 10, 2018.
*/

#ifndef MCP3914_h
#define MCP3914_h

#include "Arduino.h"
#include "SPI.h"

//MCP3914 Register Map
#define ADDR_BITS       	0x01            //Hardware adress of ADC.

#define MCP3914_CH0     	0x00
#define MCP3914_CH1     	0x01
#define MCP3914_CH2     	0x02
#define MCP3914_CH3     	0x03
#define MCP3914_CH4     	0x04
#define MCP3914_CH5     	0x05
#define MCP3914_CH6     	0x06
#define MCP3914_CH7     	0x07

#define MCP3914_MOD     	0x08
#define MCP3914_PHASE0   	0x09
#define MCP3914_PHASE1   	0x0A
#define MCP3914_GAIN    	0x0B
#define MCP3914_STATCOM 	0x0C
#define MCP3914_CONFIG0 	0x0D
#define MCP3914_CONFIG1 	0x0E
#define MCP3914_OFF0    	0x0F
#define MCP3914_GC0     	0x10
#define MCP3914_OFF1    	0x11
#define MCP3914_GC1     	0x12
#define MCP3914_OFF2    	0x13
#define MCP3914_GC2     	0x14
#define MCP3914_OFF3    	0x15
#define MCP3914_GC3     	0x16
#define MCP3914_OFF4    	0x17
#define MCP3914_GC4     	0x18
#define MCP3914_OFF5    	0x19
#define MCP3914_GC5     	0x1A
#define MCP3914_OFF6    	0x1B
#define MCP3914_GC6     	0x1C
#define MCP3914_OFF7    	0x1D
#define MCP3914_GC7     	0x1E
#define MCP3914_LOCK    	0x1F

struct REGISTER_SETTINGS{
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
	uint8_t READ;             
    	uint8_t WRITE;        
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


class MCP3914
{
	public:
		MCP3914();
	    void begin(int _CLOCK_PIN, int _CS_PIN);
		void generate_CLK(void);
		void configure(REGISTER_SETTINGS setting);
		float read_chX(uint8_t channel);
	    long read_raw_data(uint8_t channel);
		float data_to_voltage(long data, uint8_t channel);
		void enter_reset_mode();
		void exit_reset_mode();
		void write_offset(long offset, uint8_t channel);
		uint8_t read_register(uint8_t reg);
	
	private:
		int CLOCK_PIN;
		int CS_PIN;
		REGISTER_SETTINGS settings;
};

#endif
