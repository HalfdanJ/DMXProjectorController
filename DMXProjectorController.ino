/***********************************************************
 * DMXProjectorController                                   *
 * Developed by HalfdanJ.dk                                 *
 ************************************************************/

/* Compile error? http://doityourselfchristmas.com/forums/showthread.php?20062-arduino-on-WIFI-with-vixen-drivers-for-g35-lights/page
 The file that has to be edited is the hardware/arduino/cores/arduino/HardwareSerial.cpp
 Basically you have to comment out from just after the store_char procedure until just before the serialEventRun procedure.
 */


#include <SoftwareSerial.h>
#include "EEPROM.h"

//Pin assignment
#define SerialRxPin 8
#define SerialTxPin 9
#define ledPin 6
#define ledPin2 7

boolean lock = 0;

//LED Status
int receiveDmxTimeout = 0;
long long receiveDmxBlinkTimer = 0;
boolean blinkState = false;
boolean fastBlink = false;

int dmxAddress = 0;

//Send states
boolean lastShutterSend = false;
boolean lastOnSend = false;
boolean lastOffSend = false;

//Serial
SoftwareSerial mySerial =  SoftwareSerial(SerialRxPin, SerialTxPin);

//DMX Setup
#define NUMBER_OF_CHANNELS 3 //512

#define SWITCH_PIN_0 11 //the pin number of our "0" switch
#define SWITCH_PIN_1 12 //the pin number of our "1" switch


//MAX485 variable declarations
#define RECEIVER_OUTPUT_ENABLE 2
/* receiver output enable (pin2) on the max485.
 *  will be left low to set the max485 to receive data. */

#define DRIVER_OUTPUT_ENABLE 3
/* driver output enable (pin3) on the max485.
 *  will left low to disable driver output. */

#define DMXRxPin 0   // serial receive pin, which takes the incoming data from the MAX485.
#define DMXTxPin 1   // serial transmission pin


//DMX variable declarations
volatile byte dmxvalue[NUMBER_OF_CHANNELS+1];
/*  stores the DMX values we're interested in using--
 *  keep in mind that this is 0-indexed. */
volatile boolean dmxnewvalue = false;
/*  set to 1 when updated dmx values are received
 *  (even if they are the same values as the last time). */


void setup() {
  pinMode(SerialRxPin, INPUT);
  pinMode(SerialTxPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  pinMode(5, OUTPUT);
  mySerial.begin(9600);

  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);

  pinMode(18, INPUT);
  digitalWrite(18, HIGH);

  /******************************* Max485 configuration ***********************************/

  pinMode(RECEIVER_OUTPUT_ENABLE, OUTPUT);
  pinMode(DRIVER_OUTPUT_ENABLE, OUTPUT);
  digitalWrite(RECEIVER_OUTPUT_ENABLE, LOW);
  digitalWrite(DRIVER_OUTPUT_ENABLE, LOW);

  pinMode(DMXRxPin, INPUT);  //sets serial pin to receive data

  /******************************* Addressing subroutine *********************************/

  pinMode(SWITCH_PIN_0, INPUT);           //sets pin for '0' switch to input
  digitalWrite(SWITCH_PIN_0, HIGH);       //turns on the internal pull-up resistor for '0' switch pin
  pinMode(SWITCH_PIN_1, INPUT);           //sets pin for '1' switch to input
  digitalWrite(SWITCH_PIN_1, HIGH);       //turns on the internal pull-up resistor for '1' switch pin


  /******************************* DMX Config *********************************/

  // initialize UART for DMX 
  // this will be 250 kbps, 8 bits, no parity, 2 stop bits 
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);

  Serial.begin(250000); 


  dmxAddress = readAddress();
}  //end setup()




void loop()  {

  
  if (dmxnewvalue == 1) {    //when a new set of values are received, jump to action loop...
    action(); //Here is the fun!

    dmxnewvalue = 0;
  }


  //As long as DMX is received blink quickly, if command blink slowly
  if(receiveDmxTimeout > 0){

    receiveDmxTimeout --;
    receiveDmxBlinkTimer --;

    if(receiveDmxBlinkTimer < 0){
      

      
      if(blinkState){
        blinkState = false;
      } 
      else {
        blinkState = true;
      }
      digitalWrite(ledPin2,blinkState);   
      analogWrite(ledPin,120*blinkState*fastBlink);   
      if(fastBlink){
        receiveDmxBlinkTimer = 22000;  
      } 
      else { 
        receiveDmxBlinkTimer = 32000;  
      }
    } 
  } 
  else {
    blinkState = true;
    digitalWrite(ledPin2,0);
    digitalWrite(ledPin,blinkState);
  }
} //end loop()





enum 
{ 
  DMX_IDLE, 
  DMX_BREAK, 
  DMX_START, 
  DMX_RUN 
}; 

volatile unsigned char dmx_state = DMX_IDLE; 


// this is the current address of the dmx frame 
unsigned int dmx_addr; 

// this is used to keep track of the channels 
unsigned int chan_cnt; 

// tell us when to update the pins 
volatile unsigned char update = 0; 

ISR(USART_RX_vect) 
{ 

  unsigned char status = UCSR0A; 
  unsigned char data = UDR0; 

  switch (dmx_state) 
  { 
  case DMX_IDLE: 
    if (status & (1<<FE0)) 
    { 

      if(!lock){
        dmx_addr = 0; 
        dmx_state = DMX_BREAK; 
        update = 1; 
        //     digitalWrite(ledPin2,HIGH);   
      }
    } 
    break; 

  case DMX_BREAK: 

    if (data == 0) 
    { 

      dmx_state = DMX_START; 

      //     digitalWrite(ledPin2,LOW);   

    } 
    else 
    { 
      dmx_state = DMX_IDLE; 
    } 
    break; 

  case DMX_START: 
    dmx_addr++; 
    if (dmx_addr == dmxAddress) 
    { 
      //   digitalWrite(ledPin2,HIGH);   
      chan_cnt = 0; 
      dmxvalue[chan_cnt++] = data; 
      dmx_state = DMX_RUN; 
    } 
    break; 

  case DMX_RUN: 
    dmxvalue[chan_cnt++] = data; 
    if (chan_cnt >= NUMBER_OF_CHANNELS) 
    { 
      dmx_state = DMX_IDLE; 
      //     digitalWrite(ledPin2,LOW);   
      dmxnewvalue = 1;

    } 
    break; 

  default: 
    dmx_state = DMX_IDLE; 
    break; 
  } 


} 



















