//Precisa apertar 'boot' for fazer upload do código, na parte de 'connecting'

//#include <bits/cpu_defines.h>
#include <Arduino.h>
#include "String.h"
#include "time.h"
#include "PubSubClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

//Define informacoes da rede VIVOFIBRA-09D8
#define WLAN_SSID      "VIVOFIBRA-09D8"
#define WLAN_PASS      "816329FCDE"

//redes wifi
  /*
  //Define informacoes da rede Bia 2
  #define WLAN_SSID      "Bia 2"
  #define WLAN_PASS      "coisafacil"
  */
  /*Define informacoes da rede casa piscina
  #define WLAN_SSID      "CS_TELECOM_CS96"
  #define WLAN_PASS      "cs2017cs3337"
  */

  /*
  //Define informacoes da rede da casa da Lu
  #define WLAN_SSID      "VIVOFIBRA-DB00"
  #define WLAN_PASS      "55228E47BB"
  */

  /*
  //Define informacoes da rede
  #define WLAN_SSID      "Escritorio"
  #define WLAN_PASS      "cs2017cs3337"
  */

// Cria um WiFiClient class para utilizar no MQTT server.
WiFiClientSecure client;

//Define informacoes MQTT
#define SERVER      "25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud"
#define SERVERPORT   8883 
#define user        "Campos"
#define pass        "campos0102"
// Cria os clientes MQTT
PubSubClient mqtt(client);
unsigned long lastMsg = 0;

void reconnect();
unsigned long millisZero();
void wait(int ciclo, int num);

//Variaveis
  int f = 10;                   // valor em hz
  unsigned long time_ini;       // tempo em ms que comecou o segundo no timestamp
  unsigned long refTime = 0;    // tempo de inicio do loop
  bool ativar = true;          // indica se vai rodar a transmissao

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
// declaração I/O


void setup() {
  //Iniciando  
    Serial.begin(9600);
    Serial.println("Inicio");
  // declara os pinos de saida e entrada
   pinMode(32, INPUT_PULLUP); // botao
   pinMode(34, INPUT); // leitura potenciometro
   pinMode(17, OUTPUT); // led Verde
   pinMode(18, OUTPUT); // led Vermelho
   pinMode(5, OUTPUT); // led verde 2
   pinMode(33, OUTPUT); //led para conexão mqtt, se aceso conectado 
   wait(500,2);  
  // Conecta o Wifi na rede
    Serial.println(); Serial.println();
    Serial.print("Conectando em ");
    Serial.println(WLAN_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);  

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    Serial.println("WiFi connected");
    Serial.print("IP address: "); 
    Serial.println(WiFi.localIP());

  //Conecta servidor MQTT
   client.setCACert(root_ca);
   mqtt.setServer(SERVER, SERVERPORT);
   reconnect();
   delay(500);
   }











bool toggle = false, toggle1 = false;
String string_padrao = "10%20%30";
String payload;
//int porta_ligada, porta_desligada;

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  int potValue = analogRead(34);

  // Acender de leds dependendo da leitura do potenciometro
  if (potValue >= 2730) {
    digitalWrite(17, LOW);
    digitalWrite(18, HIGH);
    digitalWrite(5, LOW);
    //Serial.println("Esquerda");
  } else if (potValue >= 1365 && potValue < 2730) {
    digitalWrite(17, LOW);
    digitalWrite(18, LOW);
    digitalWrite(5, HIGH);
    //Serial.println("Meio");
  } else {
    digitalWrite(17, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(18, LOW);
    //Serial.println("Direito");
  }

  if (!digitalRead(32) && toggle == false) {
    int valor_a_mudar = seleciona_parte(potValue);
    Serial.print("primeiro IF ");
    Serial.println(valor_a_mudar);
    toggle1 = true;

    if (string_padrao[valor_a_mudar] == '0' && toggle1 == true) {
      string_padrao[valor_a_mudar] = '1';
      Serial.print("Segundo IF ");
      Serial.println(string_padrao[valor_a_mudar]);
      toggle1 = !toggle1;
      delay(200);
    } else if (string_padrao[valor_a_mudar] == '1' && toggle1 == true) {
      string_padrao[valor_a_mudar] = '0';
      Serial.println("outro else");
      toggle1 = !toggle1;  // Corrected from toggle to toggle1
      delay(200);
    }

    payload = string_padrao;
    Serial.println(payload);
    mqtt.publish("topic", payload.c_str());
    toggle = !toggle;
    delay(200);    
  }else if (!digitalRead(32) && toggle == true){
    toggle = !toggle;
  }
}

//"10%20%30";
int seleciona_parte(int valor){
  if (valor >= 2730){
    return 1;
  } else if (valor >= 1365 && valor < 2730){
    return 4;
  } else{
    return 7;
  }
}
void reconnect() {
  //Rotina de conexao
  while (!mqtt.connected()) {
    digitalWrite(33, LOW);
    digitalWrite(25, HIGH);
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "Esp32";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), user, pass)) {
      Serial.println("conectado");

      mqtt.subscribe("topic", 0);   // inscricao no topico 'topic'
    } else {
      Serial.print("falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 5 segundos");
      wait(1000,5);
    }
  }
  digitalWrite(33, HIGH);}

void wait(int ciclo, int num){ //ciclo -> tempo do ciclo, num -> numero de repeticoes, LED -> porta led  
  for(int i = 0; i < num; i++)
  {
    //digitalWrite(LED, HIGH);
    delay( int(ciclo/2) );
    //digitalWrite(LED, LOW);
    delay( int(ciclo/2) );
  }}



