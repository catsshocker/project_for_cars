void setup() {
  pinMode(4,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  pinMode(5,OUTPUT);

  digitalWrite(5,HIGH);
  digitalWrite(16,HIGH);
  
  for(int i=0;i<=255;i++){
    analogWrite(4,i);
    delay(10);
  }
  delay(500);
  for(int i=255;i>0;i--){
    analogWrite(4,i);
    delay(10);
  }
  digitalWrite(16,HIGH);

  digitalWrite(17,HIGH);

  delay(1000);
  digitalWrite(16,LOW);
  
  for(int i=0;i<=255;i++){
    analogWrite(4,i);
    delay(6);
  }
  delay(100);
  for(int i=127;i>0;i--){
    analogWrite(4,i*2);
    delay(1);
  }
  digitalWrite(17,LOW);
  digitalWrite(5,LOW);

}

void loop() {
  // put your main code here, to run repeatedly:

}
