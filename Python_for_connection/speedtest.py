##############################################
import numpy as np
tempo = 6 # meses
taxa = 0.022 # 2.2% ao mês
montante = 8000 # valor objetivo
#Partindo da fórmula:
#montante = Investimento * (1 + taxa)^tempo
#Podemos chegar em:
#Investimento = montante / ((1+taxa)^tempo)
#portanto:

Investimento = montante / (np.power((1+taxa), tempo))
print("Investimento= " + str(Investimento))
#Investimento = 7020.767840120046
#############################################
