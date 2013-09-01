#ifndef _process_h_
#define _process_h_
#include <paging.h>
#include <list.h>

typedef uint32_t tid_t;

typedef enum
{

    PROCESS_ACTIVE = 1,

    PROCESS_FREEZED =  2,

    PROCESS_ZOMBIE = 4,

    PROCESS_NOT_FOUND = -1,

}pflag_t;

typedef uint32_t pid_t;

typedef enum
{

    KERNELMODE = 0,

    USERMODE = 3

} privilege_t;

struct process_state
{
    pid_t pid;
    pd_t *pagedir;
    char *name;
    char *desc;
    struct process_state* parent;
    list_t *ports;
    list_t *threads;
    tid_t tid_counter;
    list_t *zombie_tids;
    list_t *children;
    uint16_t flags;
};

struct child
{
    int return_value;
    pid_t pid;
    struct process_state *process;
};


    struct process_state *process_create(const char *name, const char *desc, uint16_t flags,struct process_state *parent);

    void process_kill(struct process_state *process);

    struct process_state *process_find(pid_t id);

#endif