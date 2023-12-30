int leitura = 0;
bool botao_apertado = 0;


void setup()
{
  Serial.begin(9600);
  pinMode(5, OUTPUT);// Led Verde
  pinMode(18, OUTPUT); // Led Vermelho
  pinMode(34, INPUT); //Leitura potenciometro
  pinMode(32, INPUT_PULLUP); // Leitura Botao
}

void loop() {
  leitura = analogRead(34);
  if (!digitalRead(32)){
    botao_apertado = !botao_apertado;
    delay(200);
  } else{
    botao_apertado = 0;
  }
  if (leitura > 2045 ) {
   digitalWrite(5, HIGH);
   digitalWrite(18, LOW);
  }else if (leitura <= 2045 ){
   digitalWrite(18, HIGH);
   digitalWrite(5, LOW);
 }
}




