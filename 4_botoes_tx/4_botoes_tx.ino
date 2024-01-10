//Precisa apertar 'boot' for fazer upload do código, na parte de 'connecting'

//#include <bits/cpu_defines.h>
#include <Arduino.h>
#include <NTPClient.h>
#include "String.h"
#include "time.h"
#include "PubSubClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>


// Define a struct to hold WiFi configuration
  struct WifiConfig {
      const char* SSID;
      const char* PASS;
  };

// Instantiate instances of the struct for each network
  struct WifiConfig bia2Config = {
    .SSID = "Bia 2",
    .PASS = "coisafacil"
  };

  struct WifiConfig casaPiscinaConfig = {
    .SSID = "CS_TELECOM_CS96",
    .PASS = "cs2017cs3337"
  };

  struct WifiConfig vivofibraConfig = {
    .SSID = "VIVOFIBRA-09D8",
    .PASS = "816329FCDE"
  };

  struct WifiConfig casaLuConfig = {
    .SSID = "VIVOFIBRA-DB00",
    .PASS = "55228E47BB"
  };

  struct WifiConfig escritorioConfig = {
    .SSID = "Escritorio",
    .PASS = "cs2017cs3337"
  };

// Create an array of WifiConfig structs
  struct WifiConfig wifiVector[] = {
      bia2Config,
      casaPiscinaConfig,
      vivofibraConfig,
      casaLuConfig,
      escritorioConfig
  };

void connectToWiFi(const struct WifiConfig& config, unsigned long timeoutMillis) {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(config.SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(config.SSID, config.PASS);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeoutMillis) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("Failed to connect to WiFi within the timeout. Moving to the next network.");
    }
}

// Cria um WiFiClient class para utilizar no MQTT server.
WiFiClientSecure client;
//Cria uma instância para medição de tempo
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);

//Define informacoes MQTT
#define SERVER      "25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud"
#define SERVERPORT   8883 
#define user        "TX_Esp32"
#define pass        "Campos0102"
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

bool primeiro_post;
String tempo, string_pub;
void setup() {
  //Iniciando  
    Serial.begin(9600);
    Serial.println("Inicio");
    primeiro_post = true;
  // declara os pinos de saida e entrada
   //pinMode(34, INPUT_PULLUP); // botao 1
   pinMode(32, INPUT_PULLUP); // botao 1 
   pinMode(33, INPUT_PULLUP); // botao 2
   pinMode(27, INPUT_PULLUP); // botao 3
   pinMode(26, INPUT_PULLUP); // botao 4
   
   
   
   pinMode(18, OUTPUT);  // led 1
   pinMode(5, OUTPUT);   // led 2
   pinMode(17, OUTPUT);  // led 3
   pinMode(16, OUTPUT);  // led 4
   
   
   
   pinMode(15, OUTPUT); //led para conexão mqtt, se aceso conectado 


   wait(500,2);  
  // Conecta o Wifi na rede
    for (size_t ji = 0; ji < sizeof(wifiVector) / sizeof(wifiVector[0]); ++ji) {
           connectToWiFi(wifiVector[ji], 15000); // Timeout set to 15 seconds (15000 milliseconds)

           // Check if connected to WiFi
           if (WiFi.status() == WL_CONNECTED) {
               break; // Exit the loop if connected successfully
           }
       }

  //Conecta servidor MQTT
   client.setCACert(root_ca);
   mqtt.setServer(SERVER, SERVERPORT);
   reconnect();
   delay(500);
  //Inicia NTP para adquirir data e hora
   ntp.begin();
   ntp.setTimeOffset(-10800);//corrige para fuso horário
}

bool toggle1 = false, toggle11 = false, toggle2 = false,toggle3 = false,toggle4 = false;
String string_padrao = "10%20%30%40";
String payload;
String msg_inicio = "inicio_tx";
String space = "-";
//int porta_ligada, porta_desligada;
int tempo_de_publi;
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  if (primeiro_post == true){
    mqtt.publish("idle_tx", msg_inicio.c_str());
    Serial.println(msg_inicio);
    tempo_de_publi = millis();
    primeiro_post = false;
  }

  
  //lógica do botao 1
  if (!digitalRead(32) && toggle1 == false){
    string_padrao[1] = '1';
    tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());    
    tempo_de_publi = millis();
    Serial.print("botao 1 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle");  
    digitalWrite(18, HIGH); 
    delay(200);
    toggle1 = !toggle1;
  } else if (!digitalRead(32) && toggle1 == true){
    string_padrao[1] = '0';
    tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());    
    tempo_de_publi = millis();
    Serial.print("botao 1 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle"); 
    digitalWrite(18, LOW);   
    delay(200);
    toggle1 = !toggle1;     
  }

  // Lógica do botao 2
  if (!digitalRead(33) && toggle2 == false){
    string_padrao[4] = '1';
     tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());     
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle");  
    digitalWrite(5, HIGH); 
    delay(200);
    toggle2 = !toggle2;
  } else if (!digitalRead(33) && toggle2 == true){
    string_padrao[4] = '0';
     tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());     
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle"); 
    digitalWrite(5, LOW);   
    delay(200);
    toggle2 = !toggle2;     
  }
  // Lógica do botao 3
  if (!digitalRead(27) && toggle3 == false){
    string_padrao[7] = '1';
     tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());     
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle");  
    digitalWrite(17, HIGH); 
    delay(200);
    toggle3 = !toggle3;
  } else if (!digitalRead(27) && toggle3 == true){
    string_padrao[7] = '0';
     tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());    
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle"); 
    digitalWrite(17, LOW);   
    delay(200);
    toggle3 = !toggle3;     
  }
    // Lógica do botao 4
  if (!digitalRead(26) && toggle4 == false){
    string_padrao[10] = '1';
    tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());     
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle");  
    digitalWrite(16, HIGH); 
    delay(200);
    toggle4 = !toggle4;
  } else if (!digitalRead(26) && toggle4 == true){
    string_padrao[10] = '0';
    tempo = ntp.getFormattedTime();
    string_pub = string_padrao + space + tempo;
    mqtt.publish("controle", string_pub.c_str());    
    tempo_de_publi = millis();
    Serial.print("botao 2 apertado-");
    Serial.print(string_padrao);
    Serial.println("-publicado em controle"); 
    digitalWrite(16, LOW);   
    delay(200);
    toggle4 = !toggle4;     
  }
  



  int tempo_idle = millis();
  if (tempo_idle - tempo_de_publi == 5000){//impede a desconexão ao broker
    String idle = "TX-ping";
    tempo = ntp.getFormattedTime();
    string_pub = idle + space + string_padrao + space + tempo;    
    mqtt.publish("idle_tx", string_pub.c_str()); 
    Serial.print(idle);
    Serial.println(" publicado em idle_tx");
    tempo_de_publi = millis();
  }
  ntp.update();
  string_pub = "";
}


void reconnect() {
  //Rotina de conexao
  while (!mqtt.connected()) {
    digitalWrite(15, LOW);
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "Esp32";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), user, pass)) {
      Serial.println("conectado");

      mqtt.subscribe("controle", 0);   // inscricao no topico 'controle'
    } else {
      Serial.print("falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 5 segundos");
      wait(1000,5);
    }
  }
  digitalWrite(15, HIGH);}

void wait(int ciclo, int num){ //ciclo -> tempo do ciclo, num -> numero de repeticoes, LED -> porta led  
  for(int i = 0; i < num; i++)
  {
    //digitalWrite(LED, HIGH);
    delay( int(ciclo/2) );
    //digitalWrite(LED, LOW);
    delay( int(ciclo/2) );
  }}



