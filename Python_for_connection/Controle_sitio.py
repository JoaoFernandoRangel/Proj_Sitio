import time
import winsound
import paho.mqtt.client as paho
from paho import mqtt
import pyttsx3

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
    

def play_beep_sound(duration_ms):
    # Use winsound.Beep to play a beep sound
    winsound.Beep(1000, duration_ms)  # 1000 Hz frequency, specified duration in milliseconds

def speak_message(message):
    # Use pyttsx3 to speak the message
    engine = pyttsx3.init()
    engine.say(message)
    engine.runAndWait()

def display_countdown(seconds, idle_rx_message):
    # Convert seconds to HH:MM:SS format
    hours, remainder = divmod(seconds, 3600)
    minutes, seconds = divmod(remainder, 60)
    countdown_str = "{:02}:{:02}:{:02}".format(int(hours), int(minutes), int(seconds))
    
    print(f"Tempo restante: {countdown_str}{idle_rx_message}")

client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect
client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)
client.username_pw_set("teste_python", "Campos0102")
client.connect("25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", 8883)
print("Escreva o tópico onde será publicada a mensagem")
print("Para ligar a bomba escreva controle")
topico = input("Escreva o tópico: ")
print("-----------")
#client.on_subscribe = on_subscribe
client.on_message = on_message
#client.on_publish = on_publish

client.subscribe(str(topico), qos=1)

# Get the current time in the desired format
current_time = time.strftime("%H:%M:%S")
print("Por quanto tempo deseja ligar a bomba ?")
tempo_ligada = input("Escreva o tempo em minutos: ")
print("-----------")
# Calculate the time when the bomb will be turned off
turn_off_time = time.time() + int(tempo_ligada)*60
print(f"A bomba será desligada às {time.strftime('%H:%M:%S', time.localtime(turn_off_time))}")
print("-----------")
# Publish the initial message with the current time
client.publish(topico, f"11%20%30%40-{current_time}", qos=1)
print("Bomba Ligada - Desligando às", time.strftime('%H:%M:%S', time.localtime(turn_off_time)))

# Countdown loop
while time.time() < turn_off_time:
    remaining_seconds = int(turn_off_time - time.time())
    
    # Check for any messages on the "idle_rx" topic
    client.loop(timeout=1.0, max_packets=1)
    
    # Print the received message alongside the countdown
    display_countdown(remaining_seconds,"")
    
    time.sleep(1)

# Get the current time again1
    
current_time = time.strftime("%H:%M:%S")

# Publish the updated message with the current time
client.publish(topico, f"10%20%30%40-{current_time}", qos=1)
print("Bomba desligada")

# Play a beeping sound for 2 seconds
play_beep_sound(3000)

# Speak the message "Bomba desligada"
speak_message("Bomba desligada")

# Disconnect from the broker and exit the program
client.disconnect()
print("Program closed.")
