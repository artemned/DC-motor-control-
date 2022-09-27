/********************************************************************
	created:	2022/07/01
	created:	1:7:2022   10:58
	filename: 	C:\Users\sWe\Documents\Atmel Studio\7.0\pinDefines.h
	file path:	C:\Users\sWe\Documents\Atmel Studio\7.0
	file base:	pinDefines
	file ext:	h
	author:		ArtNeko
	
	purpose:	
*********************************************************************/
#pragma once
#include <avr/io.h>
#include "Bit_manip.h"

#define F_LED PD0
#define S_LED PD1
#define B_LED PB0
#define SB_FORD PD2
#define SB_BACK PD4
#define SB_STOP PD5
#define HIN_FORWARD PB1
#define HIN_BACK PD3
#define REF_VALUE PD6
#define D_PIN PD7

#define CURRENT_VALUE_1 PC0
#define CURRENT_VALUE_2 PC1
#define HAND_ADJUSTABLE PC2
#define SENS_1 PC3
#define SENS_2 PC4

#define DIRECTION_PORTS_D DDRD
#define READ_PIN_D PIND
#define CONFIGURATION_PORTS_D PORTD

#define DIRECTION_PORTS_B DDRB
#define READ_PIN_B PINB
#define CONFIGURATION_PORTS_B PORTB 

#define DIRECTION_PORTS_C DDRC
#define READ_PIN_C PINC
#define CONFIGURATION_PORTS_C PORTC

////////////////Analog registrs////////////////////
#define CONTROL_STATE_REGISTER_A ADCSRA
#define CONTROL_STATE_REGISTER_B ADCSRB
#define ANALOG_COMP_STATE_REGISTR ACSR
#define MUX_SELECTION_REGISTER ADMUX
#define DIGITAL_INPUT_DISABLE_0 DIDR0
#define DIGITAL_INPUT_DISABLE_1 DIDR1 
/////////Voltage edge////////////////////////////
#define REFERENCE_VOLTAGE 4.93
#define MAXIMUM_VALUE 3.3



