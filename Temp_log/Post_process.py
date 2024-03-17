import numpy as np
import matplotlib.pyplot as plt

pc = "jf"  #altere caso troque de computador

if (pc == "jf"):
    diretorio = "C:\\Users\\João Fernando Rangel\\Desktop\\Proj_Sitio\\Temp_log\\"
else:
    diretorio = "C:\\Users\\Beatriz\\Documents\\GitHub\\Proj_Sitio\\Temp_log\\"

nome_do_arquivo = "2024-03-16_19-11-15__ Dados de temperatura.txt"
novo_arquivo = "temperatura.csv"
nome = diretorio + nome_do_arquivo
arquivo = open(nome, "r")
arquivo_novo = open(diretorio + novo_arquivo, "w")
i=0
for linha in arquivo:
    quebrado = linha.split("+++")
    final = quebrado[0] + "," + quebrado[3][:4]
    arquivo_novo.write(final.strip() + "\n")
    print(final.strip())
