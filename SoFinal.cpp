#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>  // Para o Sleep
#include <conio2.h>   // Para gotoxy, textcolor e textbackground

#define MAXFILA 10
#define MAX_QUANTUM 10  // Limite máximo de tempo que um processo pode executar de uma vez
#define MAX_FILHOS_POR_PROCESSO 2  // Limite de filhos que cada processo pode criar
#define MAX_PROFUNDIDADE 3  // Limite de gerações (pai, filho, neto)

// Estruturas para processos e filas
struct ProcessoControl {
    int id;
    int TimeRest;  // tempo restante para execução completa
    int TimeExec;  // tempo já executado
    int TimeParado;  // tempo parado (tempo na fila de prontos)
    int isChild;   // flag para indicar se é processo filho
    int waitingForChild;  // flag para indicar se o processo pai está esperando um filho
    int childId;   // armazena o ID do filho que o pai está esperando
    int timesBlocked;  // número de vezes que foi bloqueado
    int totalBlockedTime;  // tempo total que passou bloqueado
    int timeInReadyQueue;  // tempo total na fila de prontos
    int numFilhos;  // número de filhos criados
    int childBlockedTime;  // tempo de bloqueio/espera devido à criação de filhos
    int profundidade;  // Nível de profundidade na árvore de processos
};
// Fila Circular (Pronto)
struct TpFilaCircular {
    int inicio, fim, tamanho;
    ProcessoControl fila[MAXFILA];
};

// Fila FIFO (Espera e Execução)
struct TpFilaFIFO {
    int inicio, fim, tamanho;
    ProcessoControl fila[MAXFILA];
};

// Variáveis globais para estatísticas
int totalProcessosFinalizados = 0;
int totalProcessosBloqueados = 0;
int totalTempoBloqueio = 0;
int totalEntreExecucaoPronto = 0;

// Inicializa a fila circular
void initFilaCircular(TpFilaCircular &f) {
    f.inicio = 0;
    f.fim = 0;
    f.tamanho = 0;
}

// Inicializa a fila FIFO
void initFilaFIFO(TpFilaFIFO &f) {
    f.inicio = 0;
    f.fim = 0;
    f.tamanho = 0;
}

// Verifica se a fila circular está vazia
int emptyFilaCircular(TpFilaCircular &f) {
    return f.tamanho == 0;
}

// Verifica se a fila FIFO está vazia
int emptyFilaFIFO(TpFilaFIFO &f) {
    return f.tamanho == 0;
}

// Adiciona um elemento na fila circular
int pushFilaCircular(TpFilaCircular &f, ProcessoControl p) {
    if (f.tamanho < MAXFILA) {
        f.fila[f.fim] = p;
        f.fim = (f.fim + 1) % MAXFILA;
        f.tamanho++;
        return 0;
    } else {
        printf("Fila circular cheia!\n");
        return -1;
    }
}

// Adiciona um elemento na fila FIFO
int pushFilaFIFO(TpFilaFIFO &f, ProcessoControl p) {
    if (f.tamanho < MAXFILA) {
        f.fila[f.fim] = p;
        f.fim = (f.fim + 1) % MAXFILA;
        f.tamanho++;
        return 0;
    } else {
        printf("Fila FIFO cheia!\n");
        return -1;
    }
}

// Remove um elemento da fila circular
int popFilaCircular(TpFilaCircular &f, ProcessoControl &ret) {
    if (!emptyFilaCircular(f)) {
        ret = f.fila[f.inicio];
        f.inicio = (f.inicio + 1) % MAXFILA;
        f.tamanho--;
        return 0;
    } else {
        return -1;
    }
}

// Remove um elemento da fila FIFO
int popFilaFIFO(TpFilaFIFO &f, ProcessoControl &ret) {
    if (!emptyFilaFIFO(f)) {
        ret = f.fila[f.inicio];
        f.inicio = (f.inicio + 1) % MAXFILA;
        f.tamanho--;
        return 0;
    } else {
        return -1;
    }
}

