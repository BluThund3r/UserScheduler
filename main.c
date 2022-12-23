#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define TIME_SLICE 10

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
    char* username;
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

void add_process(struct User user, pid_t pid, double burst_time) {
    struct Process* temp = alloc_process();
    temp->pid = pid;
    temp->burst_time = burst_time;
    temp->next = NULL;
    temp->prev = NULL;
    if(!user.nr_proc) {
        user.process_list = create_process_list();
        user.process_list->head = temp;
        user.process_list->tail = temp;
    }

    else {
        temp->prev = user.process_list->tail;
        user.process_list->tail->next = temp;
        user.process_list->tail = temp;
    }

    ++ user.nr_proc;
}

void pop_process_user(struct User user) {
    struct Process* temp = user.process_list->head;
    user.process_list->head = user.process_list->head->next;
    free(temp);
    -- user.nr_proc;
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

void add_user_to_userlist(struct UserList* user_list, int user_id, char* username_, double pondere_) {
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


int main() {

    return 0;
}
