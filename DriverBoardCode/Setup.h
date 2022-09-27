/*
 * Setup.h
 *
 * Created: 31.07.2022 19:43:36
 *  Author: ArtNeko
 */ 

#pragma  once
#include "pinDefines.h"

#define  sei()  __asm__ __volatile__ ("sei" ::: "memory")
#define  cli()  __asm__ __volatile__ ("cli" ::: "memory")

using REGISTER = uint16_t; 
 float filValue=0;
 static float koef = 1.0;
int filterEx(float value)
{
  filValue+=(value-filValue)*koef;
  return filValue;	
}

float CONTAINER[4];
const uint8_t numerals=4;
uint8_t count=0;

uint16_t averageValue(float* value)
{
	float sum=0;
	for(uint8_t i=0;i<numerals;i++)
	{
	  sum+=CONTAINER[i];	  	
	}	
	return filterEx ((float(sum)/numerals));
}


inline void setInterrupt(void)
{
  CONFIGURATION_PORTS_D |=(_BV(SB_FORD) | _BV(SB_STOP) | _BV(SB_BACK)); // Enable pull up input
  DIRECTION_PORTS_D &= ((~_BV(SB_FORD)) | (~_BV(SB_STOP)) | (~_BV(SB_BACK))); //Set pins as input
  set_bit(PCICR,PCIE2);                                  //Enable interrupt on PD pins
  PCMSK2 |= (_BV(SB_FORD) | _BV(SB_BACK) | _BV(SB_STOP));//Enable interrupt on corresponding pins
}



inline void setPins(void)
{
   /*Setup board pins for beginning initialization*/
  set_bit(DIRECTION_PORTS_D,F_LED);       //Pin as out for led
  set_bit(DIRECTION_PORTS_D,S_LED);       //Pin as out for led
  set_bit(DIRECTION_PORTS_B,B_LED);       //Pin as out for led
  set_bit(CONFIGURATION_PORTS_B,D_PIN);   // Enable pull up for avoid tri-state
  set_bit(DIGITAL_INPUT_DISABLE_1,AIN0D);
  DIDR0 =0x30;//For reduce power consumption in the digital input buffer
  
}
inline void shutDonw(void)
{
  CONTROL_STATE_REGISTER_A=0x00;
  ANALOG_COMP_STATE_REGISTR=0x00;
  CONTROL_STATE_REGISTER_B=0x00;
  DIRECTION_PORTS_B=0x2C;//Set as pins input besides pins for SPI
  DIRECTION_PORTS_D=0x00;
  DIRECTION_PORTS_C=0x00;
  CONFIGURATION_PORTS_D=0xFF;// Pull-up enable
  CONFIGURATION_PORTS_B=0xFF;// Pull-up enable
  CONFIGURATION_PORTS_C=0xFF;// Pull-up enable
  PCICR=0x00;    //Disable interrupt
  PCMSK2=0x00;   //Disable interrupt                
}


struct PwmTimer
{
    enum PWMstate{FORWARD=0,REVERS}pwm_state;
        
  REGISTER r_timer[4]={0x0000,0x0005,0x0001,0xC2F6};
  REGISTER r_pwm[3]={0x0001,0x0001,0x0000};//Set work`s mode (PWM, Phase Correct, 8-bit),Setup prescaler clk/(1*2*255)=31372.54 HG
    /*list of register*/
    //TCCRNN
    //TIMSK1
    //TCNT1

 void setTimer(void)
{ 
  TCCR1A=r_timer[0];
  TCCR1B=r_timer[1];
  TIMSK1=r_timer[2];
  TCNT1 =r_timer[3];  
}
 void setPwm(void)
{
  TCCR1A=r_pwm[0];
  TCCR1B=r_pwm[1];    
  TCCR2A=r_pwm[0];
  TCCR2B=r_pwm[1];
  TIMSK1=r_pwm[2];                
}
 void setState(const uint8_t& state_)
 {
  pwm_state = state_ ? PWMstate::FORWARD
                     : PWMstate::REVERS;
 }
 void pwmOff(void)
{
   
  char cSREG = SREG;
  cli();
  switch(pwm_state)
  {
   case PwmTimer::REVERS:
   {
     for(uint8_t decrease=OCR1A;decrease!=0;decrease--)//Smooth stop
     {                           
       OCR1A=decrease;                 
     }                           
     OCR1A=0;                      
     clear_bit(TCCR1A,COM1A1);               
     clear_bit(DIRECTION_PORTS_B,HIN_FORWARD);    
     break;
   }
   case PwmTimer::FORWARD:
   {
     for(uint8_t decrease=OCR2B;decrease!=0;decrease--)//Smooth stop
     {                           
       OCR2B=decrease;                 
     }                           
     OCR2B=0;                      
     clear_bit(TCCR2A,COM2B1);               
     clear_bit(DIRECTION_PORTS_D,HIN_BACK);           
     break;
   }
   default:break;
   
  }
  SREG=cSREG;
} 
void pwmOn(volatile bool& flagComparator)
{
  switch((int8_t)flagComparator)
 {
  case 0:{
    if((int8_t)pwm_state == (int8_t)PWMstate::FORWARD)
    {
      pwmOff();
      char cSREG = SREG;
      cli();
      set_bit(DIRECTION_PORTS_B,HIN_FORWARD);// Turn on pin
      set_bit(TCCR1A,COM1A1); //Non inverting mode
      SREG =cSREG;
      //      writeADC();
    }
    else if((int8_t)pwm_state == (int8_t)PWMstate::REVERS)
    {
      pwmOff();
      char cSREG = SREG;
      cli();
      set_bit(DIRECTION_PORTS_D,HIN_BACK);// Turn on pin
      set_bit(TCCR2A,COM2B1); // Non inverting mode
      SREG = cSREG;
    }
    break;
  }
  case 1:{
    break;
  }
  default:break;
    
  } 
} 

};