// Sorteio para criar filhos
int sorteio_filho() {
    return rand() % 2;
}

//Processo de criar filhos
ProcessoControl criarProcessoFilho(int paiId, int profundidadePai) {
    static int contadorFilhos = 1;  // Gera IDs únicos para os filhos
    ProcessoControl filho;
    filho.id = 100 + contadorFilhos++;
    filho.TimeRest = rand() % 20 + 5;   // Tempo de execução aleatório entre 5 e 25
    filho.TimeExec = 0;
    filho.TimeParado = 0;
    filho.isChild = 1;  // Marca como processo filho
    filho.waitingForChild = 0;
    filho.childId = -1;
    filho.timesBlocked = 0;
    filho.totalBlockedTime = 0;
    filho.timeInReadyQueue = 0;
    filho.numFilhos = 0;
    filho.childBlockedTime = 0;
    filho.profundidade = profundidadePai + 1;  // Incrementa a profundidade para netos
    return filho;
}

//-------------------------------------------------------------------------------------//


// Função para adicionar processo pelo usuário
void adicionarProcessoUsuario(TpFilaCircular &filaProntos, ProcessoControl processos[], int &processosCount) {
    ProcessoControl novoProcesso;
    
    // Definindo os atributos do novo processo
    novoProcesso.id = processosCount + 1;  // Gera ID com base no total de processos
    printf("Adicionando um novo processo...\n");
    printf("Digite o tempo total de execucao para o processo (entre 5 e 30): ");
    scanf("%d", &novoProcesso.TimeRest);  // Define o tempo de execução restante

    // Valida o tempo inserido
    if (novoProcesso.TimeRest < 5 || novoProcesso.TimeRest > 30) {
        printf("Tempo invalido. Definindo valor padrao de 10 unidades de tempo.\n");
        novoProcesso.TimeRest = 10;
    }

    novoProcesso.TimeExec = 0;
    novoProcesso.TimeParado = 0;
    novoProcesso.isChild = 0;
    novoProcesso.waitingForChild = 0;
    novoProcesso.childId = -1;
    novoProcesso.timesBlocked = 0;
    novoProcesso.totalBlockedTime = 0;
    novoProcesso.timeInReadyQueue = 0;
    novoProcesso.numFilhos = 0;
    novoProcesso.childBlockedTime = 0;
    novoProcesso.profundidade = 0;  // É um processo pai, porta	nto profundidade 0

    // Adiciona o processo à lista de processos e à fila de prontos
    processos[processosCount] = novoProcesso;
    processosCount++;
    pushFilaCircular(filaProntos, novoProcesso);

    printf("Novo processo %d adicionado com sucesso à fila de Pronto.\n", novoProcesso.id);
}


// Função para finalizar o filho e desbloquear o pai
void finalizarFilhoEDesbloquearPai(TpFilaFIFO &filaBloqueados, TpFilaCircular &filaProntos, int childId) {
    for (int i = filaBloqueados.inicio; i != filaBloqueados.fim; i = (i + 1) % MAXFILA) {
        ProcessoControl &pai = filaBloqueados.fila[i];
        if (pai.waitingForChild && pai.childId == childId) {
            pai.waitingForChild = 0;  // Desbloqueia o pai
            pai.childId = -1;
            pushFilaCircular(filaProntos, pai);
            printf("Processo pai %d foi desbloqueado e movido para a fila de Pronto.\n", pai.id);
        }
    }
}

