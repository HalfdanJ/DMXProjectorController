void action() { 

  receiveDmxTimeout = 32000;
  /*********** Put what you want the code to do with the values (dmxvalue) here *************
   * example code: print out the received values to the serial port, and set PWM pins 5 and 6 
   to the first two values received.  You can take this code out and put your own in.*/

  //begin example code

  //analogWrite(6,400);
  /* analogWrite(5, dmxvalue[0]);
   analogWrite(6, dmxvalue[1]);*/
  //end example code
  /*
  mySerial.print(readAddress());
   mySerial.print("  ");
   mySerial.print(dmxvalue[2+readAddress()]);
   mySerial.print("  ");
   mySerial.print(dmxvalue[2+readAddress()+1]);
   mySerial.print("  ");
   mySerial.print(dmxvalue[2+readAddress()+2]);
   mySerial.print(" \n ");*/
  fastBlink = false;

  if(dmxvalue[2+readAddress()] > 127) {
    fastBlink = true;
    if(lastShutterSend == false){
      lastShutterSend = true;

      //Video Mute
      mySerial.print('C');
      mySerial.print('0');
      mySerial.print('D');
      mySerial.print('\r');
      mySerial.print('\n'); 
    }
  } 
  else {
    if(lastShutterSend == true){
      lastShutterSend = false;

      //Video Mute Off
      mySerial.print('C');
      mySerial.print('0');
      mySerial.print('E');
      mySerial.print('\r');
            mySerial.print('\n'); 
    }
  }

  if(dmxvalue[2+readAddress()+1] > 127) {
    fastBlink = true;
    if(lastOnSend == false){
      lastOnSend = true;

      //Turn on
      mySerial.print('C');
      mySerial.print('0');
      mySerial.print('0');
      mySerial.print('\r');
            mySerial.print('\n'); 

    }
  } 
  else {
    lastOnSend = false;
  }
  if(dmxvalue[2+readAddress()+2] > 127) {
    fastBlink = true;
    if(lastOffSend == false){
      lastOffSend = true;

      //Turn off
      mySerial.print('C');
      mySerial.print('0');
      mySerial.print('1');
      mySerial.print('\r');
    }
  }
  else {
    lastOffSend = false;
  }




  return;  //go back to loop()
} //end action() loop










