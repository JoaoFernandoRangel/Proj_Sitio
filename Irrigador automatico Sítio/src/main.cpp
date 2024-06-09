#include <Arduino.h>
#include <NTPClient.h>
#include "String.h"
#include "time.h"
#include "PubSubClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Define a struct to hold WiFi configuration
struct WifiConfig
{
  const char *SSID;
  const char *PASS;
};

// Instantiate instances of the struct for each network
struct WifiConfig bia2Config = {
    .SSID = "Bia 2",
    .PASS = "coisafacil"};

struct WifiConfig casaPiscinaConfig = {
    .SSID = "CS_TELECOM_CS96",
    .PASS = "cs2017cs3337"};

struct WifiConfig vivofibraConfig = {
    .SSID = "VIVOFIBRA-09D8",
    .PASS = "816329FCDE"};

struct WifiConfig casaLuConfig = {
    .SSID = "VIVOFIBRA-DB00",
    .PASS = "55228E47BB"};

struct WifiConfig sitioNewnet = {
    .SSID = "SITIO_NEWNET",
    .PASS = "coisafacil"};

struct WifiConfig escritorioConfig = {
    .SSID = "Escritorio",
    .PASS = "cs2017cs3337"};

struct WifiConfig rede_celular = {
    .SSID = "REDE",
    .PASS = "12345678a"};

// Create an array of WifiConfig structs
struct WifiConfig wifiVector[] = {
    bia2Config,
    sitioNewnet,
    rede_celular,
    casaPiscinaConfig,
    vivofibraConfig,
    casaLuConfig,
    escritorioConfig

};
void connectToWiFi(const struct WifiConfig &config, unsigned long timeoutMillis)
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(config.SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.SSID, config.PASS);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeoutMillis)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println();
    Serial.println("Failed to connect to WiFi within the timeout. Moving to the next network.");
  }
}

// Cria um WiFiClient class para utilizar no MQTT server.
WiFiClientSecure client;
// Cria uma instância para medição de tempo
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);

// Declarações das portas da esp32
#define Rele10 13
#define Rele20 27
#define Rele30 26
#define Rele40 33
#define LedMqtt 19

int portas[] = {0, 13, 27, 26, 33};

