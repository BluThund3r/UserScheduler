#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <search.h>

#define TIME_SLICE 3
#define MAX_USERS 7
#define MAX_PROCESSES 10
#define MAX_BURST_TIME 15

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
    int pondere;
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
double arr_ponderi[7] = {0.1, 0.3, 0.6, 0.8, 1, 1.2, 1.5};
struct UserList* niveluri[7];
int no_users = 0;
int arr_number_users[7] = {0};

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

void pop_process_user(struct User* user, struct Process* process) {
    if(user->nr_proc == 1)
        user->process_list->head = user->process_list->tail = NULL;
    else if(process->next == NULL) {
        process->prev->next = NULL;
        user->process_list->tail = process->prev;
    }
    else if(process->prev == NULL) {
        process->next->prev = NULL;
        user->process_list->head = process->next;
    }
    else {
        process->next->prev = process->prev;
        process->prev->next = process->next;
    }

    free(process);
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
    ++ no_users;
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
//TODO: discuta daca ar trebui sa nu mai facem pop din lista de useri, si doar sa sarim peste el in caz de ceva
// void pop_user_from_userlist (struct UserList* user_list, struct User* user) {
//     if(!user_list->size) {
//         perror("Lista nu are elemente, deci nu puteti sterge!");
//         return;
//     }

//     else if(user_list->size == 1) 
//         user_list->head = user_list->tail = NULL;
    
//     else {
//         user->next->prev = user->prev;
//         user->prev->next = user->next;
//         if(user == user_list->tail) 
//             user_list->tail = user->prev;
//         else if(user == user_list->head)
//             user_list->head = user->next;
        
//         free(user);
//     }

//     -- user_list->size;
//     -- no_users;
// }

void actualizeaza_lista(time_t timp_initial){
    time_t timp_curent = time(NULL);
    time_t diferenta_timp = timp_curent - timp_initial;
    while (input_counter < input_list_size && diferenta_timp >= lista_input[input_counter].timp){
        ENTRY item;
        ENTRY *found_item;
        strcpy(ptr_str, lista_input[input_counter].nume);
        item.key = ptr_str;
        int prioritate = lista_input[input_counter].pondere;
        if((found_item = hsearch(item, FIND)) == NULL) {
            add_user_to_userlist(niveluri[prioritate], ++ id_gen, lista_input[input_counter].nume, lista_input[input_counter].pondere);
            arr_number_users[prioritate] ++;
            double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);
            if(((double)rand() / (double)(RAND_MAX)) < (double)0.1)
                burst_time = (double)__INT_MAX__;
            // double burst_time = 6;
            printf("A fost adaugat procesul %d al user-ului %s, avand burst time de %.2fs\n", lista_input[input_counter].pid, niveluri[prioritate]->tail->username, burst_time);
            add_process(niveluri[prioritate]->tail, lista_input[input_counter].pid, burst_time);
            info_ptr->user_ptr = niveluri[prioritate]->tail;
            item.data = info_ptr;

            ptr_str += strlen(ptr_str) + 1;
            info_ptr ++;
            hsearch(item, ENTER);
        }
        
        else {
            struct User* user = ((struct info_hash *)found_item->data)->user_ptr;
            if(!user->nr_proc)
                ++ arr_number_users[prioritate];
            double burst_time = ((double)rand() / (double)(RAND_MAX)) * (double)(MAX_BURST_TIME);
            if(((double)rand() / (double)(RAND_MAX)) < (double)0.1)
                burst_time = (double)__INT_MAX__;
            // double burst_time = 6;
            printf("A fost adaugat procesul %d al user-ului %s, avand burst time de %.2fs\n", lista_input[input_counter].pid, user->username, burst_time);
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

void round_robin()
{
    int iterator;
    for(int i = 0; i < 7; ++ i)
        if(arr_number_users[i]) {
            iterator = i;
            break;
        }

    time_t timp_initial = time(NULL);
    int lista_useri_vida = 0;
    // while-ul asta ar trebui sa fie ceva gen "cat timp mai am procese undeva in niveluri[] (o variabila globala maybe) || ...rest..."
    while(no_users || input_counter < input_list_size) // la un moment dat pot ramane fara procese in lista, dar poate mai sunt altele care asteapta sa fie inserate in lista
    {
        do {        // asteapta pana cand lista_useri are cel putin un user si mai apoi lasa algoritmul sa continue
            actualizeaza_lista(timp_initial);
            // printf("AM RAMAS BLOCAT %ld\n", time(NULL) - timp_initial);
            if(lista_useri_vida && no_users){  // ma asigura ca stiu cine este urmatorul user in cazul in care lista era vida si am adaugat acum un proces nou
                for(int i = 0; i < 7; ++ i)
                    if(arr_number_users[i])
                        iterator = i;
                lista_useri_vida = 0;
            }
        } while(!no_users);
        // while care sa parcurga lista de useri din nivelul curent
        // alt while care sa parcurga lista proceselor user-ului curent
        if(arr_number_users[iterator] == 0) {
            iterator = (iterator + 1) % 7;
            continue;   
        }

        struct User* user = niveluri[iterator]->head;
        do
        {
            if(!user->nr_proc) {
                user = user->next;
                continue;
            }

            struct Process* pr = user->process_list->head;
            while(pr != NULL)
            {
                printf("Waiting...\n");
                double timp_minim = min_double(pr->burst_time, arr_ponderi[iterator] * TIME_SLICE);
                usleep(timp_minim * 1000000);       // usleep asteapta numar intreg de microsecunde, pe cand sleep astepta numar intreg de secunde
                printf("S-a executat procesul %d al utilizatorului %s timp de %.2fs\n", pr->pid, user->username, timp_minim);
                pr->burst_time -= timp_minim;
                if(pr->burst_time <= 0) {
                    printf("Procesul %d al utilizatorului %s si-a terminat executia\n", pr->pid, user->username);
                    struct Process* temp = pr->next;
                    pop_process_user(user, pr);
                    pr = temp;
                    if(user->nr_proc == 0) {
                        -- no_users;
                        -- arr_number_users[iterator];
                        break;
                    }
                    continue;
                }
                pr = pr->next;
            }
            user = user->next;
        } while (user != niveluri[iterator]->head);
        
        iterator = (iterator + 1) % 7;
        if(!no_users)
            lista_useri_vida = 1;
    }
    printf("\n\n=======ALGORITMUL S-A TERMINAT=======\n");
}

int main() {
    srand(time(NULL));

    citire_fisier();

    for(int i = 0; i < 7; ++ i)
        niveluri[i] = create_user_list();

    for(int i = 0; i < input_list_size; ++ i){
        printf("%d %s %d %d\n", lista_input[i].timp, lista_input[i].nume, lista_input[i].pid, lista_input[i].pondere);
    }

    hcreate(1024);
    
    actualizeaza_lista(time(NULL));

    round_robin();

    hdestroy();

    return 0;
}
