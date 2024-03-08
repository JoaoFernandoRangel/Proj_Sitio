from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.dropdown import DropDown

class DropdownApp(App):
    def build(self):
        layout = BoxLayout(orientation='vertical', spacing=10, padding=10)

        # Botão Dropdown 1 - Seleção de tempo
        dropdown1 = DropDown()
        for option in ['1 min', '5 min', '10 min', '20 min', '30 min']:
            btn = Button(text=option, size_hint_y=None, height=44)
            btn.bind(on_release=lambda btn: dropdown1.select(btn.text))
            dropdown1.add_widget(btn)

        button1 = Button(text='Selecione o Tempo')
        button1.bind(on_release=dropdown1.open)
        dropdown1.bind(on_select=lambda instance, x: setattr(button1, 'text', x))
        layout.add_widget(button1)

        # Botão Dropdown 2
        dropdown2 = DropDown()
        for option in ['Opção 2A', 'Opção 2B', 'Opção 2C']:
            btn = Button(text=option, size_hint_y=None, height=44)
            btn.bind(on_release=lambda btn: dropdown2.select(btn.text))
            dropdown2.add_widget(btn)

        button2 = Button(text='Selecione Opção 2')
        button2.bind(on_release=dropdown2.open)
        dropdown2.bind(on_select=lambda instance, x: setattr(button2, 'text', x))
        layout.add_widget(button2)

        return layout

if __name__ == '__main__':
    DropdownApp().run()