// Exibe as estatísticas finais ao final da simulação
// Exibe as estatísticas finais ao final da simulação
void exibirEstatisticas(ProcessoControl processos[], int processosCount) {
    int totalTempoExecucao = 0;
    printf("\nEstatisticas Finais:\n");

    // Quantidade de processos finalizados
    printf("Quantidade de processos finalizados: %d\n", totalProcessosFinalizados);

    // Processos bloqueados e tempo médio de bloqueio
    if (totalProcessosBloqueados > 0) {
        float mediaBloqueio = (float)totalTempoBloqueio / totalProcessosBloqueados;
        printf("Quantidade de processos bloqueados: %d\n", totalProcessosBloqueados);
        printf("Tempo médio de bloqueio: %.2f\n", mediaBloqueio);
    }

    // Processos entre execução e pronto (não bloqueados)
    printf("Quantidade de processos que alternaram entre Execução e Pronto: %d\n", totalEntreExecucaoPronto);

    // Tempo total de execução de cada processo
    printf("Tempo total de execução de cada processo (incluindo espera):\n");
    for (int i = 0; i < processosCount; i++) {
        // Calcular o tempo total de execução
        totalTempoExecucao = processos[i].TimeExec + processos[i].totalBlockedTime + processos[i].timeInReadyQueue;
        
        // Verificar se o totalTempoExecucao está correto
        if (totalTempoExecucao > 0) {
            printf("Processo %d: %d unidades de tempo\n", processos[i].id, totalTempoExecucao);
        } else {
            printf("Processo %d: Nenhum tempo de execução registrado.\n", processos[i].id);
        }
    }

    // Exibir quantidade de filhos criados e tempo de bloqueio devido à criação de filhos
    printf("Quantidade de filhos criados e tempo de bloqueio/espera devido a isso:\n");
    for (int i = 0; i < processosCount; i++) {
        // Verificar se o processo criou filhos
        if (processos[i].numFilhos > 0) {
            printf("Processo %d criou %d filho(s) e ficou bloqueado por %d unidades de tempo esperando os filhos.\n",
                   processos[i].id, processos[i].numFilhos, processos[i].childBlockedTime);
        } else {
            printf("Processo %d nao criou nenhum filho.\n", processos[i].id);
        }
    }
}


