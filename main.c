/*
    UNIVERSIDADE FEDERAL DA PARAÍBA – CENTRO DE INFORMÁTICA
    LPII - Programação Concorrente - 2020.2 EOO2
    Prof. Carlos Eduardo Batista

    Aluno: Jackson Leandro do Nascimento
    Matricula: 20180019614
*/

#include <stdio.h>   /* Input/Output */
#include <stdlib.h>  /* General Utilities */
#include <pthread.h> /* POSIX Threads */
#include <unistd.h>  /* Symbolic Constants */

#define QTD_PASSAGEIROS 50 //Quantidade de passageiros
#define QTD_VOLTAS 4       //Quantidade de voltas

//Declaração da variavel de controle de barreira, do tipo pthread barrie t
pthread_barrier_t controle_barreira;

//Variaveis para a manipulação do ticket.
int number = 1;
int next = 1;
int turn[3];
int lock = 0;

int cpd_carro = 0;              //Variavel que guarda a capacidade dos carros
int fechouParque = 0;           //Condição que vai no ciclo da thread dos passageiros
int existemPassageirosNoParque; //Condição que vai no cicle da thread do carro
int volta = 0;                  //Numero de voltas dadas no parque
int contador = 1;               //Variavel que serve como contador para algun ciclos
int passeia;                    //Variavel que guarda o tempo aleatorio
int gatilho = 0;                //Variavel que serve como criteiro de entrada em um if da seção critica da thread dos passageiros

int id_passageiros[11]; //Guarda o ID dos passageiros para identificação da saida dos passageiros

//Declaração das funções de mapiluação que iram nas thread, as suas implementações estão após a main
void entraNoCarro();
void esperaEncher();
void saiDoCarro();
void esperaEsvaziar();
void esperaVoltaAcabar();
void daUmaVolta();
void passeiaPeloParque();

//Thead do carro
void *carro(void *p)
{
    long idx = (long)p;

    while (existemPassageirosNoParque)
    {
        //while (__sync_lock_test_and_set(&lock, 1))
        ; // protocolo de entrada
        // Inicio da seção critica
        puts("...");
        //T_CARRO é para mostrar no console os processos da thread carro

        printf("[T_CARRO]---Faltam %d voltas para o parque fechar---\t[T_CARRO]\n", 4 - volta);
        printf("Enchendo o carro...\t[T_CARRO]\n");
        esperaEncher();
        printf("Dando uma volta");
        fflush(stdout);
        daUmaVolta();
        puts("");
        printf("A volta acabou, se preparando para esvaziar\t[T_CARRO]\n");
        printf("Esvaziando...\n");
        esperaEsvaziar();
        printf("Todos os passageiros sairam.\t[T_CARRO]\n");
        sleep(1);
        __sync_fetch_and_add(&volta, 1);
        //volta++;
        sleep(passeia);

        //Fim da seção critica
        //lock = 0; // protocolo de saida
    }
}

//Thead dos passageiros
void *passageiros(void *p)
{
    long idx = (long)p;

    while (!fechouParque)
    {

        /*
            Fazendo um teste em cima de uma posição especifica do array turn[idx]
            Para evitar problema de contenção de memoria.
            Fazendo com que cada thread acesse uma posição distinta.
            Essa tecnica é bastante conhecida, e usada, como Exclusão Multua
            que serve para evitar que dois processos ou threads tenham 
            acesso simultaneamente a um recurso compartilhado, acesso esse denominado 
            por seção crítica.
            Assim, cada thread obtem uma entrada eventual com ausencia de atraso desnecessario,
            que é um processo tentando entrar em sua seção critica e obtendo sucesso
            caso os outras já tenham saido das seções criticas deles.
        */
        turn[idx] = __sync_fetch_and_add(&number, 1);
        while (!(turn[idx] == next))
            ;

        //Dentro(inicio) da secao critica

        sleep(1);
        if (gatilho == 1) //Este gatilho é ativado na função "daUmaVolta" na thread do carro
        {
            //T_PASSAGEIROS é para mostrar no console os processos da thread passageiro
            esperaVoltaAcabar(); //Espera a volta acabar
            printf("Os passageiros estão saindo...\t[T_PASSAGEIROS]\n");
            saiDoCarro(); //Tira os passageiros do carro
            printf("Passeiando pelo parque\t[T_PASSAGEIROS]\n");
            passeiaPeloParque(); //Tempo aleatorio para passeio
            gatilho = 0;         //Seta o gatilho para 0
        }
        __sync_fetch_and_add(&next, 1);

        //Fora da seção critica

        int wait_ret;
        entraNoCarro();
        if (cpd_carro % 11 != 0)
        {
            id_passageiros[cpd_carro] = idx;
            printf("Passageiro [%ld] entrou no carro e ocupou a cadeira %d\t[T_PASSAGEIROS]\n", idx, cpd_carro);

            //Esta barreira para 10 treads, que equivale ao numero de vagas no carro da
            //montanha russa
            wait_ret = pthread_barrier_wait(&controle_barreira);

            /*
                Eu implementei a barreira fora da seção critica, pois dentro dela processos
                estava ocorrendo Deadlock que é uma situação em que ocorre um impasse, e dois ou mais 
                processos ficam impedidos de continuar suas execuções - ou seja, ficam bloqueados, 
                as threads esperada um pela outra, e o programa ficava bloqueado.
            */

            if ((wait_ret != 0) && (wait_ret != PTHREAD_BARRIER_SERIAL_THREAD))
            {
                printf("Barrier ERROR\n");
                exit(0);
            }
        }
    }
}

