//Precisa apertar 'boot' for fazer upload do código, na parte de 'connecting'

#include <Arduino.h>
#include <NTPClient.h>
#include "String.h"
#include "time.h"
#include "PubSubClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

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

struct WifiConfig sitioNewnet = {
  .SSID = "SITIO_NEWNET",
  .PASS = "coisafacil"
};

struct WifiConfig escritorioConfig = {
  .SSID = "Escritorio",
  .PASS = "cs2017cs3337"
};

struct WifiConfig rede_celular = {
  .SSID = "REDE",
  .PASS = "12345678"
};

// Create an array of WifiConfig structs
struct WifiConfig wifiVector[] = {
  sitioNewnet,
  bia2Config,
  rede_celular,
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

// Declarações para sensor DHT11
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Declarações das portas da esp32
#define Rele10 13
#define Rele20 27
#define Rele30 26
#define Rele40 33
#define LedMqtt 19

//Define informacoes MQTT
#define SERVER "25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud"
#define SERVERPORT 8883
#define user "RX_Esp32"
#define pass "Campos0102"
// Cria os clientes MQTT
PubSubClient mqtt(client);
unsigned long lastMsg = 0;
int contador = 0;

// prototipos de funcao
void reconnect();
unsigned long millisZero();
void wait(int ciclo, int num);

// Variaveis
int f = 10;                 // valor em hz
unsigned long time_ini;     // tempo em ms que comecou o segundo no timestamp
unsigned long refTime = 0;  // tempo de inicio do loop
bool ativar = true;         // indica se vai rodar a transmissao

// Variáveis para operação automática
int horas_auto = 4, minutos_auto = 15;
unsigned long agora, tempo_auto = 0;
bool liga_auto, auto_enable = HIGH;


// Variáveis genéricas
unsigned long lastToggleTime = 0, inicio;
bool port33State = HIGH;                                       // Initial state (HIGH or LOW)
bool primeiro_post;                                            // variável de primeiro post
const unsigned long TWO_MINUTES = 2 * 60 * 1000;               // 2 minutes in milliseconds
const unsigned long MINUTE = 1 * 60 * 1000;                    // 2 minutes in milliseconds
const unsigned long THIRTY_SECONDS = 30 * 1000;                // 30 seconds in milliseconds
const unsigned long VINTE_QUATRO_HORAS = 24 * 60 * 60 * 1000;  // 12 HORAS
const unsigned long HORA = 60 * 60 * 1000;                     // 1 HORA
float t, setpoint = 50;                                        // mude o valor da temperatura para alterar o setpoint para ligar o cooler
int interacao_com_mqtt;
String cooler = "-Cooler desligado", dht_String = "---";
String para_idle = "00000000000";
String space = "-";
String rx_ping = "RX-ping-";
String msg_inicio = "inicio_rx";

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


void muda_hora(int hora_nova) {
  horas_auto = hora_nova;
  String pub_horas_auto = "O novo intervalo é de " + String(horas_auto) + "horas";
  mqtt.publish("idle_rx", pub_horas_auto.c_str());
}

void muda_min(int min_nova) {
  minutos_auto = min_nova;
  String pub_minutos_auto = "O novo intervalo de bomba ligada é " + String(minutos_auto) + " minutos";
  mqtt.publish("idle_rx", pub_minutos_auto.c_str());
}


void callback(char* controle, byte* payload, unsigned int length = 21) {
  Serial.print("Message received on topic: ");
  Serial.println(controle);
  interacao_com_mqtt = millis();
  String receivedMessage = "";
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }
  handleMessage(receivedMessage);
}


