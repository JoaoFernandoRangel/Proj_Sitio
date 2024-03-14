



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Antes delay");
  delay(10000);
  Serial.println("Depois delay");
  ESP.restart() ;
}
