#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RES_TYPES 3
#define MAX_PROCESSES 128

typedef unsigned res_t;
typedef res_t res_table_t[RES_TYPES];

typedef struct {
    size_t pid;
    res_table_t max, alloc;
} process_t;

typedef struct {
    size_t pid;
    res_table_t current_need;
} need_t;

typedef struct {
    process_t processes[MAX_PROCESSES];
    size_t process_count;
    res_table_t available;
} system_t;

typedef int(*algorithm_t)(system_t*, const need_t*);

int apply_need(system_t* sys, const need_t* need) {
    size_t i, j;

    for (i = 0; i < sys->process_count; ++i) {
        process_t* proc = &sys->processes[i];
        if (need->pid == proc->pid) {
            int succeed = 1;
            for (j = 0; j < RES_TYPES; ++j) {
                succeed = succeed &&
                    need->current_need[j] <= proc->max[j] - proc->alloc[j];
                succeed = succeed &&
                    need->current_need[j] <= sys->available[j];
            }

            if (succeed) {
                for (j = 0; j < RES_TYPES; ++j) {
                    proc->alloc[j] += need->current_need[j];
                    sys->available[j] -= need->current_need[j];
                }
                return 1;
            }
        }
    }

    return 0;
}

void kill_finished_processes(system_t* sys) {
    size_t i, j, k = 0;
    for (i = 0; i < sys->process_count; ++i) {
        int is_finished = 1;

        sys->processes[k] = sys->processes[i];

        for (j = 0; j < RES_TYPES; ++j)
            if (sys->processes[i].max[j] - sys->processes[i].alloc[j])
                is_finished = 0;

        if (!is_finished) {
            k++;
        }
        else {
            for (j = 0; j < RES_TYPES; ++j)
                sys->available[j] += sys->processes[i].alloc[j];
        }
    }
    sys->process_count = k;
}

int banker_verify(system_t sys) {
    if (sys.process_count == 0) return 1;
    else {
        size_t i, j;
        for (i = 0; i < sys.process_count; ++i) {
            need_t need;
            need.pid = sys.processes[i].pid;
            for (j = 0; j < RES_TYPES; ++j) 
                need.current_need[j] = sys.processes->max[j] - sys.processes->alloc[j];
            
            if (apply_need(&sys, &need)) {
                kill_finished_processes(&sys);
                if (banker_verify(sys) == 1)
                    return 1;
            }
        }

        return 0;
    }
}

int banker_algorithm(system_t* sys, const need_t* need) {
    system_t sys_for_verify = *sys;
    if (apply_need(&sys_for_verify, need)) {
        kill_finished_processes(&sys_for_verify);
        if (banker_verify(sys_for_verify)) {
            *sys = sys_for_verify;
            return 1;
        }
        else {
            printf("Can not find a safe path.");
            return 0;
        }
    }
    else {
        printf("Can not allocate the resource.");
        return 0;
    }
}

int random_algorithm(system_t* sys, const need_t* need) {
    system_t sys_for_verify = *sys;
    if (apply_need(&sys_for_verify, need)) {
        if (rand() % 2) {
            printf("Refused randomly.\n");
            return 0;
        }
        else {
            *sys = sys_for_verify;
            return 1;
        }
    }
    else {
        return 0;
    }
}

void print_system(const system_t* sys) {
    size_t i;
    
    printf("Avaliable:");
    for(i = 0; i < RES_TYPES; ++i)
        printf(" %d", sys->available[i]);
    printf("\n");
    
    printf("PID\tMax\tAlloc\tNeed\n");
    for (i = 0; i < sys->process_count; ++i) {
        size_t j;
        const process_t* proc = &sys->processes[i];
        printf("%d\t", proc->pid);
        
        for (j = 0; j < RES_TYPES; ++j)
            printf("%d ", proc->max[j]);
        printf("\t");

        for (j = 0; j < RES_TYPES; ++j)
            printf("%d ", proc->alloc[j]);
        printf("\t");

        for (j = 0; j < RES_TYPES; ++j)
            printf("%d ", proc->max[j] - proc->alloc[j]);
        printf("\n");
    }
}

int main() {
    system_t current_system = {
        {
            { 0, { 5, 5, 9 }, { 2, 1, 2 } },
            { 1, { 5, 3, 6 }, { 4, 0, 2 } },
            { 2, { 4, 0, 11 }, { 4, 0, 5 } },
            { 3, { 4, 2, 5 }, { 2, 0, 4 } },
            { 4, { 4, 2, 4 }, { 3, 1, 4 } }
        },
        5,
        { 17, 5, 20 }
    };

    algorithm_t algorithm = NULL;

    srand((unsigned)time(NULL));

    while (algorithm == NULL) {
        int selection = 0;
        printf("\nSelect the algorithm:\n");
        printf("1. Banker Algorithm.\n");
        printf("2. Random Algorithm.\n");
        printf("Input the number: ");
        selection = getchar();
        switch (selection) {
        case '1':
            algorithm = &banker_algorithm;
            printf("\nUsing banker algorithm.\n");
            break;
        case '2':
            algorithm = &random_algorithm;
            printf("\nUsing random algorithm.\n");
            break;
        default:
            printf("\nSelection must be 1 or 2.\n");
            break;
        };
    }

    printf("\n");

    while (current_system.process_count > 0) {
        need_t need;
        size_t i;
        print_system(&current_system);
        printf("Which process will allocate the resource? Input the PID: ");
        scanf("%u", &need.pid);
        for (i = 0; i < RES_TYPES; ++i) {
            printf("Input the allocate count of the resource %u: ", i);
            scanf("%u", &need.current_need[i]);
        }

        switch (algorithm(&current_system, &need)) {
        case 1:
            kill_finished_processes(&current_system);
            printf("Succeed!");
            break;
        case 0:
            printf("Failed!");
            break;
        default:
            printf("Error!");
            break;
        }

        printf("\n\n");
    }

    printf("All process finished.\n");

    return 0;
}