#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RES_TYPES 3                        /* 资源类型数量 */
#define MAX_PROCESSES 128                  /* 系统中存在的最大进程数量 */

typedef unsigned res_t;                    /* 资源计数器 */
typedef res_t res_table_t[RES_TYPES];      /* 资源表 */

typedef struct {
    size_t pid;			                /* 进程ID */
    res_table_t max, alloc;                /* 最大需求表、已分配表 */
} process_t;                               /* 进程 */

typedef struct {
    size_t pid;                            /* 需求这些资源的进程ID */
    res_table_t current_need;              /* 对资源的需求表 */
} need_t;                                  /* 资源需求 */

typedef struct {
    process_t processes[MAX_PROCESSES];    /* 系统中存在的进程 */
    size_t process_count;                  /* 系统中存在的进程数量 */
    res_table_t available;                 /* 系统中可用的资源数量 */
} system_t;                                /* 系统 */

typedef int(*algorithm_t)(system_t*, const need_t*);    /* 指向资源分配算法的指针类型 */

/* 将需求应用到系统上
 * 
 * 如果可以将此需求应用到系统上，则返回1，并修改传入的system_t。
 * 如果当前系统不满足此需求，不能应用到系统上，则返回0。
 */
int apply_need(system_t* sys, const need_t* need) {
    size_t i, j;

    /* 遍历所有进程，寻找要分配资源的进程 */
    for (i = 0; i < sys->process_count; ++i) {
        process_t* proc = &sys->processes[i];
        
        /* 找到要分配资源的进程后 */
        if (need->pid == proc->pid) {
        
            /* 对所有类型的资源进行检查，如果不满足检查则将succeed设置为0 */
            int succeed = 1;
            for (j = 0; j < RES_TYPES; ++j) {
            
                /* 要求需求的资源一定小于“还需资源”数量 */
                /* 还需资源数量 = 最大资源数量 - 已分配资源数量 */
                succeed = succeed &&
                    need->current_need[j] <= proc->max[j] - proc->alloc[j];
                    
                /* 要求需求的资源一定小于系统可用资源数量 */
                succeed = succeed &&
                    need->current_need[j] <= sys->available[j];
            }

            /* 如果分配成功 */
            if (succeed) {
                /* 则将分配后的结果写入到原来的系统中 */
                for (j = 0; j < RES_TYPES; ++j) {
                
                    /* 分配了资源的进程的资源分配表加上要分配的资源数量 */
                    proc->alloc[j] += need->current_need[j];
                    
                    /* 系统可用资源表减去要分配的资源数量 */
                    sys->available[j] -= need->current_need[j];
                }
                return 1;
            }
        }
    }

    /* 如果分配失败则返回0 */
    return 0;
}

/* 杀死已经运行结束的进程并回收资源 
 *
 * 对传入的系统检查是否存在已经结束的进程，
 * 杀死已经结束的进程，将其从系统中删除，并回收资源，添加进“系统可用资源表”。
 */
void kill_finished_processes(system_t* sys) {
    size_t i, j, k = 0;                            /* i和j用于遍历，k为当前的写入游标 */
    for (i = 0; i < sys->process_count; ++i) {     /* 遍历所有进程 */
        int is_finished = 1;                       /* 如果当前进程已经完成，则为1，如果进程尚未完成则为0，假设当前进程已经结束 */

        sys->processes[k] = sys->processes[i];     /* 将第i个进程写入到第k个上，用于将已经删除的进程的空洞进行紧缩 */

        for (j = 0; j < RES_TYPES; ++j)            /* 遍历所有类型的资源 */
            if (sys->processes[i].max[j] - sys->processes[i].alloc[j])    /* 如果存在任何一种资源其“还需资源数量”不为0 */
                is_finished = 0;                   /* 则进程尚未结束 */

        if (!is_finished) {                        /* 如果进程尚未结束，则将写入游标向下移动，防止此进程被删除 */
            k++;
        }
        else {
            for (j = 0; j < RES_TYPES; ++j)        /* 如果此进程已经结束，则将此进程占用的资源回收到系统可用资源表*/
                                                   /* 由于游标没有移动，所以此进程将会被下一个进程覆盖 */
                sys->available[j] += sys->processes[i].alloc[j];
        }
    }
    sys->process_count = k;                        /* 计算当前系统中剩余的进程数量 */
}

