#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>

void Fork_Function()
{

    syslog(LOG_INFO, "Forkowanie procesu");
    pid_t pid, sid;
    pid = fork();

    if(pid < 0) {
        exit(EXIT_FAILURE);
    }
    if(pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    umask(0);
    

    sid = setsid();
    
    if (sid < 0) {
            exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) {
            exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
