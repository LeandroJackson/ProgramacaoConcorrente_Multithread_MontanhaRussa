# Parque de Diversões - Montanha Russa (Multithread)

Este é um projeto, da disciplina Linguagem Programação II, de simulação de um parque de diversões, focado na operação de uma montanha russa, utilizando programação concorrente com múltiplas threads.

## Descrição

O projeto consiste em simular o funcionamento de um parque de diversões, onde há um carro da montanha russa e vários passageiros. A ideia é que os passageiros entrem no carro, aguardem até que o carro esteja cheio, realizem uma volta completa e, em seguida, desembarquem para dar espaço a novos passageiros.

O programa utiliza a biblioteca `pthread` para criar e gerenciar as threads correspondentes ao carro e aos passageiros. Além disso, são utilizadas técnicas de exclusão mútua, como semáforos e barreiras, para sincronizar o acesso ao carro e garantir que as ações sejam realizadas de forma correta.

## Funcionalidades

- Entrada e saída dos passageiros no carro da montanha russa
- Espera até que o carro esteja cheio para iniciar a volta
- Realização de voltas completas pela montanha russa
- Contagem do número de voltas realizadas
- Finalização do programa após um número pré-definido de voltas

## Como executar

1. Certifique-se de ter o compilador `gcc` instalado no seu sistema.
2. Clone este repositório para o seu computador.
3. Abra um terminal na pasta raiz do projeto.
4. Execute o seguinte comando para compilar o programa:
   ```
   gcc -o parque main.c -lpthread
   ```
5. Execute o programa gerado com o comando:
   ```
   ./parque
   ```
