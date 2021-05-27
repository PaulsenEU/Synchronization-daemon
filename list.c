#include "list.h"
#include <string.h>
#include <stdio.h>

list *create_list() {
    list *file_list = malloc(sizeof(list));
    file_list -> next = NULL;
    return file_list;
}

void add_node(list **file_list, char *path, char *name, int file_size, char* type) {

    list *tmp = (list*) malloc(sizeof(list));
    list *last = *file_list;

    tmp -> path = path;
    tmp -> name = name;
    tmp -> file_size = file_size;
    tmp -> type = type;
    tmp -> next = NULL;

    if(*file_list == NULL)
    {
        *file_list = tmp;
        return;
    }

    while(last->next !=NULL)
    {
        last = last->next;
    }
    last->next = tmp;
    return;

}

void printFilesList(list *file_list)
{
  while (file_list != NULL)
  {
     printf("Nazwa pliku: %s, Typ: %s, Path: %s\n", file_list->name, file_list->type, file_list->path);
     file_list = file_list->next;
  }
}


int get_list_size(list *directory_list)
{
    int i=0;
    list *tmp;
    tmp = directory_list;
    while(tmp != NULL)
    {
        i++;
        tmp = tmp->next;
    }
    return i;
}
void remove_list(list **first) {
    list *tmp, *curr;
    curr = (*first);
    while(curr != NULL) {
        tmp = curr -> next;
        free(curr);
        curr = tmp;
    }

    *first = NULL;
}
