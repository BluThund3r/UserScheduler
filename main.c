#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define TIME_SLICE 10
#define MAX_USERS 7
#define MAX_PROCESSES 10
#define MAX_BURST_TIME 15


double min_double(double a, double b)
{
    if(a > b)
        return b;
    return a;    
}


struct Process {
    pid_t pid;
    double burst_time;
    struct Process* next;
    struct Process* prev;
};

struct ProcessList {
    struct Process *head, *tail;
};

struct User {
    int id;
    int nr_proc;
    char username[10];
    double pondere;
    struct ProcessList* process_list; 
    struct User* next;
    struct User* prev;
};

struct UserList {
    struct User *head, *tail;
    int size;
};

struct ProcessList* create_process_list() {
    struct ProcessList* list = (struct ProcessList*)malloc(sizeof(struct ProcessList));
    list->tail = NULL;
    list->head = NULL;
    return list;
}

struct Process* alloc_process() {
    struct Process* rez = (struct Process*)malloc(sizeof(struct Process));
    if(!rez)
        perror("Eroare la alocarea procesului!");
    return rez;
}

void add_process(struct User *user, pid_t pid, double burst_time) {
    struct Process* temp = alloc_process();
    temp->pid = pid;
    temp->burst_time = burst_time;
    temp->next = NULL;
    temp->prev = NULL;
    if(!user->nr_proc) {
        user->process_list = create_process_list();
        user->process_list->head = temp;
        user->process_list->tail = temp;
    }

    else {
        temp->prev = user->process_list->tail;
        user->process_list->tail->next = temp;
        user->process_list->tail = temp;
    }

    ++ user->nr_proc;
}

void pop_process_user(struct User* user) {
    struct Process* temp = user->process_list->head;
    user->process_list->head = user->process_list->head->next;
    free(temp);
    -- user->nr_proc;
}

struct UserList* create_user_list() {
    struct UserList* list = (struct UserList*)malloc(sizeof(struct UserList));
    list->tail = NULL;
    list->head = NULL;
    list->size = 0;
    return list;
}

struct User* alloc_user() {
    struct User* rez = (struct User*)malloc(sizeof(struct User));
    if(!rez)
        perror("Eroare la alocarea user-ului!");
    return rez;
}

void add_user_to_userlist(struct UserList* user_list, int user_id, char username_[10], double pondere_) {
    struct User* temp = alloc_user();
    temp->id = user_id;
    strcpy(temp->username, username_);
    temp->pondere = pondere_;
    temp->nr_proc = 0;
    temp->process_list = NULL;
    if(!user_list->size) {
        user_list->head = user_list->tail = temp; 
        temp->next = temp;
        temp->prev = temp;
    }

    else {
        temp->prev = user_list->tail;
        temp->next = user_list->head;
        user_list->head->prev = temp;
        user_list->tail->next = temp;
        user_list->tail = temp;
    }

    ++ user_list->size;
}

void pop_user_from_userlist (struct UserList* user_list, struct User* user) {
    if(!user_list->size) {
        perror("Lista nu are elemente, deci nu puteti sterge!");
        return;
    }

    else if(user_list->size == 1) 
        user_list->head = user_list->tail = NULL;
    
    else {
        user->next->prev = user->prev;
        user->prev->next = user->next;
        if(user == user_list->tail) 
            user_list->tail = user->prev;
        else if(user == user_list->head)
            user_list->head = user->next;
        
        free(user);
    }

    -- user_list->size;
}

void genereaza_useri(struct UserList* lista_useri)
{

    srand(time(NULL));
    int nr_useri = rand() % MAX_USERS + 1;

    printf("NR USERI: %d\n", nr_useri);

    for(int i = 0; i < nr_useri; i++)
    {

        double p = ((double)rand() / (double)(RAND_MAX));

        char username[10];

        sprintf(username, "user%d", i);

        add_user_to_userlist(lista_useri, i, username, p);

        int nr_processes = rand() % MAX_PROCESSES + 1;

        printf("userul %d are %d procese\n", i, nr_processes);

        for(int j = 0; j < nr_processes; j++)
        {

            double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);

            pid_t pid = fork();


            // executare proces

            if(pid == 0)
            {

                char *argumente[2] = {"/bin/ls", NULL};

                execve("/bin/ls", argumente, NULL);
            }


            add_process(lista_useri->tail, pid, burst_time);




        }

        

    }

}

void round_robin(struct UserList* lista_useri)
{
    struct User* user = lista_useri->head;

    while(lista_useri->size)
    {
        printf("s-a executat procesul %d al utilizatorului %s timp de %f\n", user->process_list->head->pid, user->username, min_double(user->process_list->head->burst_time, user->pondere * TIME_SLICE));
        user->process_list->head->burst_time -= user->pondere * TIME_SLICE;
        if(user->process_list->head->burst_time <= 0)
            pop_process_user(user);

        if(!user->nr_proc)
            pop_user_from_userlist(lista_useri, user);

        user = user->next;
    }

}

int main() {

    struct UserList* lista_utilizatori = create_user_list();

    genereaza_useri(lista_utilizatori);

    round_robin(lista_utilizatori);


    return 0;
}
