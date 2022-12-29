#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>

int v[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

struct info {
    int* vptr;
}info_instance[100], *info_ptr; //info_instance - loc unde sa pun informatiile (values) din hash

// ne trebuie pointeri atat pentru keys cat si pentru values, pentru ca asa sunt implementate functiile alea (hcreate, hsearch, hdestroy)

int main() {
    // IDEEA: putem sa folosim hash tables si sa tinem acolo pointeri catre nodurile din lista de useri ca sa putem insera usor procesele la fiecare user.
    // putem sa inseram si repede in hash perechi (user_name, user_ptr).

    hcreate(1024);	// creeaza hash-ul care poate sa tina maxim 1024 de perechi key-value
    ENTRY item;
    ENTRY *found_item;

    char strs[1024 * 20], *ptr_str = strs; // strs - loc pentru keys (care pot sa fie doar strings); trebuie sa fie in zone diferite de memorie
    					// ptr_str - pointer ca sa pot sa mut cum vreau locul unde scriu keys in buffer
    info_ptr = info_instance;
    FILE *proc = popen("/bin/ps aux | awk '{print $1}' | sort | uniq","r"); // asta imi da userii (unici)
    char buf[2048];
    int counter = 0;
    while ( !feof(proc) && fgets(buf,sizeof(buf),proc) )
    {
        if(!strcmp(buf, "USER\n"))	// in proc am si "USER\n"; comanda aia de useri unici imi da si numele coloanei 
            continue;
            
        strncpy(ptr_str, buf, strlen(buf) - 1); // toate cuvintele care ajung in buf au '\n' la final; aici scot \n
        info_ptr->vptr = v + (counter ++);
        item.key = ptr_str;	// item = pereche key-value
        item.data = info_ptr;	
        ptr_str += strlen(ptr_str) + 1;
        info_ptr ++;
        hsearch(item, ENTER);	// insert
        //============================================================ NOTHING IMPORTANT ATM
        // printf("%s, %d\n", item.key, ((struct info*)item.data)->id);
        // char* p = strtok(buf, " ");
        // int counter = 0;
        // while(counter < 10) {
        //     ++ counter;
        //     if(counter == 1 || counter == 2 || counter == 9)
        //         printf("%s ", p);
        //     p = strtok(NULL, " ");
        // }
        // printf("\n");
        //============================================================
    }

	// test sa vad daca "root" este in hash
    char name_to_find[10] = "root";

    item.key = name_to_find;
    found_item = hsearch(item, FIND);	// returneaza pointer catre item-ul gasit; daca nu il gaseste returneaza NULL
    if(found_item) 
        printf("%d. %s\n", *((struct info*)found_item->data)->vptr, name_to_find);
    else
        printf("No such entry as '%s'\n", name_to_find);
}