int main() {
    srand(time(NULL));

    // Inicializa as filas
    TpFilaCircular filaProntos;
    TpFilaFIFO filaEspera, filaExecucao;
    initFilaCircular(filaProntos);
    initFilaFIFO(filaEspera);
    initFilaFIFO(filaExecucao);

    int numProcessosIniciais = rand() % 10 + 1;

    ProcessoControl processos[100];
    int processosCount = 0;

    // Inicializando processos principais
    for (int i = 0; i < numProcessosIniciais; i++) {
        processos[processosCount] = {i + 1, rand() % 30 + 10, 0, 0, 0, 0, -1, 0, 0, 0, 0};  // Processos principais
        processos[processosCount].profundidade = 0;  // Processo inicial (pai) tem profundidade 0
        pushFilaCircular(filaProntos, processos[processosCount]);
        printf("Processo %d adicionado a fila de Pronto.\n", processos[processosCount].id);
        processosCount++;
    }
    printf("Inciado com: %d processos\n",processosCount);

    // Loop de simulação
    // Loop de simulação
	while (!emptyFilaCircular(filaProntos) || !emptyFilaFIFO(filaEspera)) {
		Sleep(200);
		textcolor(13);
	    // Verificação de tecla pressionada para adicionar um processo
	    if (kbhit()) {  // Verifica se uma tecla foi pressionada
	        char tecla = getch();  // Captura a tecla pressionada
	        if (tecla == 'A' || tecla == 'a') {
	            adicionarProcessoUsuario(filaProntos, processos, processosCount);  // Adiciona um novo processo
	        }
	    }
	
	    ProcessoControl proc;
	
	    // Se houver processos prontos, mova para execução
	    if (!emptyFilaCircular(filaProntos)) {
	        popFilaCircular(filaProntos, proc);
	        pushFilaFIFO(filaExecucao, proc);
	        printf("Processo %d movido para a fila de Execucao.\n", proc.id);
	    }
	
	    // Executa o processo
	    if (!emptyFilaFIFO(filaExecucao)) {
	        popFilaFIFO(filaExecucao, proc);
	
	        int tempoExec = rand() % MAX_QUANTUM + 1;  // Tempo aleatório de execução
	
	        // Criação de filhos ou netos, se ainda houver profundidade permitida
	        if (proc.numFilhos < MAX_FILHOS_POR_PROCESSO && proc.profundidade < MAX_PROFUNDIDADE && sorteio_filho()) {
	            ProcessoControl filho = criarProcessoFilho(proc.id, proc.profundidade);
	            processos[processosCount++] = filho;
	            proc.numFilhos++;
	            printf("Processo %d criou um filho (profundidade %d) com ID %d.\n", proc.id, filho.profundidade, filho.id);
	            pushFilaCircular(filaProntos, filho);
	            printf("Filho %d movido para a fila de Pronto.\n", filho.id);
	
	            // O pai fica bloqueado esperando o filho terminar
	            proc.waitingForChild = 1;
	            proc.childId = filho.id;
	            proc.childBlockedTime += tempoExec;  // Incrementa o tempo de bloqueio devido ao filho
	            pushFilaFIFO(filaEspera, proc);
	            printf("Processo pai %d foi movido para a fila de Espera aguardando o filho %d.\n", proc.id, filho.id);
	        } else {
	            // Processo continua executando
	            if (proc.TimeRest > tempoExec) {
	                proc.TimeRest -= tempoExec;
	                proc.TimeExec += tempoExec;  // Incrementa o tempo de execução
	
	                // Caso o processo precise ser bloqueado
	                if (rand() % 2) {
	                    if (proc.timesBlocked == 0) {  // Verifica se é o primeiro bloqueio
	                        totalProcessosBloqueados++;
	                    }
	                    proc.timesBlocked++;
	                    totalTempoBloqueio += tempoExec;  // Atualiza o tempo total de bloqueio
	                    proc.totalBlockedTime += tempoExec;  // Atualiza tempo bloqueado
	                    pushFilaFIFO(filaEspera, proc);
	                    printf("Processo %d bloqueado e movido para a fila de Espera.\n", proc.id);
	                } else {
	                    totalEntreExecucaoPronto++;
	                    pushFilaCircular(filaProntos, proc);
	                    printf("Processo %d movido de volta para a fila de Pronto com tempo restante %d.\n", proc.id, proc.TimeRest);
	                }
	            } else {
	                proc.TimeExec += proc.TimeRest;  // Finaliza o processo
	                totalProcessosFinalizados++;
	                printf("Processo %d finalizado.\n", proc.id);
	
	                // Desbloqueia o pai se o processo for um filho
	                if (proc.isChild) {
	                    finalizarFilhoEDesbloquearPai(filaEspera, filaProntos, proc.id);
	                }
	            }
	        }
	
	        // Atualizar o processo na lista de processos
	        for (int i = 0; i < processosCount; i++) {
	            if (processos[i].id == proc.id) {
	                processos[i] = proc;  // Atualiza o processo original com as novas informações
	                break;
	            }
	        }
	    }
	
	    // Processos bloqueados voltam para a fila de prontos
	    if (!emptyFilaFIFO(filaEspera)) {
	        ProcessoControl blockedProc;
	        popFilaFIFO(filaEspera, blockedProc);
	        blockedProc.timeInReadyQueue += rand() % MAX_QUANTUM + 1;  // Atualiza o tempo na fila de prontos
	        pushFilaCircular(filaProntos, blockedProc);
	
	        // Atualizar o processo na lista de processos
	        for (int i = 0; i < processosCount; i++) {
	            if (processos[i].id == blockedProc.id) {
	                processos[i] = blockedProc;  // Atualiza o processo original com as novas informações
	                break;
	            }
	        }
	
	        printf("Processo %d desbloqueado e movido para a fila de Pronto.\n", blockedProc.id);
	    }
	}


    // Exibir as estatísticas finais
    exibirEstatisticas(processos, processosCount);

    printf("Simulação finalizada.\n");
    return 0;
}



