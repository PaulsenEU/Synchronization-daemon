#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct list list;

struct list {
    char *path;
    char *name;
    int file_size;
    char *type;
    list *next;
};


// utworzenie listy
list *create_list();

int get_list_size(list *directory_list);

// dodanie kolejnego elementu
void add_node(list **file_list, char *path, char *name, int file_size, char* type);

void printFilesList(list *file_list);

// usunięcie listy
void remove_list(list **list);


#endif
