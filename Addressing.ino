  void setMux(int val){
  digitalWrite(14,val & 1);
  digitalWrite(15,val & 2);
  digitalWrite(16, val & 4);
  digitalWrite(17, val & 8);
}

int readMux(int val){
  setMux(val);
 // delay(1);
  return analogRead(4);
}

int rotaryRead(int rot){
  int val = 4*rot;

  int b1 = (readMux(val) > 500)?0 : 1;
  int b2 = (readMux(val+1)> 500)?0 : 1;
  int b3 = (readMux(val+2)> 500)?0 : 1;
  int b4 = (readMux(val+3)> 500)?0 : 1;

  return b1*4 + b2*8 + b3 + b4*2;
  ;

}

int readAddress(){
  return rotaryRead(2) + 10*rotaryRead(1) + 100*rotaryRead(0) ;
}