struct AnalogToDigitalConverter
{ 
  enum ADCState{ANALOG,COMPARATOR}adc_state=ANALOG;
  float voltageValue=0;
  uint16_t procValue=0;
  REGISTER r_adc[4]={0x0087,0x0000,0x0040,0x0080};
  REGISTER r_comparator[4]={0x0000,0x0040,0x0000,0x0001};

 void pullUpCompPin(PwmTimer::PWMstate& pwm_state)
 {
  CONFIGURATION_PORTS_C = static_cast<uint8_t>(pwm_state) ? 0x01
  						                                  : 0x02;
 }

float readVoltage(PwmTimer::PWMstate& pwm_state)
{
  if((uint8_t)pwm_state == (uint8_t)PwmTimer::FORWARD)
  {
    ADMUX = (ADMUX & 0xF0) | CURRENT_VALUE_1;
  }
  else if((uint8_t)pwm_state == (uint8_t)PwmTimer::REVERS)
  {
    ADMUX = (ADMUX & 0xF0) | CURRENT_VALUE_2;
  }
   CONTROL_STATE_REGISTER_A |=_BV(ADSC);
  loop_until_bit_is_clear(CONTROL_STATE_REGISTER_A,ADSC);//Wait for conversion
   return (REFERENCE_VOLTAGE*ADC)/1024;   
}


void setOnlyAnalog(void)
{
  CONTROL_STATE_REGISTER_A = r_adc[0];
  CONTROL_STATE_REGISTER_B = r_adc[1];
  MUX_SELECTION_REGISTER = r_adc[2];
  ANALOG_COMP_STATE_REGISTR = r_adc[3];
}

void readOnlyPOt(void)
{
   ADMUX = (ADMUX & 0xF0) | HAND_ADJUSTABLE;   // Bit mask clear all pin and set our pin
   CONTROL_STATE_REGISTER_A |=_BV(ADSC);
   loop_until_bit_is_clear(CONTROL_STATE_REGISTER_A,ADSC);//Wait for conversion
   voltageValue = (REFERENCE_VOLTAGE*ADC)/1024;
   procValue = (voltageValue/REFERENCE_VOLTAGE)*100;
   CONTAINER[count++] = (voltageValue/REFERENCE_VOLTAGE)*100;
   if(count==numerals)
   {
	   procValue=averageValue(CONTAINER);
	   count=0;
   }
   ADC=0x00;
}

void writeADC(PwmTimer::PWMstate& pwm_state) //Read data from analog pin
{   
   ADMUX = (ADMUX & 0xF0) | HAND_ADJUSTABLE;   // Bit mask clear all pin and set our pin
   CONTROL_STATE_REGISTER_A |=_BV(ADSC);
   loop_until_bit_is_clear(CONTROL_STATE_REGISTER_A,ADSC);//Wait for conversion
   voltageValue = (REFERENCE_VOLTAGE*ADC)/1024;
   CONTAINER[count++]=(voltageValue/REFERENCE_VOLTAGE)*100;
   if(count==numerals)
   {
	   procValue=averageValue(CONTAINER);
	   count=0;
   } 
   if((uint8_t)pwm_state == (uint8_t)PwmTimer::FORWARD)
   {
     OCR1A=(ADC>>2);
   }
   else if((uint8_t)pwm_state == (uint8_t)PwmTimer::REVERS)
   {
     OCR2B =(ADC>>2);
   }  
   ADC=0x00;
  clear_bit(CONTROL_STATE_REGISTER_A,ADIF);
}

void setADC(PwmTimer::PWMstate& pwm_state)
{   
  switch(adc_state)
  {
    case ANALOG:// ADC setup
    {
	  CONTROL_STATE_REGISTER_A = r_adc[0];
	  CONTROL_STATE_REGISTER_B = r_adc[1];
	  MUX_SELECTION_REGISTER = r_adc[2];
	  ANALOG_COMP_STATE_REGISTR = r_adc[3];
      writeADC(pwm_state);
	  adc_state=ADCState::COMPARATOR;
      break;
    }
    case COMPARATOR: //Comparator setup
    {	
	  CONTROL_STATE_REGISTER_A = r_comparator[0];
	  CONTROL_STATE_REGISTER_B = r_comparator[1];
	  MUX_SELECTION_REGISTER = static_cast<uint8_t>(pwm_state) ? r_comparator[3] //select pin for negativ input
	                                                           : r_comparator[2];
	  								   
	  ACSR &= ~(_BV(ACD));//Enable comparator
	  ACSR |= _BV(ACIE); //Enable comparator interrupt
	  ACSR |= _BV(ACIS1); //
	  ACSR |= _BV(ACIS0); // Comparator Interrupt on Rising Output Edge.
	  adc_state=ADCState::ANALOG;
      break;
    }   
     default:break;
    }
}

};

