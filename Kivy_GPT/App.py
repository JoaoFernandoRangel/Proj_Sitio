#Timer funcionando, Seleção de topico funcionando. Ajustar conexão e interação com servidor MQTT.



from kivy.app import App
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.dropdown import DropDown
from kivy.clock import Clock
import paho.mqtt.client as mqtt

class MQTTClient:
    def __init__(self, client_id="", username="", password="", host="", port=1883):
        self.client = mqtt.Client(client_id=client_id)
        self.client.username_pw_set(username, password)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.host = host
        self.port = port

    def connect(self):
        self.client.connect(self.host, self.port)
        self.client.loop_start()

    def disconnect(self):
        self.client.disconnect()
        self.client.loop_stop()

    def on_connect(self, client, userdata, flags, rc):
        print(f"Connected with result code {rc}")

    def on_message(self, client, userdata, message):
        global label_message_recebida
        label_message_recebida = message.payload.decode()

    def subscribe(self, topic):
        self.client.subscribe(topic)

    def publish(self, topic, message, qos=0):  # Adicionando o parâmetro qos
        self.client.publish(topic, message, qos=qos)

class DropdownButton(Button):
    def __init__(self, **kwargs):
        super(DropdownButton, self).__init__(**kwargs)
        self.dropdown = DropDown()

    def on_release(self):
        self.dropdown.open(self)

    def on_select(self, text):
        self.text = text  # Update the text of the button with the selected item
        self.dropdown.dismiss()  # Close the dropdown after selection

    def add_item(self, text):
        btn = Button(text=text, size_hint_y=None, height=30)
        btn.bind(on_release=lambda btn: self.on_select(btn.text))
        self.dropdown.add_widget(btn)


class MyApp(App):
    def __init__(self, **kwargs):
        super(MyApp, self).__init__(**kwargs)
        self.label_message_recebida = ""  # Definindo a variável global

        self.mqtt_client = MQTTClient(client_id="", username="teste_python", password="Campos0102",
                                       host="25d06c5109f94ef78c7bcfc1c33fdf20.s2.eu.hivemq.cloud", port=8883)

    def on_start(self):
        self.mqtt_client.connect()

    def build(self):
        self.layout = GridLayout(cols=1, spacing=10)

        # Dropdown 1
        self.dropdown1_btn = DropdownButton(text='Tempo: ', size_hint_y=None, height=50)
        self.dropdown1_btn.add_item('1 min')
        self.dropdown1_btn.add_item('5 min')
        self.dropdown1_btn.add_item('10 min')
        self.dropdown1_btn.add_item('20 min')
        self.dropdown1_btn.add_item('30 min')

        # Dropdown 2
        self.dropdown2_btn = DropdownButton(text='Tópico: ', size_hint_y=None, height=50)
        self.dropdown2_btn.add_item('controle')
        self.dropdown2_btn.add_item('teste')

        # Button to print to the terminal
        self.print_button = Button(text="Imprimir", size_hint_y=None, height=50)
        self.print_button.bind(on_press=self.start_timer)

        # Timer label
        self.timer_label = Label(text="00:00", size_hint_y=None, height=50)

        # Message label
        self.message_label = Label(text=self.label_message_recebida, size_hint_y=None, height=150, max_lines=10)

        self.layout.add_widget(self.dropdown1_btn)
        self.layout.add_widget(self.dropdown2_btn)
        self.layout.add_widget(self.print_button)
        self.layout.add_widget(self.timer_label)
        self.layout.add_widget(self.message_label)

        return self.layout

    def start_timer(self, instance):
        self.remaining_time = self.get_selected_time_in_seconds()
        self.update_timer_label()
        # Adicionando publicação da mensagem no tópico "aaa"
        self.mqtt_client.publish("aaa", "Timer started", qos=1)
        Clock.schedule_interval(self.update_timer_label, 1)

    def get_selected_time_in_seconds(self):
        selected_time_text = self.dropdown1_btn.text.split()[0]  # Extracting the numerical value from the text
        selected_time_minutes = int(selected_time_text)
        return selected_time_minutes * 60

    def update_timer_label(self, *args):
        minutes = self.remaining_time // 60
        seconds = self.remaining_time % 60
        self.timer_label.text = f"{minutes:02}:{seconds:02}"
        self.remaining_time -= 1
        if self.remaining_time < 0:
            self.timer_label.text = "Tempo terminado"
            return False  # Stop the Clock schedule

    def on_stop(self):
        self.mqtt_client.disconnect()
        super(MyApp, self).on_stop()

if __name__ == '__main__':
    MyApp().run()
