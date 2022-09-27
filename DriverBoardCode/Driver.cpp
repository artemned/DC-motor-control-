
/*
 * DriverBoard.cpp
 *
 * Created: 30.07.2022 14:01:40
 * Author : ArmNeko
 */ 
#include <Arduino.h>
#include <LiquidCrystalRus.h>
#include "Setup.h"
#include <avr/interrupt.h>



volatile bool start_click=0;//counter up number of occurrences
volatile bool back_click=0;  //counter up number of occurrences
volatile bool stop_click=0; //counter up number of occurrences

volatile bool comparatorState;// Comparator state value
bool oldComparatorState; 

volatile bool startState,backState,stopState; //Button state value
bool oldStartState,oldBackState,oldStopState;

volatile int8_t seconds=0;

LiquidCrystalRus lcd(10);
AnalogToDigitalConverter  adcConverter;
PwmTimer myPWMtimer;

enum Mode{WORK,EMERGENCY,SHORTCIRCUIT}myMode=WORK;

 volatile enum struct Motor
 {
   BACK=0,
   START,
   STOP
 }motor;

ISR(PCINT2_vect)
{ 
	 asm volatile("NOP");
	 
  if(bit_is_clear(READ_PIN_D,SB_FORD))
  {
    startState=true;
    stop_click=false;
    back_click=false;
	  
  }else if(bit_is_clear(READ_PIN_D,SB_STOP))
  {
    stopState=true;
    start_click=false;
    back_click=false;
	
  }else if(bit_is_clear(READ_PIN_D,SB_BACK))
  {
    backState=true;
    start_click=false;
    stop_click=false;
  }
    asm volatile("NOP");      
}

ISR(TIMER1_OVF_vect)
{ 
  TCNT1+=0xC2F6;
  seconds++;
}

ISR(ANALOG_COMP_vect)
{
  ACSR=0x00;
  clear_bit(TCCR1A,COM1A1);
  clear_bit(TCCR2A,COM2B1);
  clear_bit(DIRECTION_PORTS_B,HIN_FORWARD);
  clear_bit(DIRECTION_PORTS_D,HIN_BACK); 
  OCR1A=0;
  OCR2B=0; 
  motor=Motor::STOP;
  bit_is_clear(ANALOG_COMP_STATE_REGISTR,ACI);
  comparatorState = true;     
  myMode=comparatorState ? EMERGENCY : WORK;                 
}


