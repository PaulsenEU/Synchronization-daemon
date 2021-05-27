#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>
#include "File_management.h"
#include <utime.h>

//#include "list.h"


char* get_file_type (const char* path)
{
    struct stat st;
    lstat (path, &st);
    if (S_ISLNK (st.st_mode))
      return "symbolic link";
    else if (S_ISDIR (st.st_mode))
      return "directory";
    else if (S_ISCHR (st.st_mode))
      return "character device";
    else if (S_ISBLK (st.st_mode))
      return "block device";
    else if (S_ISFIFO (st.st_mode))
      return "fifo";
    else if (S_ISSOCK (st.st_mode))
      return "socket";
    else if (S_ISREG (st.st_mode))
      return "regular file";
    else
      assert (0);
} 

time_t get_file_modification_date(char *path)
{
  
  struct stat st;
  stat(path, &st);
  printf("%s: Ostatnia data modyfikacji: %s", path, ctime(&st.st_mtime));
  return st.st_mtime;
}

void clone_timestamp(char *src_file, char *dest_file){
    struct stat st;
    struct utimbuf new_stamp;
    stat(src_file, &st);
    new_stamp.actime = st.st_atim.tv_sec;
    new_stamp.modtime = st.st_mtim.tv_sec;
    utime(dest_file, &new_stamp);
    chmod(dest_file, st.st_mode);
}


bool compare_files_modification_date(char *source_file,  char *dest_file)
{
  
  struct stat st1, st2;

  if(stat(source_file, &st1) != 0 || stat(dest_file, &st2) != 0)
  {
    printf("file extension");
    return NULL;
  }

  if(difftime(st1.st_mtime, st2.st_mtime) > 0)
  {
    return true;
  }
  else 
    return false;
    
}

char *get_full_path (const char *path, char *file_name)
{
  char *full_path = (char*) malloc((strlen(path) + strlen(file_name) + 2));
  strcpy(full_path, path);
  strcat(full_path, "/");
  strcat(full_path, file_name);
  return full_path;
}

list *read_dir(const char *path, bool R) {
    DIR *dir;
    list *file_list = NULL;
    struct dirent *rdir;
    dir = opendir(path);

    while((rdir = readdir(dir)) != NULL) {
        if(strcmp(rdir -> d_name, ".") == 0 || strcmp(rdir -> d_name, "..") == 0) continue;
        
        struct stat st;
        char *full_path = get_full_path(path,rdir->d_name);
        char *type = get_file_type(full_path);

        stat(full_path, &st);

        if(type == "regular file") // sprawdzamy czy plik jest zwyklym plikiem 
        {
          add_node(&file_list, full_path, rdir -> d_name, st.st_size, type);
        }

        if(type == "directory" && R == true)
        {
          add_node(&file_list, full_path, rdir -> d_name, st.st_size, type);
        }

    }

    return file_list;
}

void create_file(char *path) {

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    int fd = open(path, O_WRONLY | O_EXCL | O_CREAT, mode); // tworzenie pliku
    if(fd == -1) {
      syslog(LOG_CRIT, "Nie można utworzyć pliku");
      exit(EXIT_FAILURE);
    }

    close(fd);
}

void copy_file(char *src_path, char *dest_path) {

    int fdsrc, fddest;
    char ch;
    fdsrc = open(src_path, O_RDONLY);

    if(fdsrc == -1) {
      close(fdsrc);
      return;
    }
    fddest = open(dest_path,
            O_WRONLY | O_CREAT , 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    while(read(fdsrc, &ch, 1)) {
      write(fddest, &ch, 1);
    }

    close(fdsrc);
    close(fddest);
}

/*void copy_file(char *src_path, char *dest_path) {
  char buf;
	int fd_one, fd_two;

	fd_one = open(src_path, O_RDONLY);

	if (fd_one == -1)
	{
		close(fd_one);
		return;
	}

	fd_two = open(dest_path, 
				  O_WRONLY | O_CREAT,
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	
	while(read(fd_one, &buf, 1))
	{
		write(fd_two, &buf, 1);
	}

	
	close(fd_one);
	close(fd_two);
}*/


void copy_file_mmap(const char *src_path, const char *dest_path) {

  int fdsrc, fddest;
  char *src, *dest;
  struct stat st;
  size_t file_size;


  // otwarcie pliku zrodlowego
  fdsrc = open(src_path, O_RDONLY);
  file_size = lseek(fdsrc, 0, SEEK_END);
  src = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fdsrc, 0);


  // otwarcie pliku docelowego
  fddest = open(dest_path, O_RDWR | O_CREAT, 0666);
  ftruncate(fddest, file_size);
  dest = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fddest, 0);


  // kopiowanie
  memcpy(dest, src, file_size);
  munmap(src, file_size);
  munmap(dest, file_size);

  close(fdsrc);
  close(fddest);

}


void remove_folder_files(list *folder, char *folder_path, bool R)
{
    while(folder != NULL)
    {
        printf("Path usuwanego pliku: %s\n", folder->path);

        if(strcmp(folder->type, "regular file") == 0)
        {
            remove(folder->path);
        }

        if(strcmp(folder->type, "directory") == 0)
        {
            list *temp = read_dir(folder->path,R);
            
                remove_folder_files(temp, folder->path, R);
        }

        folder = folder->next;
    } 
    
        rmdir(folder_path);
}


void remove_files(list *source_list, list *dest_list, bool R)
{
    list *tmp = source_list;
    list *tmp1 = dest_list;
    


    if(dest_list != NULL && source_list == NULL)
    {
        printf("katalog docelowy zawiera pliki a katalog zrodlowy jest pusty\n");

        while(dest_list != NULL)
        {
            
            if(strcmp(dest_list->type, "regular file") == 0)
            {
                remove(dest_list->path);
            }

            if(strcmp(dest_list->type, "directory") == 0)
            {
                list *temp = read_dir(dest_list->path, R);
                remove_folder_files(temp, dest_list->path, R);
            }
            dest_list = dest_list->next;

        }
    }
    
    //jezeli katalog docelowy i zrodlowy zawieraja pliki
    if(dest_list != NULL && source_list != NULL)
    {
        printf("Katalogi maja zawartosc\n");

        while(dest_list != NULL)
        {
            int i=0;
            printf("Plik %s ", dest_list->name);

            while(source_list != NULL)
            {
                if(strcmp(dest_list->name, source_list->name) == 0 && strcmp(dest_list->type,source_list->type) == 0)
                {
                    printf("znajduje sie w katalogu zrodlowym\n");

                    if(strcmp(dest_list->type, "directory") == 0)
                    {
                        printf("S: %s , D: %s\n", source_list->path, dest_list->path);

                        list *s_temp = read_dir(source_list->path, R);

                        list *d_temp = read_dir(dest_list->path, R);
                        
                        printFilesList(s_temp);

                        printFilesList(d_temp);

                        remove_files(s_temp, d_temp, R);
                    }
                }
                else 
                {
                    i++;
                }

                if(i == get_list_size(tmp))
                {
                    printf("plik %s nie znajduje sie w katalogu zrodlowym\n", dest_list->name);

                    if(strcmp(dest_list->type, "directory") == 0)
                    {
                        list *d_temp = read_dir(dest_list->path, R);

                        remove_folder_files(d_temp, dest_list->path, R);
                    }

                    if(strcmp(dest_list->type, "regular file") == 0)
                    {
                        remove(dest_list->path);
                    }
                }

                source_list = source_list->next;
                
            }
            source_list = tmp;
            dest_list = dest_list -> next;
        }
    }
    
}

