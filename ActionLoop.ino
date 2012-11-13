void action() { 
  lock = true;

  receiveDmxTimeout = 32000;



  fastBlink = false;

  if(dmxvalue[0] > 127) {
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

  if(dmxvalue[1] > 127) {
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
  if(dmxvalue[2] > 127) {
    fastBlink = true;
    if(lastOffSend == false){
      lastOffSend = true;

      //Turn off
      mySerial.print('C');
      mySerial.print('0');
      mySerial.print('1');
      mySerial.print('\r');
      mySerial.print('\n'); 
    }
  }
  else {
    lastOffSend = false;
  }


  lock = false;

  return;  //go back to loop()
} //end action() loop














