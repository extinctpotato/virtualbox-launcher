#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

#define VBOXPATH "/usr/bin/VirtualBox"

void signal_handler()
{
	exit(0);
}

pid_t proc_find(const char* name) 
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if(!(dir = opendir("/proc"))) 
    {
        return -1;
    }

    while((ent = readdir(dir)) != NULL) 
	{
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') 
        {
            continue;
		}

		/* try to open the cmdline file */
		snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
		FILE* fp = fopen(buf, "r");

		if (fp) 
		{
			if (fgets(buf, sizeof(buf), fp) != NULL) 
			{
				/* check the first token in the file, the program name */
				char* first = strtok(buf, " ");
				if (!strcmp(first, name)) 
				{
					fclose(fp);
					closedir(dir);
					return (pid_t)lpid;
				}
			}
			fclose(fp);
		}
	}
	
	closedir(dir);
	return -1;
}

int main()
{
	signal(SIGCHLD, signal_handler);
	pid_t vbox_pid = fork();
	
	if(vbox_pid < 0)
	{
		return 1;
	}
	else if(vbox_pid == 0)
	{
		execvp(VBOXPATH, NULL);

	}
	while(1)
	{
		if(proc_find("/usr/lib/virtualbox/VirtualBoxVM") != -1)
		{
			sleep(3);
			kill(vbox_pid, SIGTERM);
			return 0;
		}
		sleep(1);
	}
}