int main(void)
{
  sei();
  setPins();
  setInterrupt();
  motor=Motor::STOP;
 
    while (1) 
    {
/////////////////Press buttons//////////////////////////////   
   
  if(startState)// Button start
 {
   if(oldStartState==0)
   {
    motor=Motor::START;
    oldStartState=true;
    oldStopState=false;
    oldBackState=false;
   }
            
      startState=false;   
 
 }
  else if(stopState)// Button stop
 {
  if(oldStopState==0)
  {
     motor=Motor::STOP;
     oldStopState=true;
     oldStartState=false;
     oldBackState=false;
  }
   stopState=false;
 }
  else if(backState) //Button revers
 {
  if(oldBackState==0)
  {
     motor=Motor::BACK;
     oldBackState=true;
     oldStartState=false;
     oldStopState=false;     
  }
   backState=false;
 }
   
/////////////////////////////////////////////////////////  
   



/////////////////Programm main tread///////////////////////////////////////
        
   switch(myMode)
{

 case EMERGENCY:
 {
   myPWMtimer.pwmOff();
   adcConverter.setOnlyAnalog();
   myPWMtimer.setTimer();
   lcd.setCursor(0,0);
   lcd.print("Аварийный режим!");
   lcd.setCursor(0,1);
   lcd.print("перегрузка...");
   //lcd.print("OverLoad...");
   //lcd.scrollDisplayLeft();
   seconds=0;
   while(seconds!=1){;}; 
   float operatingVoltage = adcConverter.readVoltage(myPWMtimer.pwm_state);
   if(operatingVoltage >= MAXIMUM_VALUE)
   {
     myMode=SHORTCIRCUIT;	 
   }
   else 
   {
	 seconds=0;
	 stop_click=false;
	 start_click=false;
	 back_click=false;
	 oldStartState=false;
	 oldStopState=false;
	 oldBackState=false;
	 while(seconds!=5){;};  
     myMode=WORK;
	 bit_is_clear(ANALOG_COMP_STATE_REGISTR,ACI);
     comparatorState=false;
	 lcd.clear();
     lcd.setCursor(4,0);
     lcd.print("Проверьте");
     lcd.setCursor(4,1);
     lcd.print("шпиндель!");
	 //lcd.setCursor(4,0);
	 //lcd.print("Cheack");
	 //lcd.setCursor(4,1);
	 //lcd.print("spindle!");
	 seconds=0;
	 while(seconds!=5){;};
   }
  
 break;
}
 case SHORTCIRCUIT:
 {

    shutDonw();
    lcd.clear();
    lcd.setCursor(7,0);
    lcd.print("Короткое замыкание...");
	//lcd.print("Short circuit!");
    lcd.setCursor(7,1);
    lcd.print("Устраните причину!");
	//lcd.print("Eliminate the cause!")
    while(1)
    {
   	  _delay_ms(500);
   	  lcd.scrollDisplayLeft();
    }	 
   break;
 }
 case WORK:
 { 
     myPWMtimer.setPwm();
    switch(static_cast<uint8_t>(motor))
   {
         
     case static_cast<uint8_t>(Motor::START):
   {
      if(!start_click)
     {
       myPWMtimer.setState((uint8_t)Motor::START);
       myPWMtimer.pwmOn(comparatorState);
	   adcConverter.pullUpCompPin(myPWMtimer.pwm_state);
       set_bit(CONFIGURATION_PORTS_D,F_LED);
       start_click=true;
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Прямое вращение");
       lcd.setCursor(0,1);
       lcd.print("скорость:");
     //lcd.setCursor(0,0);
     //lcd.print("Forward rotation");
     //lcd.setCursor(2,1);
     //lcd.print("Speed:"); 
     }
     while(start_click)
     {
       lcd.setCursor(9,1);
       adcConverter.setADC(myPWMtimer.pwm_state); 
       lcd.print(adcConverter.procValue);
	   if(adcConverter.procValue < 10)
	   {
		lcd.setCursor(10,1);
		lcd.print("  ");   
	   }
       lcd.setCursor(12,1);
       lcd.print('%');  
       if(comparatorState)break;
     }
     clear_bit(CONFIGURATION_PORTS_D,F_LED);   
     lcd.clear(); 
    break;         
     }
     case static_cast<uint8_t>(Motor::BACK):
   {
     if(!back_click)
     {
      myPWMtimer.setState((uint8_t)Motor::BACK);
      myPWMtimer.pwmOn(comparatorState);
	  adcConverter.pullUpCompPin(myPWMtimer.pwm_state);
      set_bit(CONFIGURATION_PORTS_B,B_LED);
      back_click=true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Обратное вращен.");
      lcd.setCursor(0,1);
      lcd.print("скорость:");
      //lcd.setCursor(2,0);
      //lcd.print("Back rotation");
      //lcd.setCursor(2,1);
      //lcd.print("speed:");
     }
     while(back_click)
     {    
      lcd.setCursor(9,1);
      adcConverter.setADC(myPWMtimer.pwm_state);
      lcd.print(adcConverter.procValue);
     if(adcConverter.procValue < 10)
       {
	       lcd.setCursor(10,1);
	       lcd.print("  ");
       }
       lcd.setCursor(12,1);
       lcd.print('%');
      if(comparatorState)break;
     }
     clear_bit(CONFIGURATION_PORTS_B,B_LED);
     lcd.clear();
     break;
     }
     case static_cast<uint8_t>(Motor::STOP):
     {       
    if(!stop_click)
    {
      myPWMtimer.pwmOff(); 
	  adcConverter.setOnlyAnalog();
      set_bit(CONFIGURATION_PORTS_D,S_LED);
      stop_click=true;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Шпиндель выкл!");
      lcd.setCursor(0,1);
      lcd.print("скорость:");
	  //lcd.print("Spindle off!");
	  //lcd.print("speed:");
    }
    while(stop_click)
    {  
      lcd.setCursor(9,1);
      adcConverter.readOnlyPOt();
      lcd.print(adcConverter.procValue);
      if(adcConverter.procValue < 10)
       {
	       lcd.setCursor(10,1);
	       lcd.print("  ");
       }
       lcd.setCursor(12,1);
       lcd.print('%');
      if(comparatorState)break;
    }
    clear_bit(CONFIGURATION_PORTS_D,S_LED);
    lcd.clear();
      break;
   }
   
    default:break;
         
   }
 }
               
  default:break;
}

}//while

}
