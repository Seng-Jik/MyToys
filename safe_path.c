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

/* 根据PID在系统中查找进程
 *
 * 如果失败则返回NULL。
 */
process_t* find_process_by_pid(system_t* sys, size_t pid) {
    size_t i;
    for (i = 0; i < sys->process_count; ++i)
        if (sys->processes[i].pid == pid)
            return &sys->processes[i];
    return NULL;
}

/* 将需求应用到系统上
 *
 * 如果可以将此需求应用到系统上，则返回1，并修改传入的system_t。
 * 如果当前系统不满足此需求，不能应用到系统上，则返回0。
 */
int apply_need(system_t* sys, const need_t* need) {
    size_t j;

    /* 查找需要分配资源的进程 */
    process_t* proc = find_process_by_pid(sys, need->pid);
    if (proc == NULL) return 0;

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

/* 打印给定路径 */
void print_path(size_t* path, size_t path_size) {
    size_t i;
    for (i = 0; i < path_size; ++i)
        printf("%d ", path[i]);
    printf("\n");
}

/* 搜索并打印全部安全路径 */
int print_safe_path(system_t sys, size_t* path, size_t path_size) {
    if (sys.process_count == 0) {       /* 如果当前系统已经不存在进程，则系统安全，打印路径并返回1 */
        print_path(path, path_size);
        return 1;
    }
    else {                              /* 如果当前系统存在进程 */
        int safe = 0;                   /* 假设当前进程在危险状态 */
        size_t i, j;
        for (i = 0; i < sys.process_count; ++i) {   /* 遍历所有进程 */
            system_t s = sys;                       /* 复制系统，该系统用于进一步搜索安全路径 */
            need_t need;                            /* 创建当前进程的全部资源需求 */
            need.pid = s.processes[i].pid;
            for (j = 0; j < RES_TYPES; ++j)
                need.current_need[j] = s.processes[i].max[j] - s.processes[i].alloc[j];

            path[path_size] = need.pid;             /* 将此进程的PID写入到当前路径中 */

            if (apply_need(&s, &need)) {            /* 尝试满足当前进程的全部需求 */
                kill_finished_processes(&s);        /* 删除已经结束的进程并回收资源 */
                safe = print_safe_path(s, path, path_size + 1); /* 将剩下的进程进行进一步验证 */
            }
        }

        return safe;
    }
}

/* 验证给定路径是否安全 */
int verify_path(system_t sys, size_t* path, size_t path_size) {
    size_t i, j;
    for (i = 0; i < path_size; ++i) {       /* 遍历当前路径 */
        process_t* proc = find_process_by_pid(&sys, path[i]);   /* 寻找路径中当前PID的进程 */

        need_t need;                                            /* 创建资源需求，满足此进程的全部需求 */
        need.pid = proc->pid;
        for (j = 0; j < RES_TYPES; ++j) 
            need.current_need[j] = proc->max[j] - proc->alloc[j];
        
        if (apply_need(&sys, &need) == 0)                       /* 执行此需求，如果发现不能满足则验证失败，路径不安全 */
            return 0;

        kill_finished_processes(&sys);                          /* 回收已经结束的进程的资源 */
    }

    return 1;                                                   /* 如果路径已经全部走完，则路径安全 */
}

/* 打印系统状态 */
void print_system(const system_t* sys) {
    size_t i;

    printf("Avaliable:");
    for (i = 0; i < RES_TYPES; ++i)
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

/* 创建随机路径 
 * 
 * 该程序实际上就是不断从0~n的自然数集合中随机取出数据放入路径。
 */
void set_random_path(size_t* path, size_t path_size) {
    size_t i;
    size_t pids[MAX_PROCESSES] = { 0 };     /* 待取数集合 */
    size_t pid_count = path_size;           /* 待取数的数量 */

    for (i = 0; i < path_size; ++i)         /* 在pids里创建值0, 1, 2, ..., n */
        pids[i] = i;

    for (i = 0; i < path_size; ++i) {   /* 取path_size - 1次数据 */
        size_t pid_index = rand() % pid_count;  /* 获得要取的数值的坐标 */
        path[i] = pids[pid_index];              /* 取这个数，并将其放入路径 */
        pids[pid_index] = pids[--pid_count];    /* 将待取数集合中的最后一个数字放到被取走数字的位置，用于填补“空洞” */
                                                /* 之后将待取数集合的大小减去1 */
    }
}

int main() {
    /* 创建一个系统 */
    system_t current_system = {
        /* 包含五个进程 */
        {
            { 0, { 7, 5, 3 }, { 0, 1, 0 } },
            { 1, { 3, 2, 2 }, { 2, 0, 0 } },
            { 2, { 9, 0, 2 }, { 3, 0, 2 } },
            { 3, { 2, 2, 2 }, { 2, 1, 1 } },
            { 4, { 4, 3, 3 }, { 0, 0, 3 } }
        },
        5,

        /* 当前系统可用资源数 */
        { 10, 3, 0 }
    };

    size_t path_buffer[MAX_PROCESSES];

    /* 初始化随机数 */
    srand((unsigned)time(NULL));

    /* 打印此系统 */
    print_system(&current_system);

    /* 银行家算法打印安全路径 */
    printf("\nSafe path of current system:\n");
    if (0 == print_safe_path(current_system, path_buffer, 0))
        printf("There has no safe path to print.\n");

    /* 打印随机路径 */
    set_random_path(path_buffer, current_system.process_count);
    printf("\nRandom path: (%s)\n", 
        verify_path(current_system, path_buffer, current_system.process_count) ? "safe" : "unsafe");
    print_path(path_buffer, current_system.process_count);

    return 0;
}
