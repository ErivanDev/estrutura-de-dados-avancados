import sys
import matplotlib.pyplot as plt
import numpy as np

def calcular_angulo(x1, y1, x2, y2):
    # Calcula o ângulo entre a reta e o eixo x
    return np.arctan2(y2 - y1, x2 - x1) * 180 / np.pi

def ler_arquivo(nome_arquivo):
    retas = []
    pontos = []
    with open(nome_arquivo, 'r') as arquivo:
        linhas = arquivo.readlines()
        num_retas = int(linhas[0])
        for linha in linhas[1:num_retas+1]:
            pontos_str = linha.strip().split(' ')
            pontos_reta = [(float(ponto.split(',')[0]), float(ponto.split(',')[1])) for ponto in pontos_str]
            retas.append(pontos_reta)
        
        num_pontos = int(linhas[num_retas + 1])
        for linha in linhas[num_retas + 2:num_retas + num_pontos + 2]:
            x, y = linha.strip().split(',')
            pontos.append((float(x), float(y)))
    
    return retas, pontos

def desenhar_retas_e_pontos(retas, pontos):
    for i, reta in enumerate(retas):
        x = [p[0] for p in reta]
        y = [p[1] for p in reta]
        plt.plot(x, y, label=f'Reta {i+1}')
        # Calcula o ponto médio da reta
        x_medio = np.mean(x)
        y_medio = np.mean(y)
        # Calcula o ângulo da reta
        angulo = calcular_angulo(x[0], y[0], x[-1], y[-1])
        # Adiciona o texto com o número da reta com uma pequena margem acima da reta
        offset = 0.1 * max(max(y) - min(y), max(x) - min(x))
        plt.text(x_medio, y_medio + offset, f'Reta {i+1}', fontsize=8, ha='center', va='center', rotation=angulo)
    
    for i, ponto in enumerate(pontos):
        plt.scatter(ponto[0], ponto[1], color='red')
        plt.text(ponto[0], ponto[1], f'({ponto[0]}, {ponto[1]})', fontsize=8, verticalalignment='bottom')
    
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title('Retas e Pontos')
    plt.legend()
    plt.grid(True)
    plt.axis('equal')
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python script.py <caminho_do_arquivo>")
    else:
        nome_arquivo = sys.argv[1]
        retas, pontos = ler_arquivo(nome_arquivo)
        desenhar_retas_e_pontos(retas, pontos)