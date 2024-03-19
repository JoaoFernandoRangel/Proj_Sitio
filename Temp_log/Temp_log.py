import sys
import paho.mqtt.client as paho
from datetime import datetime, time
import os

pc = "jf"  # altere caso troque de computador

if pc == "jf":
    diretorio = "C:\\Users\\João Fernando Rangel\\Desktop\\Proj_Sitio\\Temp_log\\"
else:
    diretorio = "C:\\Users\\Beatriz\\Documents\\GitHub\\Proj_Sitio\\Temp_log\\"

def create_new_files():
    global data_hora_atual, arquivo1, arquivo2, nome_arquivo_registro, nome_arquivo_post
    data_hora_atual = datetime.now()
    formato_data_hora = "%Y-%m-%d_%H-%M-%S"
    timestamp = data_hora_atual.strftime(formato_data_hora)
    nome_arquivo_registro = diretorio + timestamp + "__ Dados de temperatura.txt"
    nome_arquivo_post = diretorio + timestamp + "__Temperatura.csv"
    arquivo1 = open(nome_arquivo_registro, "w")
    arquivo2 = open(nome_arquivo_post, "w")
    

def check_cross_midnight(timestamp_from_msg):
    # Extracting only the time part from the timestamp
    timestamp_msg = datetime.strptime(timestamp_from_msg, "%H:%M:%S").time()
    # Checking if the time is after midnight and before 00:00:10
    midnight = time(hour=0, minute=0, second=0)
    ten_seconds_after_midnight = time(hour=0, minute=0, second=10)
    if midnight < timestamp_msg < ten_seconds_after_midnight:
        return True
    return False
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Connected to MQTT broker")
        # Subscribe to the "idle_rx" topic when connected
        client.subscribe("idle_rx", qos=1)
    else:
        print("Connection failed with code:", rc)

def on_message(client, userdata, msg):
    global mensagem, arquivo1, arquivo2
    space = "+++"
    mensagem = msg.payload.decode()
    # Process the received message here instead of just printing
    print("Received message:", mensagem)
    listaa = mensagem.split("-")

    if check_cross_midnight(listaa[3]):
        arquivo1.close()
        arquivo2.close()
        create_new_files()

    arquivo1.write(listaa[3] + space + listaa[2] + space + listaa[4] + space + listaa[5] + "ºC" + "\n")
    arquivo2.write(listaa[3] + "," + listaa[5] + "\n")
    print("Escrito no .csv:" + listaa[3] + "," + listaa[5])

# Comandos para execução do MQTT
client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect
client.on_message = on_message
client.tls_set(tls_version=paho.ssl.PROTOCOL_TLS)
client.username_pw_set("temp_log", "Campos0102")
client.connect("25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", 8883)

# Create initial files
create_new_files()

client.loop_forever()