void check_state(bool check1, bool check2, int porta)
{
  if ((check1 || check2) == true)
  {
    digitalWrite(porta, HIGH);
  }
  else
  {
    digitalWrite(porta, LOW);
  }
}
bool handle_message_check[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool timer_check = LOW;

void double_check(bool check1, bool check2, int porta)
{
  // Function body
  if ((check1 || check2) == true)
  {
    digitalWrite(porta, HIGH);
  }
  else
  {
    digitalWrite(porta, LOW);
  }
}

// Define informacoes MQTT
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

// Variáveis de frequência e tempo
int f = 10;                // valor em Hz
unsigned long time_ini;    // tempo em ms que começou o segundo no timestamp
unsigned long refTime = 0; // tempo de início do loop

// Variáveis de controle de transmissão
bool ativar = true; // indica se vai rodar a transmissão

// Variáveis de operação automática
unsigned long agora, tempo_auto = 0;
unsigned long off_auto = 4, on_auto = 15; // horas e minutos.
bool liga_auto = LOW, auto_enable = HIGH;

// Variáveis de controle de porta
unsigned long lastToggleTime = 0, inicio;
bool port33State = HIGH; // estado inicial (HIGH ou LOW)

// Variáveis de controle de temporização
const unsigned long minutos = 1 * 60 * 1000; // 1 minuto em milissegundos
const unsigned long horas = 60 * 60 * 1000;  // 1 hora em milissegundos

// Variáveis de controle de temperatura
float t, setpoint = 50; // valor da temperatura para alterar o setpoint para ligar o cooler

// Variáveis de interação MQTT
int interacao_com_mqtt;
bool primeiro_post; // variável de primeiro post
unsigned long idle = 0;

// Strings para mensagens e controle
String cooler = "-Cooler desligado", dht_String = "---";
String space = "-";
String rx_ping = "RX-ping-";
String msg_inicio = "inicio_rx";
String top_idle = "/idle_rx";  // tópico para envio idle
String top_ctrl = "/controle"; // tópico de controle
String string_comando = "10%20%30%40-";
String string_desliga = "10%20%30%40-";
String tempo, idle_ping, string_envia_l, string_envia_d;

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

/*
Dicionário de comandos para MQTT
10%20%30%40- = controle individual de cada GPIO relacionada a régua de relés
//auto-on = Liga operação automática
//auto-off = Desliga operação automática
//tempo-off/Numero de horas = altera valor de intervalo de horas na operação automática
//tempo-on/Numero de minutos = altera valor de intervalo de minutos na operação automática
//reinicar = reinicia a Esp32
*/

void handleMessage(String receivedMessage)
{

  Serial.println(receivedMessage);
  if (receivedMessage.startsWith("//"))
  {
    if (receivedMessage.indexOf("reiniciar") != -1)
    {
      ESP.restart();
    }
    else if (receivedMessage.indexOf("auto-on") != -1)
    {
      auto_enable = HIGH;
      Serial.println("Função automática ligada");
      mqtt.publish(top_idle.c_str(), "Auto ligado");
    }
    else if (receivedMessage.indexOf("auto-off") != -1)
    {
      auto_enable = LOW;
      Serial.println("Função automática desligada");
      mqtt.publish(top_idle.c_str(), "Auto desligado");
      delay(200);
      mqtt.publish(top_ctrl.c_str(), string_desliga.c_str());
    }
    else if (receivedMessage.indexOf("tempo-on") != -1)
    {
      int startIndex = receivedMessage.indexOf("=") + 1;
      String ligado_novo = receivedMessage.substring(startIndex);
      ligado_novo.trim();
      int ligado_novo_int = int(ligado_novo.toFloat());
      String envio = "Seu novo tempo ligado é: " + ligado_novo + " minutos";
      mqtt.publish(top_idle.c_str(), envio.c_str());
      Serial.print("Minutagem ligada alterada para: ");
      Serial.println(ligado_novo_int);
      on_auto = ligado_novo_int;
    }
    else if (receivedMessage.indexOf("tempo-off") != -1)
    {
      int startIndex = receivedMessage.indexOf("=") + 1;
      String desligado_novo = receivedMessage.substring(startIndex);
      desligado_novo.trim();
      int desligado_novo_int = int(desligado_novo.toFloat());
      String envio = "Seu novo tempo desligado é: " + desligado_novo + " horas";
      mqtt.publish(top_idle.c_str(), envio.c_str());
      Serial.print("Novo tempo desligado: ");
      Serial.println(desligado_novo_int);
      off_auto = desligado_novo_int;
    }
  }
  else if (receivedMessage.length() >= 8)
  {
    // Faz a interpretação da mensagem e aciona as portas corretas
    if (receivedMessage[1] == '1')
    {
      // digitalWrite(Rele10, LOW);
      handle_message_check[1] = LOW;
    }
    else if (receivedMessage[1] == '0')
    {
      // digitalWrite(Rele10, HIGH);
      handle_message_check[1] = HIGH;
    }
    if (receivedMessage[4] == '1')
    {
      // digitalWrite(Rele20, LOW);
      handle_message_check[2] = LOW;
    }
    else if (receivedMessage[4] == '0')
    {
      // digitalWrite(Rele20, HIGH);
      handle_message_check[2] = HIGH;
    }
    if (receivedMessage[7] == '1')
    {
      // digitalWrite(Rele30, LOW);
      handle_message_check[3] = LOW;
    }
    else if (receivedMessage[7] == '0')
    {
      handle_message_check[3] = HIGH;
      // digitalWrite(Rele30, HIGH);
    }
    if (receivedMessage[10] == '1')
    {
      // digitalWrite(Rele40, LOW);
      handle_message_check[4] = LOW;
    }
    else if (receivedMessage[10] == '0')
    {
      handle_message_check[4] = HIGH;
      // digitalWrite(Rele40, HIGH);
    }
  }
  else
  {
    Serial.println("Received message is too short");
  }
  Serial.print("Received Message: ");
  Serial.println(receivedMessage);
}

void callback(char *controle, byte *payload, unsigned int length = 21)
{
  Serial.print("Message received on topic: ");
  Serial.println(controle);
  interacao_com_mqtt = millis();
  String receivedMessage = "";
  for (int i = 0; i < length; i++)
  {
    receivedMessage += (char)payload[i];
  }
  handleMessage(receivedMessage);
}

void setup()
{ // Iniciando
  Serial.begin(9600);
  Serial.println("Inicio");

  // declara os pinos de saída
  pinMode(Rele40, OUTPUT);  // Cooler
  pinMode(Rele30, OUTPUT);  // Led 3
  pinMode(Rele20, OUTPUT);  // Led 2
  pinMode(Rele10, OUTPUT);  // Led 1
  pinMode(LedMqtt, OUTPUT); // Led mqtt
  wait(500, 2);
  // Deixa todos os relés abertos no inicio da operação, somente para régua de relés
  digitalWrite(Rele40, HIGH);
  digitalWrite(Rele30, HIGH);
  digitalWrite(Rele20, HIGH);
  digitalWrite(Rele10, HIGH);
  // digitalWrite(13, LOW);
  /*
  digitalWrite(Rele40, LOW);
  digitalWrite(Rele30, LOW);
  digitalWrite(Rele20, LOW);
  digitalWrite(Rele10, LOW);

  */
  // digitalWrite(13, LOW);
  mqtt.setCallback(callback);
  primeiro_post = true;
  // Conecta o Wifi na rede
  for (size_t ji = 0; ji < sizeof(wifiVector) / sizeof(wifiVector[0]); ++ji)
  {
    connectToWiFi(wifiVector[ji], 15000); // Timeout set to 15 seconds (15000 milliseconds)
    // Check if connected to WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
      break; // Exit the loop if connected successfully
    }
  }

  // Conecta servidor MQTT
  client.setCACert(root_ca);
  mqtt.setServer(SERVER, SERVERPORT);
  reconnect();

  // Inicia NTP para adquirir data e hora
  ntp.begin();
  ntp.setTimeOffset(-10800); // corrige para fuso horário
  /*
  Inicia o sensor de temperatura
  dht.begin();
  */
  ntp.update();
}
void reconnect()
{
  // Rotina de conexao
  while (!mqtt.connected())
  {
    digitalWrite(LedMqtt, LOW);
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "Esp32";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), user, pass))
    {
      Serial.println("conectado");
      contador = 0;
      mqtt.subscribe(top_ctrl.c_str(), 0); // inscricao no topico 'controle'
    }
    else
    {
      Serial.print("falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 5 segundos");
      wait(1000, 5);
      contador++;
      Serial.println(contador);
    }
    if (contador == 15)
    {
      ESP.restart();
    }
  }
  digitalWrite(LedMqtt, HIGH);
}
/*
Dicionário de comandos para MQTT
reinicar => reinicia a Esp32
10%20%30%40- => controle individual de cada GPIO relacionada a régua de relés
auto-on => Liga operação automática
auto-off => Desliga operação automática
auto-hora=/Numero de horas => altera valor de intervalo de horas na operação automática
auto-min=/Numero de minutos => altera valor de intervalo de minutos na operação automática
*/

