#trocar inserção do tópico por um menu dropdown
#adicionar tempos fixos de 10,20,30 e um livre
#ligar apenas o comanda 10 adicionar luz de retorno para os outros comandos.





import tkinter as tk
from tkinter import messagebox
import winsound
import time
import paho.mqtt.client as paho
from paho import mqtt
import pyttsx3

circle_color = 'red'  # Default color
topico = '-'
current_time = '0'
def on_connect(client, userdata, flags, rc, properties=None):
    # Subscribe to the "idle_rx" topic when connected
    client.subscribe("idle_rx", qos=1)

def on_publish(client, userdata, mid, properties=None):
    print("publicado")

def on_subscribe(client, userdata, mid, granted_qos, properties=None):
    print("Subscrito")

def on_message(client, userdata, msg):
    mensagem = msg.payload.decode()
    global circle_color
    if mensagem[9] == '1':
        circle_color = 'green'
    else:
        circle_color = 'red'
    app.canvas.itemconfig(app.circle, fill=circle_color)

    # Display the received message in the text box
    app.message_box.insert(tk.END, f"{mensagem}\n")
    app.message_box.see(tk.END)  # Scroll to the end to show the latest message

def play_beep_sound(duration_ms):
    # Use winsound.Beep to play a beep sound
    winsound.Beep(1000, duration_ms)  # 1000 Hz frequency, specified duration in milliseconds

def speak_message(message):
    # Use pyttsx3 to speak the message
    engine = pyttsx3.init()
    engine.say(message)
    engine.runAndWait()

# Comandos para execução do MQTT
client = paho.Client(client_id="", userdata=None, protocol=paho.MQTTv5)
client.on_connect = on_connect
client.on_message = on_message
client.tls_set(tls_version=mqtt.client.ssl.PROTOCOL_TLS)
client.username_pw_set("teste_python", "Campos0102")
client.connect("25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", 8883)
client.loop_start()

class ContadorRegressivoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Controlador de Irrigação Jacaré")
        self.root.geometry("800x400")  # Definindo a largura e altura inicial da janela
        self.root.configure(bg='black')  # Set background color to black

        self.topico_var = tk.StringVar()
        self.minutos_var = tk.StringVar()

        # Set foreground and background colors for labels and entries in night mode
        label_bg = 'black'
        label_fg = 'white'
        entry_bg = 'black'
        entry_fg = 'white'

        # Increase font size
        font_style = ("Helvetica", 20)

        tk.Label(root, text="Tópico:", bg=label_bg, fg=label_fg, font=font_style).grid(row=0, column=0, padx=10, pady=10)
        tk.Entry(root, textvariable=self.topico_var, bg=entry_bg, fg=entry_fg, font=font_style).grid(row=0, column=1, padx=10, pady=10)

        tk.Label(root, text="Tempo em Minutos:", bg=label_bg, fg=label_fg, font=font_style).grid(row=1, column=0, padx=10, pady=10)
        tk.Entry(root, textvariable=self.minutos_var, bg=entry_bg, fg=entry_fg, font=font_style).grid(row=1, column=1, padx=10, pady=10)

        tk.Button(root, text="Ligar a bomba", command=self.iniciar_contagem, bg=label_bg, fg=label_fg, font=font_style).grid(row=2, column=0, columnspan=2, pady=10)

        # Create a Canvas widget to draw the red circle
        self.canvas = tk.Canvas(root, width=100, height=100, bg='black', highlightthickness=0)
        self.canvas.grid(row=2, column=2, columnspan=2, pady=10)
        
        self.message_box = tk.Text(root, wrap=tk.WORD, height=5, width=55, font=("Helvetica", 14), bg='black', fg='white')
        self.message_box.grid(row=3, column=0, columnspan=4, padx=10, pady=10)

        # Draw a red circle on the Canvas
        self.circle = self.canvas.create_oval(0, 0, 100, 100, fill=circle_color)

        self.display = tk.Label(root, text="", font=("Helvetica", 30), bg='black', fg='white')  # Increase font size for the display
        self.display.grid(row=2, column=1, columnspan=2, pady=10)
        # Bind the window close event to a custom function
        root.protocol("WM_DELETE_WINDOW", self.on_close)
        
    def iniciar_contagem(self):
        global current_time
        current_time = time.strftime("%H:%M:%S")
        global topico 
        topico = self.topico_var.get()
        if (topico == "CONTROLE"): #Adição para qualidade de vida do usuário
            topico = "controle"
        elif (topico == "bomba"):
            topico = "controle"
        elif (topico == "BOMBA"):
            topico = "controle"
        minutos = self.minutos_var.get()
        client.publish(topico, f"11%20%30%40-{current_time}", qos=1)
        try:
            segundos = int(minutos) * 60
            self.atualizar_display(segundos)
            self.contagem_regressiva(segundos)
        except ValueError:
            messagebox.showerror("Erro", "Por favor, insira um número válido para os minutos.")

    def atualizar_display(self, segundos):
        minutos, segundos = divmod(segundos, 60)
        self.display["text"] = f"{minutos:02d}:{segundos:02d}"

    def contagem_regressiva(self, segundos):
        if segundos > 0:
            self.atualizar_display(segundos)
            self.root.after(1000, lambda: self.contagem_regressiva(segundos - 1))
        else:
            current_time = time.strftime("%H:%M:%S")
            client.publish(topico, f"10%20%30%40-{current_time}", qos=1)
            play_beep_sound(500)
            speak_message("Bomba desligada")
            #self.root.destroy()
    def on_close(self):
        # Handle the window close event
        result = messagebox.askokcancel("Aviso", "Não feche o app até o fim da contagem.")
        if result:
            # Perform cleanup or other actions before exiting (if needed)
            client.publish(topico, f"10%20%30%40-{current_time}", qos=1)
            self.root.destroy()
# Cria a janela principal com uma largura e altura maiores
root = tk.Tk()
app = ContadorRegressivoApp(root)

# Inicia o loop principal da interface gráfica
root.mainloop()
