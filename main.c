#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <search.h>

#define TIME_SLICE 1
#define MAX_USERS 7
#define MAX_PROCESSES 10
#define MAX_BURST_TIME 15

// 3 user1, user2 (3 * cuanta)
    // 2 ... (2 * cuanta)
    // 1 ... (1 * cuanta)
    // 0 ... (0.8 * cuanta)
    // -1 ... (0.6 * cuanta)
    // -2 ... (0.3 * cuanta)
    // -3 ... (0.1 * cuanta)

double arr_ponderi[8] = {0.1, 0.3, 0.6, 0.8, 1, 2, 3};

struct InputList {
    int timp;
    char nume[32];
    int pid;
    int pondere;
}lista_input[1024];

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
    char username[32];
    double pondere;
    struct ProcessList* process_list; 
    struct User* next;
    struct User* prev;
};

struct UserList {
    struct User *head, *tail;
    int size;
};

struct info_hash {
    struct User* user_ptr;
}info_instance[1024], *info_ptr = info_instance; //info_instance - loc unde sa pun informatiile (values) din hash

char strs[1024 * 32], *ptr_str = strs;
int id_gen = 0;
int input_counter = 0;
int input_list_size;

double min_double(double a, double b)
{
    if(a > b)
        return b;
    return a;    
}

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

// ADAUGA USER LA FINALUL LISTEI DE USERI
void add_user_to_userlist(struct UserList* user_list, int user_id, char username_[32], double pondere_) {
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

// void genereaza_useri(struct UserList* lista_useri)
// {

//     srand(time(NULL));
//     int nr_useri = rand() % MAX_USERS + 1;

//     printf("NR USERI: %d\n", nr_useri);

//     for(int i = 0; i < nr_useri; i++)
//     {

//         double p = ((double)rand() / (double)(RAND_MAX));

//         char username[10];

//         sprintf(username, "user%d", i);

//         add_user_to_userlist(lista_useri, i, username, p);

//         int nr_processes = rand() % MAX_PROCESSES + 1;

//         printf("userul %d are %d procese\n", i, nr_processes);

//         for(int j = 0; j < nr_processes; j++)
//         {

//             double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);

//             pid_t pid = fork();


//             // executare proces

//             if(pid == 0)
//             {

//                 char *argumente[2] = {"/bin/ls", NULL};

//                 execve("/bin/ls", argumente, NULL);
//             }


//             add_process(lista_useri->tail, pid, burst_time);




//         }

        

//     }

// }

void actualizeaza_lista(struct UserList* lista_useri, time_t timp_initial){
    time_t timp_curent = time(NULL);
    time_t diferenta_timp = timp_curent - timp_initial;
    while (diferenta_timp >= lista_input[input_counter].timp && input_counter < input_list_size){
        ENTRY item;
        ENTRY *found_item;
        strcpy(ptr_str, lista_input[input_counter].nume);
        item.key = ptr_str;
        if((found_item = hsearch(item, FIND)) == NULL) {
            //double pondere = ((double)rand() / (double)(RAND_MAX));
            add_user_to_userlist(lista_useri, ++ id_gen, lista_input[input_counter].nume, lista_input[input_counter].pondere);
            double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);
            if(((double)rand() / (double)(RAND_MAX)) < (double)0.1)
                burst_time = (double)__INT_MAX__;
            printf("burst time-ul procesului %d al userului %s este %f\n", lista_input[input_counter].pid, lista_useri->tail->username, burst_time);
            add_process(lista_useri->tail, lista_input[input_counter].pid, burst_time);
            info_ptr->user_ptr = lista_useri->tail;
            item.data = info_ptr;

            ptr_str += strlen(ptr_str) + 1;
            info_ptr ++;
            hsearch(item, ENTER);
        }
        
        else {
            struct User* user = ((struct info_hash *)found_item->data)->user_ptr;
            double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);
            if(((double)rand() / (double)(RAND_MAX)) < (double)0.1)
                burst_time = (double)__INT_MAX__;
            printf("burst time-ul procesului %d al userului %s este %f\n", lista_input[input_counter].pid, user->username, burst_time);
            add_process(user, lista_input[input_counter].pid, burst_time);
        }

        input_counter++;
    } 
}

void citire_fisier(){
    int numar_intrari, timp_input, pid_user, pondere_user;
    char dummy, nume_user[32];
    FILE* fin;
    fin = fopen ("date.in", "r");
    fscanf(fin, "%d", &numar_intrari);
    input_list_size = numar_intrari;
    for (int i=0; i<numar_intrari; i++){
        fscanf(fin, "%d%c %s %d %d", &timp_input, &dummy, nume_user, &pid_user, &pondere_user);
        lista_input[i].timp = timp_input;
        strcpy(lista_input[i].nume, nume_user);
        lista_input[i].pid = pid_user;
        lista_input[i].pondere = pondere_user;
    }
    fclose(fin);
}

void round_robin(struct UserList* lista_useri)
{
    struct User* user = lista_useri->head;
    time_t timp_initial = time(NULL);
    while(lista_useri->size || input_counter < input_list_size) // la un moment dat pot ramane fara procese in lista, dar poate mai sunt altele care asteapta sa fie inserate in lista
    {
        actualizeaza_lista(lista_useri, timp_initial);
        // if(lista_useri->size == 0)
        //     printf("lista goala\n");
        printf("lista_useri->size: %d\n", lista_useri->size);
        printf("Waiting...\n");
        double timp_minim = min_double(user->process_list->head->burst_time, user->pondere * TIME_SLICE);
        sleep(timp_minim); 
        printf("s-a executat procesul %d al utilizatorului %s timp de %f s\n", user->process_list->head->pid, user->username, timp_minim);
        user->process_list->head->burst_time -= user->pondere * TIME_SLICE;
        if(user->process_list->head->burst_time <= 0)
            pop_process_user(user);

        if(!user->nr_proc)
            pop_user_from_userlist(lista_useri, user);

        user = user->next;
    }
    printf("\n\n=======ALGORITMUL S-A TERMINAT=======\n");
}

int main() {
    // lista de useri pe mai multe nivele in functie de prioritati
    // pentru fiecare user trebuie sa parcurgem toate procesele user-ului cu pondere * cuanta
    // 3 user1, user2 (3 * cuanta)
    // 2 ... (2 * cuanta)
    // 1 ... (1 * cuanta)
    // 0 ... (0.8 * cuanta)
    // -1 ... (0.6 * cuanta)
    // -2 ... (0.3 * cuanta)
    // -3 ... (0.1 * cuanta)
    srand(time(NULL));

    citire_fisier();

    for(int i = 0; i < 6; ++ i){
        printf("%d %s %d\n", lista_input[i].timp, lista_input[i].nume, lista_input[i].pid);
    }

    hcreate(1024);

    struct UserList* lista_utilizatori = create_user_list();

    // genereaza_useri(lista_utilizatori);

    actualizeaza_lista(lista_utilizatori, time(NULL));

    round_robin(lista_utilizatori);

    hdestroy();


    return 0;
}
