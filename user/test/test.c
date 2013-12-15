#include <universe.h>
#include <stdio.h>
#include <math.h>

int thread(int argc, void **argv)
{
    printf("you can't stop me!\n");
    return 0;
}

int main(int argc, char **argv) {
    printf("Userspace!\n");
    //thread_launch(thread, 0, NULL);

    int pid = linux_syscall(SYS_FORK, 0, 0, 0, 0, 0);
    if(pid == 0) {
        printf("Child!\n");
        exit(0);
    } else {
        printf("Parent!\n");
        linux_syscall(SYS_WAITPID,-1,0,0,0,0);
        printf("I'm back!\n");
    }

    char *a = malloc(10);
    a = "1234567890";
    printf(a);

    while(1);
    return 0;
}

