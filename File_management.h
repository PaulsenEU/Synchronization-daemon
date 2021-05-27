#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "list.h"
#include <stdbool.h>

char *get_file_type(const char* path);

time_t get_file_modification_date(char *path);

void clone_timestamp(char *src_file, char *dest_file);

bool compare_files_modification_date( char *source_file,  char *dest_file);

char *get_full_path(const char *path, char *file_name);

list *read_dir(const char *path, bool R);

void create_file(char *path);

void copy_file(char *src_path, char *dest_path);

void copy_file_mmap(const char *src_path, const char *dest_path);

void remove_folder_files(list *folder, char *folder_path, bool R);

void remove_files(list *source_list, list *dest_list, bool R);


#endif
