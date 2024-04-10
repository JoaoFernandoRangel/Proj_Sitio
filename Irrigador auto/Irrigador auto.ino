#include <Arduino.h>
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);  
}
int HORA = 60*60*1000, MIN = 60*1000;

void loop() {
  digitalWrite(13, LOW);
  delay(15*MIN);
  digitalWrite(13, HIGH);
  delay(4*HORA);
}
