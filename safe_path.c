#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RES_TYPES 3                        /* ��Դ�������� */
#define MAX_PROCESSES 128                  /* ϵͳ�д��ڵ����������� */

typedef unsigned res_t;                    /* ��Դ������ */
typedef res_t res_table_t[RES_TYPES];      /* ��Դ�� */

typedef struct {
    size_t pid;			                /* ����ID */
    res_table_t max, alloc;                /* ���������ѷ���� */
} process_t;                               /* ���� */

typedef struct {
    size_t pid;                            /* ������Щ��Դ�Ľ���ID */
    res_table_t current_need;              /* ����Դ������� */
} need_t;                                  /* ��Դ���� */

typedef struct {
    process_t processes[MAX_PROCESSES];    /* ϵͳ�д��ڵĽ��� */
    size_t process_count;                  /* ϵͳ�д��ڵĽ������� */
    res_table_t available;                 /* ϵͳ�п��õ���Դ���� */
} system_t;                                /* ϵͳ */

/* ����PID��ϵͳ�в��ҽ���
 *
 * ���ʧ���򷵻�NULL��
 */
process_t* find_process_by_pid(system_t* sys, size_t pid) {
    size_t i;
    for (i = 0; i < sys->process_count; ++i)
        if (sys->processes[i].pid == pid)
            return &sys->processes[i];
    return NULL;
}

/* ������Ӧ�õ�ϵͳ��
 *
 * ������Խ�������Ӧ�õ�ϵͳ�ϣ��򷵻�1�����޸Ĵ����system_t��
 * �����ǰϵͳ����������󣬲���Ӧ�õ�ϵͳ�ϣ��򷵻�0��
 */
int apply_need(system_t* sys, const need_t* need) {
    size_t j;

    /* ������Ҫ������Դ�Ľ��� */
    process_t* proc = find_process_by_pid(sys, need->pid);
    if (proc == NULL) return 0;

    /* ���������͵���Դ���м�飬�������������succeed����Ϊ0 */
    int succeed = 1;
    for (j = 0; j < RES_TYPES; ++j) {

        /* Ҫ���������Դһ��С�ڡ�������Դ������ */
        /* ������Դ���� = �����Դ���� - �ѷ�����Դ���� */
        succeed = succeed &&
            need->current_need[j] <= proc->max[j] - proc->alloc[j];

        /* Ҫ���������Դһ��С��ϵͳ������Դ���� */
        succeed = succeed &&
            need->current_need[j] <= sys->available[j];
    }

    /* �������ɹ� */
    if (succeed) {
        /* �򽫷����Ľ��д�뵽ԭ����ϵͳ�� */
        for (j = 0; j < RES_TYPES; ++j) {

            /* ��������Դ�Ľ��̵���Դ��������Ҫ�������Դ���� */
            proc->alloc[j] += need->current_need[j];

            /* ϵͳ������Դ���ȥҪ�������Դ���� */
            sys->available[j] -= need->current_need[j];
        }
        return 1;
    }
        

    /* �������ʧ���򷵻�0 */
    return 0;
}

/* ɱ���Ѿ����н����Ľ��̲�������Դ
 *
 * �Դ����ϵͳ����Ƿ�����Ѿ������Ľ��̣�
 * ɱ���Ѿ������Ľ��̣������ϵͳ��ɾ������������Դ����ӽ���ϵͳ������Դ����
 */
void kill_finished_processes(system_t* sys) {
    size_t i, j, k = 0;                            /* i��j���ڱ�����kΪ��ǰ��д���α� */
    for (i = 0; i < sys->process_count; ++i) {     /* �������н��� */
        int is_finished = 1;                       /* �����ǰ�����Ѿ���ɣ���Ϊ1�����������δ�����Ϊ0�����赱ǰ�����Ѿ����� */

        sys->processes[k] = sys->processes[i];     /* ����i������д�뵽��k���ϣ����ڽ��Ѿ�ɾ���Ľ��̵Ŀն����н��� */

        for (j = 0; j < RES_TYPES; ++j)            /* �����������͵���Դ */
            if (sys->processes[i].max[j] - sys->processes[i].alloc[j])    /* ��������κ�һ����Դ�䡰������Դ��������Ϊ0 */
                is_finished = 0;                   /* �������δ���� */

        if (!is_finished) {                        /* ���������δ��������д���α������ƶ�����ֹ�˽��̱�ɾ�� */
            k++;
        }
        else {
            for (j = 0; j < RES_TYPES; ++j)        /* ����˽����Ѿ��������򽫴˽���ռ�õ���Դ���յ�ϵͳ������Դ��*/
                                                   /* �����α�û���ƶ������Դ˽��̽��ᱻ��һ�����̸��� */
                sys->available[j] += sys->processes[i].alloc[j];
        }
    }
    sys->process_count = k;                        /* ���㵱ǰϵͳ��ʣ��Ľ������� */
}