int banker_verify(system_t sys) {                  /* 使用银行家算法验证当前的系统是否安全 */
    if (sys.process_count == 0) return 1;          /* 如果当前系统中已经不存在进程，则系统安全 */
    else {                                         /* 如果当前系统中存在进程，则搜索安全路径 */
        size_t i, j;
        for (i = 0; i < sys.process_count; ++i) {  /* 遍历所有进程 */
            need_t need;                           /* 对每个可能的进程产生一个需求，这个需求将会分配此进程所有剩下的“还需资源” */
            need.pid = sys.processes[i].pid;
            for (j = 0; j < RES_TYPES; ++j) 
                need.current_need[j] = sys.processes->max[j] - sys.processes->alloc[j];
            
            if (apply_need(&sys, &need)) {        /* 应用这个需求，如果成功 */
                kill_finished_processes(&sys);    /* 则回收应用需求后的资源 */
                if (banker_verify(sys) == 1)      /* 对剩下的部分继续做银行家算法的验证，如果它找到了安全路径，则返回1 */
                    return 1;
            }
        }

        return 0;                                 /* 未能找到安全路径，返回0 */
    }
}

/* 使用银行家算法分配资源
 *
 * 如果分配成功则返回1，否则返回0.
 */
int banker_algorithm(system_t* sys, const need_t* need) {
    system_t sys_for_verify = *sys;                    /* 创建系统的副本，用于进行验证 */
    if (apply_need(&sys_for_verify, need)) {           /* 在用于验证的副本上应用资源 */
        kill_finished_processes(&sys_for_verify);      /* 回收已经结束的进程占用的资源 */
        if (banker_verify(sys_for_verify)) {           /* 使用银行家算法进行验证，如果找到了安全路径，则 */
            *sys = sys_for_verify;                     /* 将用于验证的副本写入回系统，返回1 */
            return 1;
        }
        else {                                         /* 未能找到安全路径 */
            printf("Can not find a safe path.");
            return 0;
        }
    }
    else {                                             /* 系统资源不足，不能分配资源 */
        printf("Can not allocate the resource.");
        return 0;
    }
}

/* 使用随机算法分配资源
 * 
 * 如果分配成功则返回1，否则返回0.
 */
int random_algorithm(system_t* sys, const need_t* need) {
    system_t sys_for_verify = *sys;                    /* 创建一个系统副本 */
    if (apply_need(&sys_for_verify, need)) {           /* 在副本上应用需求 */
        if (rand() % 2) {                              /* 随机，如果为奇数则拒绝分配 */
            printf("Refused randomly.\n");
            return 0;
        }
        else {                                         /* 如果是偶数则进行分配 */
            *sys = sys_for_verify;
            return 1;
        }
    }
    else {                                             /* 如果资源不足，则不能分配 */
        return 0;
    }
}

/* 打印系统状态
 *
 */
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
        printf("%lu\t", proc->pid);
        
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
    /* 创建一个系统 */
    system_t current_system = {
        /* 包含五个进程 */
        {
            { 0, { 5, 5, 9 }, { 2, 1, 2 } },
            { 1, { 5, 3, 6 }, { 4, 0, 2 } },
            { 2, { 4, 0, 11 }, { 4, 0, 5 } },
            { 3, { 4, 2, 5 }, { 2, 0, 4 } },
            { 4, { 4, 2, 4 }, { 3, 1, 4 } }
        },
        5,
        
        /* 当前系统可用资源数 */
        { 17, 5, 20 }
    };

    /* 选定的分配算法 */
    algorithm_t algorithm = NULL;

    /* 初始化随机数 */
    srand((unsigned)time(NULL));

    /* 要求用户选择一个分配算法 */
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
    
    /* 当系统中存在进程时，要求用户提出资源需求 */
    while (current_system.process_count > 0) {
        need_t need;
        size_t i;
        print_system(&current_system);
        printf("Which process will allocate the resource? Input the PID: ");
        scanf("%lu", &need.pid);
        for (i = 0; i < RES_TYPES; ++i) {
            printf("Input the allocate count of the resource %lu: ", i);
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

    /* 所有进程结束，系统结束 */
    printf("All process finished.\n");

    return 0;
}