int main(void)
{
    //Declaração das theads, sendo threads[0] o carro, e os demais os passageiros
    pthread_t threads[QTD_PASSAGEIROS];
    srand(time(NULL)); //Setando o tempo em NULL para o uso da função rand()

    /*
      Declaração da barreira que vai parar 10 os passageiros que sairem da seção critica.
      Esta barreira é serve caso queira que algum passageiro que já deu o passeia
      venha a dar novamente. Porém, utilizando desta forma,  e com inumeras compilações
      pude observar que o consumo de processador se mantinha a cima de 80%.
      A vantagem de utilizar deste modo, é que sempre irá ter passageiros suficientes
      para encher o carrinho. Já que após 10 threads, a barreira libera as threads
      a na proxima volta, o carrinho sempre terá mais de 10 passageiros para rodar.
    */
    //COMENTE PARA TESTAR A OUTRA BARREIRA
    int init_bar_ret = pthread_barrier_init(&controle_barreira, NULL, 10);

    /*
      Declaração da barreia que vai barrar todas threads passageiros que sairem da seção critica.
      Já parando todas as threads, evitando com que algum passageiro dê outra volta, o consumo
      de processador, em meu computador, variava, pude observar que em momentos
      este consumo estava em 60%, e outros com 99%, e no final ficava entre 50% e 70%.
      A desvantagem de utilizar deste modo é que o carrinho ocupa 10 vagas, e tem um limite de 
      4 voltas. Se utilizar 49 passageiros, no final irá ocorrer um loop no esperaEncherOCarro, 
      pois não tem mais barreira para parar (contabilizar).
    */

    //DESCOMENTE PARA TESTAR
    //int init_bar_ret = pthread_barrier_init(&controle_barreira, NULL, QTD_PASSAGEIROS);

    /*Tratamento de erro, caso init_bar_ret for zero, significa que a barreira
      foi criada com sucesso*/
    if (init_bar_ret != 0)
    {
        printf("PTHREAD_BARRIER init error!\n");
        exit(0);
    }

    volta = 0;

    for (int i = 1; i < QTD_PASSAGEIROS + 1; i++)
    {
        turn[i] = 0;
    }

    existemPassageirosNoParque = 1;

    //Declaração das threads
    pthread_create(&threads[0], NULL, carro, (void *)0); //Thread do carro, onde é o indice 0

    //Thread dos passageiros, onde o indice é de 1 até QTD_PASSAGEIROS
    for (long i = 1; i < QTD_PASSAGEIROS; i++)
        pthread_create(&threads[i], NULL, passageiros, (void *)i);

    //Ciclo de parada, onde as threads iram rodar até volta ser QTD_VOLTAS (neste caso 4 voltas)
    while (volta != QTD_VOLTAS)
        //usleep(10)
        ;

    //Mensagens após o programa acabar
    printf("---Voltas dadas: %d---\n", volta);
    printf("As voltas acabaram\n");
    printf("O parque fechou\n");

    return 0;
}

//Implementações das funções de manipulação das threads
//Está função incrementa 1 unidade a cada thread que chegar na barreira
void entraNoCarro()
{
    cpd_carro = cpd_carro + 1;
}
//Está função espera encher, com um ciclo que só sai dele após a função
//Antrior incrementar 10 unidades a variavel cpd_carro
void esperaEncher()
{
    while (cpd_carro != 10)
        ;
}
//Está função é um ciclo que espera todos os passageiros sair do carro
//Neste caso até cpd_carro ser 0
void esperaEsvaziar()
{
    while (cpd_carro > 0)
        ;
}

//Está função decrementa 1 unidade do cpd_carro e imprime os passageiros que
//Sairam e suas cadeiras vagas
void saiDoCarro()
{
    contador = 1; //Utiliação da variavel contador contador
    while (contador % 11 != 0)
    {
        printf("Passageiro [%d] saiu do carro e desocupou a cadeira %d\t[T_PASSAGEIROS]\n", id_passageiros[cpd_carro--], cpd_carro);
        // fflush(stdout);
        contador++;
        sleep(1);
    }
}

//Está função espera a volta acabar, um tempo de 6 segundos, neste caso de 1 até 7.
void esperaVoltaAcabar()
{
    contador = 1;
    while (contador % 7 != 0)
    {
        contador++;
        printf(".");
        fflush(stdout);
        //Função que força o programa escrever o que está no buffer
        //Neste caso ela está sendo utiliada para fazer uma animação de "espera", imprimindo
        //pontos em 1 e 1 segundo
        sleep(1);
    }
}

//Função que roda em consonância com a função esperaVoltaAcabar()
//É um ciclo que espera o contador anterior
void daUmaVolta()
{
    gatilho = 1;
    while (contador % 7 != 0)
        ;
}
//Função que gera um tempo de espera aleatorio de 10seg até 25seg
void passeiaPeloParque()
{
    passeia = rand() % 10 + 15;
    sleep(passeia);
}
