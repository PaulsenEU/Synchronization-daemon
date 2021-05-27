#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <time.h>

#include "File_management.h"


void Compare_directories(char *source_path, char *dest_path, bool R, int file_size)
{
    list *source_list = read_dir(source_path, R);
    list *dest_list = read_dir(dest_path, R);   

    //usuwanie zbednych plików

    syslog(LOG_INFO, "Usuwanie zbednych plikow\n");

    remove_files(source_list, dest_list, R);

    //int source_list_size = get_list_size(source_list);
    int dest_list_size = get_list_size(dest_list);
    //printf("Rozmiar docelowy: %d\n", dest_list_size);
    list *tmp = dest_list;
    char *temp_type = "regular file";

    //jezeli oba katalogi sa puste, nie sprawdzamy nic, czekamy na kolejne uruchomienie demona
    {
        if(source_list == NULL && dest_list == NULL)
        {
            printf("Zaktualizuj katalogi\n");
            syslog(LOG_INFO, "Katalogi są puste, zaktualizuj\n");
            return;
        }
    }

    //jezeli katalog zrodlowy zawiera pliki a docelowy jest pusty------------------------------------------------

    if(source_list != NULL && dest_list == NULL)
    {
        printf("Katalog docelowy jest pusty, nastepuje kopiowanie zawartosci z katalogu zrodlowego\n");

        syslog(LOG_INFO, "Katalog docelowy jest pusty, nastepuje kopiowanie zawartosci z katalogu zrodlowego\n");

        while(source_list != NULL)
        {
            if(source_list->file_size < file_size)
            {

                if(strcmp(source_list->type, "regular file") == 0)
                {
                    syslog(LOG_INFO, "zwykle kopiowanie pliku ");

                    printf("plik %s skopiowany - sciezka docelowa: %s\n", source_list->path, get_full_path(dest_path, source_list->name));

                    copy_file(source_list->path, get_full_path(dest_path, source_list->name));

                    clone_timestamp(source_list->path, get_full_path(dest_path, source_list->name));
                }

                if(strcmp(source_list->type, "directory") == 0)
                {
                    syslog(LOG_INFO, "zwykle kopiowanie folderu ");
                    printf("folder %s skopiowany\n", source_list->name);
                    mkdir(get_full_path(dest_path,source_list->name),0777);
                    Compare_directories(source_list->path, get_full_path(dest_path,source_list->name), R, file_size);
                    //clone_timestamp(source_list->path, dest_list->path);
                }
            }
                else
                {
                    if(strcmp(source_list->type, "regular file") == 0)
                    {
                        syslog(LOG_INFO, "kopiowanie pliku z mmap");

                        printf("plik %s skopiowany - sciezka docelowa: %s\n", source_list->path, get_full_path(dest_path, source_list->name));

                        copy_file_mmap(source_list->path, get_full_path(dest_path, source_list->name));

                        clone_timestamp(source_list->path, get_full_path(dest_path, source_list->name));
                    }

                    if(strcmp(source_list->type, "directory") == 0)
                    {
                        syslog(LOG_INFO, "kopiowanie folderu ");
                        printf("folder %s skopiowany\n", source_list->name);
                        mkdir(get_full_path(dest_path,source_list->name),0777);
                        Compare_directories(source_list->path, get_full_path(dest_path,source_list->name), R, file_size);
                        //clone_timestamp(source_list->path, dest_list->path);
                    }
                }
            source_list = source_list->next;
        }
    }


    //jezeli katalog zrodlowy i docelowy zawieraja pliki -------------------------------------------------------------


    if(source_list != NULL && dest_list != NULL)
    {
        printf("Katalogi nie sa puste, nastepuje porownanie plikow\n");

        //przechodzimy po kolei po kazdym pliku z katalogu zrodlowego-------------------------------------------------

        while(source_list != NULL)  
        {
            printf("Plik źródłowy: %s , Typ: %s - ", source_list->name, source_list->type);

            int i = 0;

            char *s_name = source_list->name;
            char *s_type = source_list->type;
            char *s_path = source_list->path;
            int   s_size = source_list->file_size;

            //kazdy plik zrodlowy porównujemy z kazdym plikiem znajdujacym sie w katalogu docelowym-------------------
            //Przechodzimy po kazdym pliku z katalogu docelowego------------------------------------------------------

            while(dest_list != NULL)
            {
                //printf("Plik docelowy: %s , Typ: %s\n", dest_list->name, dest_list->type);

                char *d_name = dest_list->name;
                char *d_type = dest_list->type;
                char *d_path = dest_list->path;
                int   d_size = dest_list->file_size;

                //jezeli porownywane pliki maja takie same typy i nazwy ----------------------------------------------
                //to znaczy ze plik sie znajduje w katalogu docelowym-------------------------------------------------
                //w przeciwnym wypadku nastepuje zliczanie zmiennej "i". ---------------------------------------------

                if( strcmp(s_name, d_name ) == 0 && strcmp(s_type, d_type) == 0 )
                {
                    printf("znajduje sie w katalogu docelowym.  ");

                    //Sprawdzamy wnetrze katalogow, czy w katalogu zrodlowym pojawil sie jakis nowy plik--------------
                    if(strcmp(s_type, "directory") == 0 )
                    {
                        Compare_directories(source_list->path, dest_list->path, R, file_size);
                    }

                    //jezeli plik z katalogu zrodlowego ma nowsza date modyfikacji niz plik z katalogu docelowego-----
                    //nastepuje kopiowanie pliku do katalogu docelowego za pomoca kopiowania:-------------------------

                    if(compare_files_modification_date(s_path, d_path) == true)
                    {

                        syslog(LOG_INFO,"Plik %s ma nowsza date modyfikacji - robimy kopiowanie ", s_name);

                        //jezeli rozmiar pliku zrodlowego jest mniejszy niz ta podana w parametrze--------------------
                        //uzywamy kopiowania zwyklego-----------------------------------------------------------------

                        if(s_size < file_size)
                        {
                            printf("zwykle\n");

                            if( strcmp(s_type, "regular file") == 0 )
                            {
                                syslog(LOG_INFO, "usuwamy plik : %s",dest_list->path);

                                remove(dest_list->path);

                                syslog(LOG_INFO, "Kopiujemy plik %s do %s\n", source_list->path, dest_list->path);

                                copy_file(source_list->path, dest_list->path);

                                clone_timestamp(source_list->path, dest_list->path);
                            }

                            if( strcmp(s_type, "directory") == 0 )
                            {
                                Compare_directories(source_list->path, dest_list->path, R, file_size);

                                clone_timestamp(source_list->path, dest_list->path);
                            }
                        }
                        else 
                        {
                            //jezeli rozmiar pliku zrodlowego jest mniejszy niz ta podana w parametrze----------------
                            //uzywamy kopiowania zwyklego-------------------------------------------------------------

                            syslog(LOG_INFO, "Kopiowanie mmap");

                            if( strcmp(s_type, "regular file") == 0 )
                            {
                                remove(dest_list->path);

                                copy_file_mmap(source_list->path, dest_list->path);

                                clone_timestamp(source_list->path, dest_list->path);
                            }

                            if( strcmp(s_type, "directory") == 0 )
                            {
                                Compare_directories(source_list->path, dest_list->path, R, file_size);

                                clone_timestamp(source_list->path, dest_list->path);
                            }
                        }
                        
                    }else printf("\n");
                    
                }
                else i++;
                
                //jezeli wartosc zmiennej "i" bedzie równa rozmiarowi listy katalogu docelowego,to znaczy że----------
                //przeszedl po wszystkich plikach w katalogu docelowym i nie znalazl w nim pliku z katalogu źródłowego

                if(i == dest_list_size)
                {
                    syslog(LOG_INFO, "plik %s nie znajduje sie w katalogu docelowym- nastepuje kopiowanie z zrodlowego\n", source_list->path);
                    printf("nie znajduje sie w katalogu docelowym - kopiujemy go do docelowego\n\n");

                    char *temp_path;

                    if(s_size < file_size)     
                    {
                        

                        if( strcmp(s_type, "regular file") == 0 )
                        {
                            temp_path = get_full_path(dest_path, s_name);

                            printf("Source path: %s\n", s_path);

                            printf("File Temp path: %s\n", temp_path);

                            copy_file(s_path, temp_path);
                        }

                        if( strcmp(s_type, "directory") == 0 )
                        {
                            char *dir_temp = get_full_path(dest_path, s_name);

                            //printf("Sciezka Source: %s,  Sciezka katalogu dest: %s\n", s_path, dir_temp);

                            mkdir(dir_temp, 0777);

                            Compare_directories(s_path, dir_temp, R, file_size);
                        }
                        
                    }   
                    else 
                    {
                        if( strcmp(s_type, "regular file") == 0 )
                        {

                            temp_path = get_full_path(dest_path, s_name);
                            printf("Source path: %s\n", s_path);
                            printf("File Temp path: %s\n", temp_path);
                            copy_file_mmap(s_path, temp_path);
                        }
                        
                        if( strcmp(s_type, "directory") == 0 )
                        {
                            char *dir_temp = get_full_path(dest_path, s_name);

                            //printf("Sciezka Source: %s,  Sciezka katalogu dest: %s\n", s_path, dir_temp);

                            mkdir(dir_temp, 0777);

                            Compare_directories(s_path, dir_temp, R, file_size);
                        }
                    }
                
                }

                dest_list = dest_list->next;

            }

            source_list = source_list->next;

            dest_list = tmp;
            
        }
    }


    
}