#include "ppos.h"
#include "ppos-core-globals.h"

// ****************************************************************************
// Coloque aqui as suas modificações, p.ex. includes, defines variáveis,
// estruturas e funções
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
struct sigaction action;
struct itimerval timer;
void tratador(int signum)
{
    systemTime += 1;
    taskExec->processTime++;
    //incrementa o tempo de processador da tarefa em execucao
    taskExec->remaningTimeTask--;
    //decrementa o tempo restante da tarefa em execucao
    taskExec->quantum--;
    //decrementa o quantum da tarefa em execucao
    if(taskExec->quantum <= 0){
        if(taskExec->remaningTimeTask > 0){
            taskExec->activations++;
            taskExec->quantum = 20;
        }
        scheduler();
    }
    //verifica se o quantum da tarefa em execucao acabou e se acabou migra para proxima tarefa
    //que obdece as politicas do escalonador, e incrementa activations, pois, se ainda resta tempo
    //de execucao ele voltara para o processamento novamente.
}

void task_set_eet(task_t *task, int et)
{
    if (task == NULL)
    {
        taskExec->timeEstimate = task->timeEstimate - taskExec->processTime;
        taskExec->remaningTimeTask = taskExec->timeEstimate;
        //reajusta o tempo estimado da tarefa em execucao de acordo com o tempo passado
    }
    else
    {
        task->timeEstimate = et;
        task->remaningTimeTask = et;
    }
}

int task_get_eet(task_t *task)
{
    if (task != NULL)
    {
        return task->timeEstimate;
    }
    else
    {
        return taskExec->timeEstimate;
    }
    //retorna o tempo estimado de execucao
}

int task_get_ret(task_t *task)
{
    if (task != NULL)
    {
        return task->remaningTimeTask;
    }
    else
    {
        return taskExec->remaningTimeTask;
    }
    //retorna o tempo que falta de execucao
}
task_t *organiza()
{
    if (readyQueue != NULL)
    {
        
        task_t *shortestTask = readyQueue;
        //tarefa aponta para readyqueue
        task_t *currentTask = shortestTask;
        //tarefa aux
        int i = 0;
        //verifica a quantidade de tasks existentes 
        while (i < countTasks)
        {
            //verifica se o tempo é menor que 0, pois se for não é uma tarefa do usuario e ai retorna a prox tarefa
            if(shortestTask->remaningTimeTask < 0){
                shortestTask = currentTask; 
            }
            if ((task_get_ret(currentTask) < task_get_ret(shortestTask))){
                //verifica se a tarefa atual é menor que a proxima
                if (currentTask->remaningTimeTask > 0){
                    shortestTask = currentTask;
                //verifica se currentTask é maior q o e atribui a shortest
                }  
            }  

            i++;
            currentTask = currentTask->next;
        }
        if (shortestTask->id == taskExec->id)
        {
            //verifica se a task passada como menor não é a  task em exec se for ela retorna a task em exec
            //e define um quantum novo a ela
            readyQueue = taskExec;
            readyQueue->quantum = 20;
            return readyQueue;
            
        }
        //atribui a tarefa de menor tempo a readyqueue e define um quantum novo a ela
        readyQueue = shortestTask;
        readyQueue->quantum = 20;
        return readyQueue;
    }
    return NULL;
}


// ****************************************************************************

task_t *scheduler()
{
    if (readyQueue != NULL)
    {
        return organiza();
    }

    return NULL;
}


void before_ppos_init()
{
    // put your customization here

#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

void after_ppos_init()
{
    // put your customization here
    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }
#ifdef DEBUG
    printf("\ninit - AFTER");
#endif
}

void before_task_create(task_t *task)
{
    // put your customization here
    task->execTime = 99999;//tempo de execucao padrão
#ifdef DEBUG
    //printf("\ntask_create - BEFORE - [%d]", task->id);
#endif
}
void after_task_create(task_t *task)
{
    // put your customization here
    if (task->id == 2)
    {
        systemTime = 0; //zera o tempo do system a partir do momento que é criado a primeira task do usuario
    }
    task->activations = 0;
    task->processTime = 0;
    task->execTime = systime();
    task->quantum = 20;
    //inicializa as métricas

#ifdef DEBUG
    printf("\ntask_create - AFTER - [%d] tempo do sistema: %ld\n", task->id, countTasks);
#endif
}

void before_task_exit()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_exit()
{
    // put your customization here
    taskExec->quantum = 0;
    //zera o qunatum da tarefa terminada e printa as métricas dela
    printf("\nTASK %d teve tempo de execução da tarefa anterior: %d e de processador: %d e %d activations\n"
    ,taskExec->id, (systime() - taskExec->execTime), (taskExec->processTime), (taskExec->activations));
#ifdef DEBUG
    printf("\ntask_exit - AFTER- [%d]", taskExec->id);
#endif
}

void before_task_switch(task_t *task)
{
#ifdef DEBUG
    printf("\ntask_switch - BEFORE - [%d -> %d]", taskExec->id, task->id);
#endif
}

void after_task_switch(task_t *task)
{
    //printf("\nA task atual tem um quantum de: [%d]\n", taskExec->quantum);
    //incrementa activations para falar que a tarefa passada recebera o processador mais uma vez
    task->activations++;
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - AFTER - [%d -> %d]", taskExec->id, task->id);
#endif
}

void before_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - BEFORE - [%d] %d", taskExec->id);
#endif
}
void after_task_yield()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - AFTER - [%d]", taskExec->id);
#endif
}

void before_task_suspend(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - BEFORE - [%d]", task->id);
#endif
}

void after_task_suspend(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - AFTER - [%d]", task->id);
#endif
}

void before_task_resume(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - BEFORE - [%d]", task->id);
#endif
}

void after_task_resume(task_t *task)
{
    // put your customization here
#ifdef DEBUG
#endif
}

void before_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_sleep()
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - AFTER - [%d]", taskExec->id);
#endif
}

int before_task_join(task_t *task)
{
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_task_join(task_t *task)
{
#ifdef DEBUG
#endif
    return 0;
}

int before_sem_create(semaphore_t *s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_create(semaphore_t *s, int value)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_down(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_down(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_up(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_up(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_destroy(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_destroy(semaphore_t *s)
{
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_create(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_create(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_lock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_lock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_unlock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_unlock(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_destroy(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_destroy(mutex_t *m)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_create(barrier_t *b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_create(barrier_t *b, int N)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_join(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_join(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_destroy(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_destroy(barrier_t *b)
{
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_create(mqueue_t *queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_create(mqueue_t *queue, int max, int size)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_send(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_send(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_recv(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_recv(mqueue_t *queue, void *msg)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_destroy(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_destroy(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_msgs(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_msgs(mqueue_t *queue)
{
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}
