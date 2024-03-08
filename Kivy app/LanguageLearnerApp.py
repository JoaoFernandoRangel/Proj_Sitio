from kivy.app import App
from kivy.uix.button import Button


class FunkyButton(Button):
   pass
   #def __init__(self,**kwargs):
   #    super(FunkyButton, self).__init__(**kwargs)
   #    self.text = "Funky button"
   #    self.pos = (0,0)
   #    self.size_hint = (.5,.5)


 
class LanguageLearnerApp(App):
    def build(self):
        return FunkyButton(
            pos = (100,100),
            size_hint = (None, None),
            size = (500,500)
        )        
        


if __name__== '__main__':
    LanguageLearnerApp().run()
