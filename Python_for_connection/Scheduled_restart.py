import time as t
import datetime as dt
import paho.mqtt.client as paho
from paho import mqtt
import winsound as beep


def on_connect(client, userdata, flags, rc, properties=None):
    #print("CONNACK received with code %s." % rc)
    # Subscribe to the "idle_rx" topic when connected
    client.subscribe("idle_rx", qos=1)

def on_publish(client, userdata, mid, properties=None):
    print("publicado")
    #print("mid: " + str(mid))

def on_subscribe(client, userdata, mid, granted_qos, properties=None):
    print("Subscrito")
    #print("Subscribed: " + str(mid) + " " + str(granted_qos))


def on_message(client, userdata, msg):
    mensagem = msg.payload.decode()
    print("-----------")
    print(f"Mensagem recebida: {mensagem}")
    print("-----------")
    # Handle the received message here
    #display_countdown(remaining_seconds, msg.payload.decode())

client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect
client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)
client.username_pw_set("teste_python", "Campos0102")
client.connect("25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", 8883)
topico = "controle"
print("Antes do loop")
client.subscribe(str(topico), qos=1)
reiniciar = "//reiniciar"
liga = "11%20%30%40-codigoPYTHON"
desliga = "10%20%30%40-codigoPYTHON"
hora = [11,59,59] #Hora, Minuto, Segundo
ctrl = True
client.loop_start() #inicia uma thread separada para o loop do mqtt.
check1 = True
check2 = True
while ctrl:
    #client.publish(topico, "aaaa", qos=1)
    now = dt.datetime.now()  
    print(now)
    if ((now.hour == hora[0]) and (now.minute == hora[1]-17) and (now.second >= hora[2]) and check1):
        client.publish(topico, liga, qos=1)
        t.sleep(1)
        check1 = False
    elif ((now.hour == hora[0]) and (now.minute == hora[1]-2) and (now.second >= hora[2])and check2):
        client.publish(topico, desliga, qos=1)
        t.sleep(1)
        check2 = False
    elif ((now.hour == hora[0]) and (now.minute == hora[1]) and (now.second >= hora[2])):
       t.sleep(1)
       client.publish(topico, reiniciar, qos=1)
          # Sleep for 1 second to avoid multiple prints within the same second
       ctrl =  False
       beep.Beep(1000, 3000)
    t.sleep(0.5)  # Sleep for a short time to reduce CPU usage