void setup() {  //Iniciando
  Serial.begin(9600);
  Serial.println("Inicio");

  //declara os pinos de saída
  pinMode(Rele40, OUTPUT);   // Cooler
  pinMode(Rele30, OUTPUT);   // Led 3
  pinMode(Rele20, OUTPUT);   // Led 2
  pinMode(Rele10, OUTPUT);   // Led 1
  pinMode(LedMqtt, OUTPUT);  // Led mqtt
  //pinMode(, OUTPUT); // Led mqtt
  wait(500, 2);
  // Deixa todos os relés abertos no inicio da operação, somente para régua de relés
  /*digitalWrite(Rele40, HIGH);
  digitalWrite(Rele30, HIGH);
  digitalWrite(Rele20, HIGH);
  digitalWrite(Rele10, HIGH);*/
  //digitalWrite(13, LOW);
  digitalWrite(Rele40, LOW);
  digitalWrite(Rele30, LOW);
  digitalWrite(Rele20, LOW);
  digitalWrite(Rele10, LOW);
  //digitalWrite(13, LOW);
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
}
void reconnect() {
  //Rotina de conexao
  while (!mqtt.connected()) {
    digitalWrite(LedMqtt, LOW);
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
      wait(1000, 5);
      contador++;
      Serial.println(contador);
    }
    if (contador == 15) {
      ESP.restart();
    }
  }
  digitalWrite(LedMqtt, HIGH);
}
/*
Dicionário de comandos para MQTT
reinicar = reinicia a Esp32
10%20%30%40- = controle individual de cada GPIO relacionada a régua de relés
auto-on = Liga operação automática
auto-off = Desliga operação automática
auto-hora/Numero de horas = altera valor de intervalo de horas na operação automática
auto-min/Numero de minutos = altera valor de intervalo de minutos na operação automática
*/
void handleMessage(String receivedMessage) {
  Serial.println(receivedMessage);

  // Check if the received message contains "Temperatura="
  /*
  if ((receivedMessage.indexOf("Temperatura=") != -1) || (receivedMessage.indexOf("Temperatura =") != -1)) {
    // Se a mensagem contém "Temperatura=", imprime no serial monitor
    //Serial.println("Received message contains 'Temperatura='");
    int startIndex = receivedMessage.indexOf('=') + 1;           // Encontra o índice do '='
    String temperatura = receivedMessage.substring(startIndex);  // Extrai a parte da string após o '='
    temperatura.trim();                                          // Remove espaços em branco extras
    setpoint = temperatura.toFloat();
    Serial.println(setpoint);
    //Serial.println(receivedMessage);
  }
  */

  if (receivedMessage.indexOf("reiniciar") != -1) {
    ESP.restart();
  }
  // Código para acionamento remoto do modo de operação automático
  if (receivedMessage.indexOf("auto-on") != -1) {
    auto_enable = HIGH;
    String auto_on = "automático ligado";
    Serial.println(auto_on);
    mqtt.publish("idle_rx", auto_on.c_str());
    receivedMessage = "0000000000";
  }
  if (receivedMessage.indexOf("auto-off") != -1) {
    auto_enable = LOW;
    String auto_off = "automatico desligado";
    Serial.println(auto_off);
    mqtt.publish("idle_rx", auto_off.c_str());
    receivedMessage = "0000000000";
  }/*
  // Código para alteração dos intervalos de operação automática
  if (receivedMessage.indexOf("auto-hora/")) {
    int startIndex = receivedMessage.indexOf('/') + 1;         // Encontra o índice do '/'
    String hora_nova = receivedMessage.substring(startIndex);  // Extrai a parte da string após o '/'
    hora_nova.trim();
    int hora__nova = int(hora_nova.toFloat());
    muda_hora(hora__nova);
  }
  if (receivedMessage.indexOf("auto-min/")) {
    int startIndex = receivedMessage.indexOf('/') + 1;           // Encontra o índice do '/'
    String minuto_novo = receivedMessage.substring(startIndex);  // Extrai a parte da string após o '/'
    minuto_novo.trim();
    int minuto__novo = int(minuto_novo.toFloat());
    muda_hora(minuto__novo);
  }*/
  // Check if the received message is long enough
  if (receivedMessage.length() >= 8) {/*
    //Faz a interpretação da mensagem e aciona as portas corretas
    if (receivedMessage[1] == '1') {
      digitalWrite(Rele10, LOW);
    } else if (receivedMessage[1] == '0') {
      digitalWrite(Rele10, HIGH);
    }
    if (receivedMessage[4] == '1') {
      digitalWrite(Rele20, LOW);

    } else if (receivedMessage[4] == '0') {
      digitalWrite(Rele20, HIGH);
    }
    if (receivedMessage[7] == '1') {
      digitalWrite(Rele30, LOW);
    } else if (receivedMessage[7] == '0') {
      digitalWrite(Rele30, HIGH);*/
    }  // Duas sequencias de acionamento para relés com normalmente alto.
    if (receivedMessage[10] == '1') {
      digitalWrite(Rele40, HIGH);
    } else if (receivedMessage[10] == '0') {
      digitalWrite(Rele40, LOW);
    }
    if (receivedMessage[1] == '1') {
      digitalWrite(Rele10, HIGH);
    } else if (receivedMessage[1] == '0') {
      digitalWrite(Rele10, LOW);
    }
    if (receivedMessage[4] == '1') {
      digitalWrite(Rele20, HIGH);

    } else if (receivedMessage[4] == '0') {
      digitalWrite(Rele20, LOW);
    }
    if (receivedMessage[7] == '1') {
      digitalWrite(Rele30, HIGH);
    } else if (receivedMessage[7] == '0') {
      digitalWrite(Rele30, LOW);
    }  //Removida lógica de controle a partir da mensagem do servidor para controle do cooler
    if (receivedMessage[10] == '1') {
      digitalWrite(Rele40, HIGH);
    } else if (receivedMessage[10] == '0') {
      digitalWrite(Rele40, LOW);
    }
    para_idle = receivedMessage.substring(0, receivedMessage.indexOf('-'));

  } else {
    Serial.println("Received message is too short");
  }
  Serial.print("Received Message: ");
  Serial.println(receivedMessage);
}

