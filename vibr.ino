//Precisa apertar 'boot' for fazer upload do código, na parte de 'connecting'
#include <Arduino.h>
#include <NTPClient.h>
#include "String.h"
#include "time.h"
#include "PubSubClient.h" // MQTT de fato
#include <WiFi.h>
#include <WiFiClientSecure.h>


// Define a struct to hold WiFi configuration
struct WifiConfig {
  const char* SSID;
  const char* PASS;
};

// Instantiate instances of the struct for each network
struct WifiConfig rede = {
  .SSID = "VIVO GUI",
  .PASS = "123456789"
};
// Create an array of WifiConfig structs
struct WifiConfig wifiVector[] = {
  rede, 
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

//Define os IO's
#define pin_sensor 34
#define pin_buzzer 26


//Define informacoes MQTT
#define SERVER "cca032668b6b42828366174dc5fecf41.s1.eu.hivemq.cloud"
#define SERVERPORT 8883
#define user "esp32"
#define pass "Guilherme1"
// Cria os clientes MQTT
PubSubClient mqtt(client);


// prototipos de funcao
void reconnect();
static const char* root_ca PROGMEM = R"EOF(
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
String topico = "idle", topico1 = "status";
void handleMessage(String receivedMessage) {
  // 9 + 2 -> 12 item, 11 identificador
  if (receivedMessage[11] == '1'){
    digitalWrite(pin_buzzer, HIGH);
    Serial.println("Buzzer acionado");
  }else{
    digitalWrite(pin_buzzer, LOW);
    Serial.println("Buzzer desligado");
  }
}
void callback(char* topico1, byte* payload, unsigned int length = 21) {
  Serial.print("Message received on topic: ");
  Serial.println(topico1);
  String receivedMessage = "";
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }
  handleMessage(receivedMessage);
}

bool primeiro_post;

void setup() {  //Iniciando
  Serial.begin(9600);
  Serial.println("Inicio");

  mqtt.setCallback(callback);
  primeiro_post = true;
  // Conecta o Wifi na rede
  for (size_t ji = 0; ji < sizeof(wifiVector) / sizeof(wifiVector[0]); ++ji) {
    connectToWiFi(wifiVector[ji], 15000);  // Timeout set to 15 seconds (15000 milliseconds)
    // Check if connected to WiFi
    if (WiFi.status() == WL_CONNECTED) {
      break;  // Exit the loop if connected successfully
    }
  }
  //Conecta servidor MQTT
  client.setCACert(root_ca);
  mqtt.setServer(SERVER, SERVERPORT);
  reconnect();

  //Inicia NTP para adquirir data e hora
  ntp.begin();
  ntp.setTimeOffset(-10800);  //corrige para fuso horário
                              // Inicia o sensor de temperatura
                              //dht.begin();
  ntp.update();
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_sensor, INPUT);
}
int contador;
void reconnect() {
  //Rotina de conexao
  while (!mqtt.connected()) {
    //digitalWrite(LedMqtt, LOW);
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "Esp32";
    clientId += String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str(), user, pass)) {
      Serial.println("conectado");
      contador = 0;
      mqtt.subscribe("controle", 0);  // inscricao no topico 'controle'
    } else {
      Serial.print("falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 5 segundos");
      //wait(1000, 5);
      contador++;
      Serial.println(contador);
    }
    if (contador == 15) {
      ESP.restart();
    }
  }
  //digitalWrite(LedMqtt, HIGH);
}



void reconnectToWiFi() {
  // Tenta reconectar-se a uma rede WiFi
  for (size_t ji = 0; ji < sizeof(wifiVector) / sizeof(wifiVector[0]); ++ji) {
    connectToWiFi(wifiVector[ji], 15000);  // Timeout definido como 15 segundos (15000 milissegundos)

    // Verifica se está conectado ao WiFi
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi reconnected successfully.");
      return;  // Sai da função se reconectar com sucesso
    }
  }
  // Se não conseguir se reconectar a nenhuma rede, aguarde um curto período antes de tentar novamente
  Serial.println("Failed to reconnect to WiFi. Retrying in 5 seconds...");
  delay(5000);  
}
unsigned long interacao_com_mqtt, agora, antes=0;
String msg_inicio = "hello";

String idle = "_idle", espaco = "@@";
String info_vibracao, pre_analise;
//Mensagem padrão = Data&Hora @@ info_vibração @@ pré-análise
bool vibracao;
void processa_info(bool condicao){
  if (condicao){
    info_vibracao = "1";
    pre_analise = "Vibrando";
  } else{
    info_vibracao = "0";
    pre_analise = "Estável";
  }
  mqtt.publish(topico1.c_str(), (ntp.getFormattedTime() + espaco + info_vibracao + espaco + pre_analise).c_str());
}

unsigned long leitura = 0, freq_leitura = 1;
void loop() {
  agora = millis();
  if (agora - leitura >= (1000/(freq_leitura))){    
  vibracao = digitalRead(pin_sensor);    
  processa_info(vibracao);
  leitura = agora;
  }
  if (!mqtt.connected()) {
    //deu_ruim();
    reconnect();  
  }
  if (primeiro_post == true) {
    mqtt.publish(topico.c_str(), msg_inicio.c_str());
    Serial.println(msg_inicio);
    primeiro_post = false;
  }
  // Poll the MQTT client to check for incoming messages
  mqtt.loop(); // Vai para o callback
  
  if (agora - antes >= 5000) {
    String tempo = ntp.getFormattedTime();
    mqtt.publish(topico.c_str(), (tempo + idle).c_str());
    antes = agora;
    Serial.println("Mensagem Publicada");
  }
  // Add any other logic or delay if needed
  ntp.update();
  delay(500);  // Adjust the delay according to your needs
}
