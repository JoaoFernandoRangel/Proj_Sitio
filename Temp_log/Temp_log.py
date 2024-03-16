import sys
import paho.mqtt.client as paho
from datetime import datetime

print(sys.executable)
pc = "jf1"  #altere caso troque de computador

if (pc == "jf"):
    diretorio = "C:\\Users\\João Fernando Rangel\\Desktop\\Proj_Sitio\\Temp_log\\"
else:
    diretorio = "C:\\Users\\Beatriz\\Documents\\GitHub\\Proj_Sitio\\Temp_log\\"

data_hora_atual = datetime.now()
formato_data_hora = "%Y-%m-%d_%H-%M-%S"
timestamp = data_hora_atual.strftime(formato_data_hora)
nome_arquivo = diretorio + timestamp + "__ Dados de temperatura.txt"
arquivo = open(nome_arquivo, "w")
arquivo.write("Teste\n")
#arquivo.close()

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Connected to MQTT broker")
        # Subscribe to the "idle_rx" topic when connected
        client.subscribe("idle_rx", qos=1)
    else:
        print("Connection failed with code:", rc)

def on_message(client, userdata, msg):
    global mensagem
    space = "+++"
    mensagem = msg.payload.decode()
    # Process the received message here instead of just printing
    print("Received message:", mensagem)
    listaa = mensagem.split("-")
    arquivo.write(listaa[3] + space + listaa[2] + space+ listaa[4] + space + listaa[5] + "ºC"+ "\n")
    #arquivo.close()

# Comandos para execução do MQTT
client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect
client.on_message = on_message
client.tls_set(tls_version=paho.ssl.PROTOCOL_TLS)
client.username_pw_set("temp_log", "Campos0102")
client.connect("25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", 8883)
client.loop_forever()
