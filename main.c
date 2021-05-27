#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <syslog.h> //syslogi
#include <signal.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "File_management.h"
#include "Fork.h"
#include "Directories_Comparison.h"
#include "list.h"

bool Synchronization = false;
bool kill_signal = false;
static char **args;

void handler(int signum)
{
    if(signum == SIGUSR1)
    {
        if(Synchronization == true)
        {
            syslog(LOG_INFO, "Wymuszenie wybudzenia w trakcie synchronizacji, synchronizuje dalej...");
        }
        else {
            syslog(LOG_INFO, "Wybudzam demona, wymuszenie synchronizacji");
            execve("/proc/self/exe", args, NULL);
            exit(EXIT_SUCCESS);
        } 
    }

    if(signum == SIGTERM)
    {
        syslog(LOG_INFO, "Zakończenie pracy demona");
        exit(EXIT_SUCCESS);
    }

    exit(signum);
}

int main(int argc, char *argv[])
{
    args = argv;
    signal(SIGUSR1, handler);
    signal(SIGTERM, handler);

    
/*
    if (argc<3)
    {
        fprintf(stderr,"Podaj conajmniej 2 argumenty");
    }
    
        
*/
    char *source_directory = argv[1];
    char *destination_directory = argv[2];

    int time_variable = 300; //ustawiamy domyslnie czas spania na 300 s. = 5 min 
    bool recursive; 
    int default_size = 1000000000;


    if(strcmp(get_file_type(source_directory), "directory") == 0 && strcmp(get_file_type(destination_directory), "directory") == 0) //sprawdzamy czy obie sciezki są katalogami
    {
        printf("Podane argumenty są katalogami\n");

        if(argc >= 4 && argc <= 6) //sprawdzamy czy opcjonalnie jest argument ustawiajacy czas spania
        {

            if(argv[3] == "-R") 
            {
                recursive = true;

                if(strcmp(argv[4], "default") == 1)
                    time_variable = atoi(argv[4]); // czas
                
                if(strcmp(argv[5], "default") == 1)
                {
                    default_size = atoi(argv[5]); // rozmiar
                }
            }
            else 
            {
                recursive = false;

                if(strcmp(argv[3], "default") == 1)
                    time_variable = atoi(argv[3]);
                
                if(strcmp(argv[4], "default") == 1)
                    
                    default_size = atoi(argv[4]);
                    
            }
        }
    }
    Fork_Function(); //uruchamia proces

    while(1)
    {
        syslog(LOG_INFO, "Synchronizacja rozpoczęta");
        Synchronization = true;
        Compare_directories(source_directory, destination_directory, recursive, default_size);
        Synchronization = false;
        sleep(time_variable); //zasypia na okreslony czas 
    }

    closelog();
    
}    

