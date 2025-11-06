# 🤖 Simulador de Escalonador de CPU (Round Robin)


Projeto acadêmico, desenvolvido em **C/C++**, que simula o escalonador de processos de um Sistema Operacional. O programa gerencia múltiplos processos, movendo-os entre os estados de Pronto, Execução e Bloqueado, utilizando o algoritmo **Round Robin** com `quantum` de tempo.

A simulação é interativa e visual, utilizando a biblioteca `conio2.h` para exibir o status de cada processo em tempo real no console.

## 📖 Sobre o Projeto

O objetivo deste projeto é aplicar na prática os conceitos teóricos de Sistemas Operacionais. Ele gerencia um "Process Control Block" (a `struct ProcessoControl`) para cada processo, contendo informações vitais como ID, tempo restante, tempo já executado, estado e informações sobre parentesco (pais/filhos).

---

## ✨ Conceitos e Funcionalidades Implementadas

### 1. Modelo de Estados de Processo
O simulador utiliza três filas distintas para gerenciar os estados dos processos:
* **Fila de Prontos (Circular):** Uma `TpFilaCircular` que implementa o **Round Robin**. Processos prontos para executar aguardam sua vez aqui.
* **Fila de Execução (FIFO):** Uma `TpFilaFIFO` simples que (nesta simulação) contém o processo que está atualmente na "CPU".
* **Fila de Espera/Bloqueados (FIFO):** Uma `TpFilaFIFO` para onde vão os processos que:
    * Foram bloqueados (simulando E/S - I/O).
    * São processos "Pai" que estão aguardando um "Filho" terminar.

### 2. Algoritmo Round Robin
* Um processo é retirado da Fila de Prontos e executa por um `quantum` de tempo aleatório (até `MAX_QUANTUM`).
* Se o processo não terminar, ele é preemptado e retorna ao **final da Fila de Prontos**.

### 3. Criação de Processos (Simulação de `fork()`)
* Durante sua execução, um processo tem uma chance aleatória de criar um processo filho.
* **Limite de Filhos:** Cada processo pode criar até `MAX_FILHOS_POR_PROCESSO`.
* **Limite de Profundidade:** A árvore de processos é limitada a `MAX_PROFUNDIDADE` (ex: Pai -> Filho -> Neto).
* **Bloqueio do Pai:** Quando um pai cria um filho, ele é imediatamente movido para a **Fila de Espera** e só retorna à Fila de Prontos quando seu filho termina a execução (`finalizarFilhoEDesbloquearPai`).

### 4. Simulação Interativa
* A qualquer momento durante a simulação, o usuário pode pressionar a tecla **'A'** para adicionar um novo processo (`adicionarProcessoUsuario`).
* O novo processo é inserido na Fila de Prontos e entra no ciclo de escalonamento.

### 5. Coleta de Estatísticas
Ao final da simulação (quando todas as filas estão vazias), o programa exibe um relatório completo contendo:
* Total de processos finalizados.
* Total de processos que foram bloqueados.
* Tempo médio de bloqueio.
* Número de filhos que cada processo criou.
* Tempo total que cada pai ficou bloqueado esperando seus filhos.

---

## 🚀 Tecnologias Utilizadas

* **C / C++**
* **Conceitos de Sistemas Operacionais:**
    * Escalonamento de CPU (Round Robin)
    * Gerenciamento de Processos (Criação/Término)
    * Estados de Processo (Pronto, Executando, Bloqueado)
    * Process Control Block (PCB)
* **Estruturas de Dados:**
    * Fila Circular
    * Fila FIFO
* **Bibliotecas:**
    * `conio2.h` (para interface gráfica no console)
    * `windows.h` (para a função `Sleep()`)

---

## ⚙️ Como Compilar e Executar

Este projeto foi desenvolvido para o ambiente Windows, devido ao uso das bibliotecas `conio2.h` e `windows.h`.

1.  Clone o repositório.
2.  Tenha um compilador C/C++ (como o do Dev-C++ ou MinGW) configurado com a biblioteca `conio2.h`.
3.  Compile o programa (o comando exato pode depender da sua IDE, mas via linha de comando seria algo como):
    ```bash
    gcc nome_do_arquivo.c -o simulador.exe -lconio2
    ```
4.  Execute o arquivo compilado:
    ```bash
    ./simulador.exe
    ```
5.  Durante a execução, pressione **'A'** para adicionar novos processos e observe o log.