void pub_auto(bool condicao) {  // Função de publicação de atualização de estado pós irrigação automática
  if (condicao) {
    if (mqtt.connected()) {
      ntp.update();
      String agora = ntp.getFormattedTime();
      String string_auto;
      string_auto = "Acionamento automático-Será desligado 15 minutos após agora---" + agora;
      mqtt.publish("idle_rx", string_auto.c_str());
      para_idle[1] = '1';
    }
  } else {
    if (mqtt.connected()) {
      ntp.update();
      String agora = ntp.getFormattedTime();
      String string_auto;
      string_auto = "Acionamento automático-Será ligado novamento daqui 6 horas---" + agora;
      mqtt.publish("idle_rx", string_auto.c_str());
      para_idle[1] = '0';
    }
  }
}

void wait(int ciclo, int num) {  //ciclo -> tempo do ciclo, num -> numero de repeticoes, LED -> porta led
  for (int i = 0; i < num; i++) {
    //digitalWrite(LED, HIGH);
    delay(int(ciclo / 2));
    //digitalWrite(LED, LOW);
    delay(int(ciclo / 2));
  }
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

void deu_ruim() {
  /*digitalWrite(Rele40, HIGH);  // Cooler
  digitalWrite(Rele30, HIGH);  // Led 3
  digitalWrite(Rele20, HIGH);  // Led 2
  digitalWrite(Rele10, HIGH);  // Led 1
  digitalWrite(LedMqtt, LOW);  // Led mqtt*/
  digitalWrite(Rele40, LOW);  // Cooler
  digitalWrite(Rele30, LOW);  // Led 3
  digitalWrite(Rele20, LOW);  // Led 2
  digitalWrite(Rele10, LOW);  // Led 1
  digitalWrite(LedMqtt, LOW);  // Led mqtt
}

void loop() {
  inicio = millis();
  if (!mqtt.connected()) {
    //deu_ruim();
    reconnect();  // restart da placa dentro da função reconnect
  }
  if (primeiro_post == true) {
    mqtt.publish("idle_rx", msg_inicio.c_str());
    Serial.println(msg_inicio);
    interacao_com_mqtt = millis();
    primeiro_post = false;
  }
  //t = dht.readTemperature();
  // Acionamento de cooler a partir do input da temperatura. Setpoint vem do servidor MQTT
  /*if (t >= setpoint) {
      digitalWrite(Rele40, LOW);
      cooler = "-Cooler ligado";
      para_idle[10] = '1';
    } else {
      digitalWrite(Rele40, HIGH);
      cooler = "-Cooler desligado";
      para_idle[10] = '0';
    }*/
  // Poll the MQTT client to check for incoming messages
  mqtt.loop();
  int tempo_fora_do_loop = millis();
  int diferenca = tempo_fora_do_loop - interacao_com_mqtt;
  if (diferenca >= 5000) {
    String tempo = ntp.getFormattedTime();
    // Le a temperatura a cada 5 segundos
    dht_String = String(t, 1);
    String idle_ping = rx_ping + para_idle + space + tempo;  //+ cooler + space + dht_String;
    mqtt.publish("idle_rx", idle_ping.c_str());
    Serial.print("Publicado ");
    Serial.print(idle_ping);
    Serial.println(" no tópico idle_rx.");
    diferenca = 0;
    interacao_com_mqtt = millis();
  }
  // Lógica de operação automática do irrigador.
  if (auto_enable) {
    agora = millis();
    if (agora - tempo_auto >= horas_auto * HORA) {
      //digitalWrite(Rele10, HIGH);
      digitalWrite(Rele10, LOW);
      tempo_auto = agora;
      liga_auto = HIGH;
      pub_auto(liga_auto);
    }
    if (liga_auto && (agora - tempo_auto >= minutos_auto * MINUTE)) {
      //digitalWrite(Rele10, LOW);
      digitalWrite(Rele10, HIGH);
      liga_auto = LOW;
      pub_auto(liga_auto);
      tempo_auto = agora;
    }
  }
  // Add any other logic or delay if needed
  ntp.update();
  delay(500);  // Adjust the delay according to your needs
}