void wait(int ciclo, int num)
{ // ciclo -> tempo do ciclo, num -> numero de repeticoes, LED -> porta led
  for (int i = 0; i < num; i++)
  {
    // digitalWrite(LED, HIGH);
    delay(int(ciclo / 2));
    // digitalWrite(LED, LOW);
    delay(int(ciclo / 2));
  }
}
void reconnectToWiFi()
{
  // Tenta reconectar-se a uma rede WiFi
  for (size_t ji = 0; ji < sizeof(wifiVector) / sizeof(wifiVector[0]); ++ji)
  {
    connectToWiFi(wifiVector[ji], 15000); // Timeout definido como 15 segundos (15000 milissegundos)

    // Verifica se está conectado ao WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi reconnected successfully.");
      return; // Sai da função se reconectar com sucesso
    }
  }

  // Se não conseguir se reconectar a nenhuma rede, aguarde um curto período antes de tentar novamente
  Serial.println("Failed to reconnect to WiFi. Retrying in 5 seconds...");
  delay(5000);
}

void deu_ruim()
{
  digitalWrite(Rele40, HIGH); // Cooler
  digitalWrite(Rele30, HIGH); // Led 3
  digitalWrite(Rele20, HIGH); // Led 2
  digitalWrite(Rele10, HIGH); // Led 1
  /*digitalWrite(LedMqtt, LOW);  // Led mqtt
  digitalWrite(Rele40, LOW);  // Cooler
  digitalWrite(Rele30, LOW);  // Led 3
  digitalWrite(Rele20, LOW);  // Led 2
  digitalWrite(Rele10, LOW);  // Led 1
  digitalWrite(LedMqtt, LOW); // Led mqtt*/
}

