/***********************************************************
 * DMXProjectorController                                   *
 * Developed by HalfdanJ.dk                                 *
 ************************************************************/

#include <SoftwareSerial.h>
#include "EEPROM.h"

//Pin assignment
#define SerialRxPin 8
#define SerialTxPin 9
#define ledPin 6

//LED Status
int receiveDmxTimeout = 0;
int receiveDmxBlinkTimer = 0;
boolean blinkState = false;
boolean fastBlink = false;


//Send states
boolean lastShutterSend = false;
boolean lastOnSend = false;
boolean lastOffSend = false;

//Serial
SoftwareSerial mySerial =  SoftwareSerial(SerialRxPin, SerialTxPin);

//DMX Setup
#define NUMBER_OF_CHANNELS 255 //512

#define SWITCH_PIN_0 11 //the pin number of our "0" switch
#define SWITCH_PIN_1 12 //the pin number of our "1" switch

//DMX Address
unsigned int dmxaddress = 1;


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
volatile byte i = 0;              //dummy variable for dmxvalue[]
volatile byte dmxreceived = 0;    //the latest received value
volatile unsigned int dmxcurrent = 0;     //counter variable that is incremented every time we receive a value.
volatile byte dmxvalue[NUMBER_OF_CHANNELS];
/*  stores the DMX values we're interested in using--
 *  keep in mind that this is 0-indexed. */
volatile boolean dmxnewvalue = false;
/*  set to 1 when updated dmx values are received
 *  (even if they are the same values as the last time). */

//Timer2 variable declarations
volatile byte zerocounter = 0;
/* a counter to hold the number of zeros received in sequence on the serial receive pin.
 *  When we've received a minimum of 11 zeros in a row, we must be in a break.  */


void setup() {
  pinMode(SerialRxPin, INPUT);
  pinMode(SerialTxPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(6, OUTPUT);
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

  /******************************* USART configuration ************************************/

  Serial.begin(250000);
  /* Each bit is 4uS long, hence 250Kbps baud rate */

  cli(); //disable interrupts while we're setting bits in registers

  bitClear(UCSR0B, RXCIE0);  //disable USART reception interrupt

  /******************************* Timer2 configuration ***********************************/

  //NOTE:  this will disable PWM on pins 3 and 11.
  bitClear(TCCR2A, COM2A1);
  bitClear(TCCR2A, COM2A0); //disable compare match output A mode
  bitClear(TCCR2A, COM2B1);
  bitClear(TCCR2A, COM2B0); //disable compare match output B mode
  bitSet(TCCR2A, WGM21);
  bitClear(TCCR2A, WGM20);  //set mode 2, CTC.  TOP will be set by OCRA.

  bitClear(TCCR2B, FOC2A);
  bitClear(TCCR2B, FOC2B);  //disable Force Output Compare A and B.
  bitClear(TCCR2B, WGM22);  //set mode 2, CTC.  TOP will be set by OCRA.
  bitClear(TCCR2B, CS22);
  bitClear(TCCR2B, CS21);
  bitSet(TCCR2B, CS20);   // no prescaler means the clock will increment every 62.5ns (assuming 16Mhz clock speed).

  OCR2A = 64;
  /* Set output compare register to 64, so that the Output Compare Interrupt will fire
   *  every 4uS.  */

  bitClear(TIMSK2, OCIE2B);  //Disable Timer/Counter2 Output Compare Match B Interrupt
  bitSet(TIMSK2, OCIE2A);    //Enable Timer/Counter2 Output Compare Match A Interrupt
  bitClear(TIMSK2, TOIE2);   //Disable Timer/Counter2 Overflow Interrupt Enable          

  sei();                     //reenable interrupts now that timer2 has been configured. 

}  //end setup()

void loop()  {
  // the processor gets parked here while the ISRs are doing their thing. 

  if (dmxnewvalue == 1) {    //when a new set of values are received, jump to action loop...
    action(); //Here is the fun!
    dmxnewvalue = 0;
    dmxcurrent = 0;
    zerocounter = 0;      //and then when finished reset variables and enable timer2 interrupt
    i = 0;
    bitSet(TIMSK2, OCIE2A);    //Enable Timer/Counter2 Output Compare Match A Interrupt
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
      digitalWrite(ledPin,blinkState);   
      if(fastBlink){
        receiveDmxBlinkTimer = 12000;  
      } 
      else { 
        receiveDmxBlinkTimer = 32000;  
      }
    } 
  } 
  else {
    blinkState = true;
    digitalWrite(ledPin,blinkState);
  }
} //end loop()

//Timer2 compare match interrupt vector handler
ISR(TIMER2_COMPA_vect) {
  if (bitRead(PIND, PIND0)) {  // if a one is detected, we're not in a break, reset zerocounter.
    zerocounter = 0;
  }
  else {
    zerocounter++;             // increment zerocounter if a zero is received.
    if (zerocounter == 20)     // if 20 0's are received in a row (80uS break)
    {
      bitClear(TIMSK2, OCIE2A);    //disable this interrupt and enable reception interrupt now that we're in a break.
      bitSet(UCSR0B, RXCIE0);
    }
  }
} //end Timer2 ISR

ISR(USART_RX_vect){
  dmxreceived = UDR0;
  dmxcurrent++;                        //increment address counter

  if(dmxcurrent > dmxaddress) {         //check if the current address is the one we want.
    dmxvalue[i] = dmxreceived;
    i++;
     //zuuuuuuuuu6aaaaaaaauaaaaaauuxsuxssssuxus$$$$$$$$$jx1jiiiiiiiiiiiii0 mySerial.println(i);
    if(i == NUMBER_OF_CHANNELS) {
      bitClear(UCSR0B, RXCIE0);
      dmxnewvalue = 1;                        //set newvalue, so that the main code can be executed.
    }
  }
} // end ISR








