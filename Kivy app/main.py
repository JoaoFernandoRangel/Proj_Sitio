import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button

class MyGridLayout(GridLayout):
    #Initialize infinite keywords
    def __init__(self, **kwargs):
        #Call grid layout constructor
        super(MyGridLayout, self).__init__(**kwargs)

        #Set columns 
        self.cols = 1
        self.row_force_default = True
        self.row_default_height = 120
        self.col_force_default = True
        self.col_default_width = 100
        
        #create a second gridlayout
        self.top_grid = GridLayout(
                                row_force_default = True,
                                row_default_height = 120,
                                col_force_default = True,
                                col_default_width = 100)
        self.top_grid.cols = 2


        #Add widgets
        self.top_grid.add_widget(Label(text="Name: "))
        
        #Add input box
        self.name = TextInput(multiline = False)
        self.top_grid.add_widget(self.name)
        self.top_grid.add_widget(Label(text="Favorite pizza: "))
        
        #Add input box
        self.pizza = TextInput(multiline = False)
        self.top_grid.add_widget(self.pizza)

        self.top_grid.add_widget(Label(text="Color"))
       
        #Add input box
        self.color = TextInput(multiline = False)
        self.top_grid.add_widget(self.color)
       
        #add the new topgrid to our app
        self.add_widget(self.top_grid)


        #add submit button
        self.submit = Button(text="Submit", font_size = 20)
        #bind the button to a function
        self.submit.bind(on_press = self.press)
        self.add_widget(self.submit)
    
    def press(self, instance):
        
        name = self.name.text
        pizza = self.pizza.text
        color = self.color.text
        
        #print(f'Hello {name}, you like {pizza}, and {color}')
        #print it to the screen
        self.add_widget(Label(text = name + pizza)) 

class MyApp(App):
    def build(self):
        return MyGridLayout()
    

if __name__ == '__main__':
    MyApp().run()