String string_idle_constructor()
{
  String envia_idle = "10%20%30%40";
  /* Para relés normais*/
  /*
  envia_idle[1] = digitalRead(Rele10) ? '1' : '0';
  envia_idle[4] = digitalRead(Rele20) ? '1' : '0';
  envia_idle[7] = digitalRead(Rele30) ? '1' : '0';
  envia_idle[10] = digitalRead(Rele40) ? '1' : '0';*/
  /*Para régua que desliga com nível baixo*/
  envia_idle[1] = digitalRead(Rele10) ? '0' : '1';
  envia_idle[4] = digitalRead(Rele20) ? '0' : '1';
  envia_idle[7] = digitalRead(Rele30) ? '0' : '1';
  envia_idle[10] = digitalRead(Rele40) ? '0' : '1';
  return envia_idle;
}

void loop()
{
  ntp.update();
  if (primeiro_post == true)
  {
    mqtt.publish(top_idle.c_str(), msg_inicio.c_str());
    Serial.println(msg_inicio);
    primeiro_post = false;
  }
  if ((millis() - idle) >= 5000)
  {
    tempo = ntp.getFormattedTime();
    idle_ping = rx_ping + string_idle_constructor() + space + tempo;
    mqtt.publish(top_idle.c_str(), idle_ping.c_str());
    Serial.print("Publicado ");
    Serial.print(idle_ping);
    Serial.print(" no tópico: ");
    Serial.println(top_idle);
    idle = millis();
  }
  if (!mqtt.connected())
  {
    // deu_ruim();
    reconnect(); // restart da placa dentro da função reconnect
  }
  // Poll the MQTT client to check for incoming messages
  mqtt.loop();
  /* Modularizar a função de operação automática */
  // Lógica de operação automática do irrigador.
  if (auto_enable)
  {
    agora = millis();
    if (!liga_auto && (agora - tempo_auto >= off_auto * horas))
    {
      Serial.print("valor ligado: ");
      Serial.print(on_auto);
      Serial.print("||| ");
      Serial.print("valor desligado: ");
      Serial.println(off_auto);
      tempo_auto = agora;
      string_comando[1] = '1';
      string_envia_l = string_comando + "___Mensagem automática";
      timer_check = HIGH;
      Serial.println(string_envia_l);
      if (mqtt.connected())
      {
        mqtt.publish(top_ctrl.c_str(), string_envia_l.c_str());
      }
      liga_auto = HIGH;
      Serial.println("A bomba está ligada");
    }
    else if (liga_auto && (agora - tempo_auto >= on_auto * minutos))
    {
      Serial.print("valor ligado: ");
      Serial.print(on_auto);
      Serial.print("||| ");
      Serial.print("valor desligado: ");
      Serial.println(off_auto);
      tempo_auto = agora;
      liga_auto = LOW;
      string_comando[1] = '0';
      string_envia_d = string_comando + "___Mensagem automática";
      Serial.println(string_envia_d);
      if (mqtt.connected()){
      mqtt.publish(top_ctrl.c_str(), string_envia_d.c_str());
      }
      timer_check = LOW;
      Serial.println("A bomba foi desligada");
    }
  }
  for (int iii = 1; iii < 6; iii++)
  {
    double_check(handle_message_check[iii], timer_check, portas[iii]);
  }
  // Add any other logic or delay if needed;
  delay(100); // Adjust the delay according to your needs
}