/* ��ӡ����·�� */
void print_path(size_t* path, size_t path_size) {
    size_t i;
    for (i = 0; i < path_size; ++i)
        printf("%d ", path[i]);
    printf("\n");
}

/* ��������ӡȫ����ȫ·�� */
int print_safe_path(system_t sys, size_t* path, size_t path_size) {
    if (sys.process_count == 0) {       /* �����ǰϵͳ�Ѿ������ڽ��̣���ϵͳ��ȫ����ӡ·��������1 */
        print_path(path, path_size);
        return 1;
    }
    else {                              /* �����ǰϵͳ���ڽ��� */
        int safe = 0;                   /* ���赱ǰ������Σ��״̬ */
        size_t i, j;
        for (i = 0; i < sys.process_count; ++i) {   /* �������н��� */
            system_t s = sys;                       /* ����ϵͳ����ϵͳ���ڽ�һ��������ȫ·�� */
            need_t need;                            /* ������ǰ���̵�ȫ����Դ���� */
            need.pid = s.processes[i].pid;
            for (j = 0; j < RES_TYPES; ++j)
                need.current_need[j] = s.processes[i].max[j] - s.processes[i].alloc[j];

            path[path_size] = need.pid;             /* ���˽��̵�PIDд�뵽��ǰ·���� */

            if (apply_need(&s, &need)) {            /* �������㵱ǰ���̵�ȫ������ */
                kill_finished_processes(&s);        /* ɾ���Ѿ������Ľ��̲�������Դ */
                safe = print_safe_path(s, path, path_size + 1); /* ��ʣ�µĽ��̽��н�һ����֤ */
            }
        }

        return safe;
    }
}

/* ��֤����·���Ƿ�ȫ */
int verify_path(system_t sys, size_t* path, size_t path_size) {
    size_t i, j;
    for (i = 0; i < path_size; ++i) {       /* ������ǰ·�� */
        process_t* proc = find_process_by_pid(&sys, path[i]);   /* Ѱ��·���е�ǰPID�Ľ��� */

        need_t need;                                            /* ������Դ��������˽��̵�ȫ������ */
        need.pid = proc->pid;
        for (j = 0; j < RES_TYPES; ++j) 
            need.current_need[j] = proc->max[j] - proc->alloc[j];
        
        if (apply_need(&sys, &need) == 0)                       /* ִ�д�����������ֲ�����������֤ʧ�ܣ�·������ȫ */
            return 0;

        kill_finished_processes(&sys);                          /* �����Ѿ������Ľ��̵���Դ */
    }

    return 1;                                                   /* ���·���Ѿ�ȫ�����꣬��·����ȫ */
}

/* ��ӡϵͳ״̬ */
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

/* �������·�� 
 * 
 * �ó���ʵ���Ͼ��ǲ��ϴ�0~n����Ȼ�����������ȡ�����ݷ���·����
 */
void set_random_path(size_t* path, size_t path_size) {
    size_t i;
    size_t pids[MAX_PROCESSES] = { 0 };     /* ��ȡ������ */
    size_t pid_count = path_size;           /* ��ȡ�������� */

    for (i = 0; i < path_size; ++i)         /* ��pids�ﴴ��ֵ0, 1, 2, ..., n */
        pids[i] = i;

    for (i = 0; i < path_size; ++i) {   /* ȡpath_size - 1������ */
        size_t pid_index = rand() % pid_count;  /* ���Ҫȡ����ֵ������ */
        path[i] = pids[pid_index];              /* ȡ����������������·�� */
        pids[pid_index] = pids[--pid_count];    /* ����ȡ�������е����һ�����ַŵ���ȡ�����ֵ�λ�ã���������ն��� */
                                                /* ֮�󽫴�ȡ�����ϵĴ�С��ȥ1 */
    }
}

int main() {
    /* ����һ��ϵͳ */
    system_t current_system = {
        /* ����������� */
        {
            { 0, { 7, 5, 3 }, { 0, 1, 0 } },
            { 1, { 3, 2, 2 }, { 2, 0, 0 } },
            { 2, { 9, 0, 2 }, { 3, 0, 2 } },
            { 3, { 2, 2, 2 }, { 2, 1, 1 } },
            { 4, { 4, 3, 3 }, { 0, 0, 3 } }
        },
        5,

        /* ��ǰϵͳ������Դ�� */
        { 10, 3, 0 }
    };

    size_t path_buffer[MAX_PROCESSES];

    /* ��ʼ������� */
    srand((unsigned)time(NULL));

    /* ��ӡ��ϵͳ */
    print_system(&current_system);

    /* ���м��㷨��ӡ��ȫ·�� */
    printf("\nSafe path of current system:\n");
    if (0 == print_safe_path(current_system, path_buffer, 0))
        printf("There has no safe path to print.\n");

    /* ��ӡ���·�� */
    set_random_path(path_buffer, current_system.process_count);
    printf("\nRandom path: (%s)\n", 
        verify_path(current_system, path_buffer, current_system.process_count) ? "safe" : "unsafe");
    print_path(path_buffer, current_system.process_count);

    return 0;
}
