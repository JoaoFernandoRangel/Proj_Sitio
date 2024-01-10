//Precisa apertar 'boot' for fazer upload do código, na parte de 'connecting'

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
#define user        "RX_Esp32"
#define pass        "Campos0102"
// Cria os clientes MQTT
PubSubClient mqtt(client);
unsigned long lastMsg = 0;
// prototipos de funcao
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
int interacao_com_mqtt;
String para_idle = "00000000000";
String space = "-";
String rx_ping = "RX-ping-";
void callback(char* controle, byte* payload, unsigned int length = 21) {
  Serial.print("Message received on topic: ");
  Serial.println(controle);
  interacao_com_mqtt = millis();

 
  String receivedMessage = "";
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }
  Serial.print("Print dentro de callback: ");
  Serial.print(receivedMessage);
  Serial.println("------");
  for (int ii  = 0; ii< 11; ii++){//loop para pegar somente a parte dos comandos da string
    para_idle[ii] = receivedMessage[ii];
  }
 // para_idle = receivedMessage;
  handleMessage(receivedMessage);
}
bool primeiro_post;

void setup() {
  //Iniciando  
  Serial.begin(9600);
  Serial.println("Inicio");
  //declara os pinos de saída
    pinMode(33, OUTPUT); // Led 4
    pinMode(26, OUTPUT); // Led 3 
    pinMode(27, OUTPUT); // Led 2
    pinMode(13, OUTPUT); // Led 1
    pinMode(19, OUTPUT); // Led mqtt
    wait(500,2); 
  mqtt.setCallback(callback); 
  primeiro_post = true;
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

 //Inicia NTP para adquirir data e hora
   ntp.begin();
   ntp.setTimeOffset(-10800);//corrige para fuso horário
         
}

String msg_inicio = "inicio_rx";


void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  if (primeiro_post == true){
    mqtt.publish("idle_rx", msg_inicio.c_str());
    Serial.println(msg_inicio);
    interacao_com_mqtt = millis();
    primeiro_post = false;
  }
  
  // Poll the MQTT client to check for incoming messages
  mqtt.loop();
  int tempo_fora_do_loop = millis();
  int diferenca = tempo_fora_do_loop - interacao_com_mqtt;
  if (diferenca >= 5000){
  
    String tempo = ntp.getFormattedTime();
    String idle_ping = rx_ping + para_idle + space + tempo;
    mqtt.publish("idle_rx", idle_ping.c_str());
    Serial.print("Publicado ");
    Serial.print(idle_ping);
    Serial.println(" no tópico idle_rx.");
    diferenca = 0;
    interacao_com_mqtt = millis();
  }
  // Add any other logic or delay if needed
  ntp.update();
  delay(1000);  // Adjust the delay according to your needs
}



void reconnect() {
  //Rotina de conexao
  while (!mqtt.connected()) {
    digitalWrite(19, LOW);
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
  digitalWrite(19, HIGH);
}

void handleMessage(String receivedMessage){
Serial.println(receivedMessage);
// Check if the received message is long enough
if (receivedMessage.length() >= 8) {
  if (receivedMessage[1] == '1') {
    digitalWrite(13, HIGH);
  } else if (receivedMessage[1] == '0') {
    digitalWrite(13, LOW);
  }

  if (receivedMessage[4] == '1') {
    digitalWrite(27, HIGH);
  } else if (receivedMessage[4] == '0') {
    digitalWrite(27, LOW);
  }

  if (receivedMessage[7] == '1') {
    digitalWrite(26, HIGH);
  } else if (receivedMessage[7] == '0') {
    digitalWrite(26, LOW);
  }
  if (receivedMessage[10] == '1') {
    digitalWrite(33, HIGH);
  } else if (receivedMessage[10] == '0') {
    digitalWrite(33, LOW);
  }
} else {
  Serial.println("Received message is too short");
}

Serial.print("Received Message: ");
Serial.println(receivedMessage);
}

void wait(int ciclo, int num) //ciclo -> tempo do ciclo, num -> numero de repeticoes, LED -> porta led
{  
  for(int i = 0; i < num; i++)
  {
    //digitalWrite(LED, HIGH);
    delay( int(ciclo/2) );
    //digitalWrite(LED, LOW);
    delay( int(ciclo/2) );
  }
}



