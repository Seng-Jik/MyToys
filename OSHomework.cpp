#include <iostream>     // IO流头文件
#include <string>       // 字符串头文件
#include <cstdint>      // 长度明确int头文件
#include <forward_list> // 单向链表头文件
#include <algorithm>    // 算法库头文件

using namespace std;    // 打开标准库

enum class ProcessState : uint8_t   // 进程状态
{
    Running,    // 运行态
    Ready,      // 就绪态
    Finished    // 完成态
};

struct PCB final    // 对PCB的定义
{
    string name;                                // 进程名
    uint8_t priority;                           // 优先数
    ProcessState state = ProcessState::Ready;   // 进程状态，初始为就绪
    uint64_t cputime = 0;                       // 已经占用的CPU时间
    uint64_t needtime;                          // 还需时间
    uint64_t count = 0;                         // 被调上CPU的次数
    
    PCB(string&& name,uint8_t priority,uint64_t needtime):  // PCB的构造函数
        name { std::move(name) },
        priority { priority },
        needtime { needtime }
    {}
};

ostream& operator << (ostream& o,const PCB& pcb)    // 用于将PCB打印到流的函数
{
    o   << "Process:"<< pcb.name << '\t'
        << "Priority:" << static_cast<int>(pcb.priority) << '\t'
        << "State:" << static_cast<int>(pcb.state) << '\t'
        << "CPUTime:" << pcb.cputime << '\t'
        << "NeedTime:" << pcb.needtime << '\t'
        << "Count: " << pcb.count;
    return o;
}

using PCBList = forward_list<PCB>;                              // 定义PCB列表为PCB的单向链表

void Run(PCB& process,uint64_t time)                            // 用于将进程调上CPU开始执行
{
    if (process.state == ProcessState::Finished) return;        // 排除掉完成态进程
    
    // Startup
    process.state = ProcessState::Running;                      // 设置为运行态
    const auto runningTime = clamp(time,0ul,process.needtime);  // 计算真实的时间片长度
    process.needtime -= runningTime;                            // 还需时间减去时间片长度
    process.cputime += runningTime;                             // 已占用时间加上时间长度
    process.count ++;                                           // 调上CPU次数加一
    
    // Shutdown
    process.state =                                             // 计算下个状态，它可能是就绪态，也可能是完成态
        process.needtime <= 0 ? 
            ProcessState::Finished : 
            ProcessState::Ready;
    
    cout<<"Running:"<<process<<" for "<<runningTime<<"ms.";     // 输出当前进程的运行状态
    if(process.state == ProcessState::Finished)                 // 如果当前状态为完成态
        cout << "Finished." << endl;                            // 则输出一个Finished后缀
    else cout << endl;                                          // 否则不输出后缀
}

bool AllProcessFinished(PCBList& processes)                     // 检查一个PCB列表内是否所有PCB均已是完成态
{
    return all_of(processes.begin(),processes.end(),            // 使用all_of组合子在全列表根据谓词进行判定
        [](const PCB& p){                                       // 谓词字面量
            return p.state == ProcessState::Finished;           // 当PCB状态为Finished的时候，谓词为true
        }
    );
}

using SchedulingMethod = function<void(PCBList&)>;              // 定义所有调度算法的类型

template <uint64_t TimeSlice>                                   // 模板参数为时间片长度
SchedulingMethod RoundRun = [](PCBList& ls){                    // 时间片轮转算法
    while(!AllProcessFinished(ls))                              // 当存在未完成的进程时
        for(auto& pcb:ls)                                       // 对所有PCB遍历
            Run(pcb,TimeSlice);                                 // 根据固定时间片运行
};

template <uint8_t PrioritySub,uint64_t TimeSlice>               // 模板参数为优先权减少数和时间片
SchedulingMethod PriorityMethod = [](PCBList& ls){              // 优先权算法
    while(!AllProcessFinished(ls))                              // 当存在未完成的进程时
    {
        PCB* toRun = nullptr;                                   // 定义“即将执行的进程”，目前未知
        for(auto& p : ls)                                       // 对每一个PCB进行遍历
        {
            if(p.state != ProcessState::Finished)               // 排除掉所有已完成的进程
            {
                if(toRun)                                       // 如果存在“即将执行的进程”
                {
                    if(toRun->priority < p.priority)            // 如果当前进程的优先数大于“即将执行的进程”
                        toRun = &p;                             // “即将执行的进程”被更新为正在遍历的进程
                }
                else
                    toRun = &p;                                 // 否则将当前进程设为“即将执行的进程”
            }                                                   // 此后“即将执行的进程”为优先数最大的进程
        }
        Run(*toRun,TimeSlice);                                  // 运行此进程
        toRun->priority -=                                      // 减去优先数，截断到0防止减法溢出
            std::clamp(PrioritySub,static_cast<uint8_t>(0),toRun->priority);
    }
};

const auto StaticPriority = PriorityMethod<0u,100u>;            // 静态优先级调度算法
const auto DynamicPriority = PriorityMethod<3u,100u>;           // 动态优先级调度算法

int main(void)
{
    PCBList ls;                                                 // 创建一个PCB列表
    
    for(int i = 0;i < 10;++i)                                   // 在PCB列表里创建10个PCB
        ls.emplace_front(                                       // 在列表内就地创建PCB
            "process"+std::to_string(i),                        // 给进程一个名字
            rand() % 256,                                       // 随机设定优先数（0~255）内
            rand() % 500 + 500);                                // 在500到999的范围内随机设定一个需要时间
    
    cout << "Processes:" << endl;                               // 打印每一个PCB的信息
    for(auto& p:ls)
        cout << p << endl;
    
    auto ls2 = ls;                                              // 从PCB列表ls复制两个副本
    auto ls3 = ls;
    
    cout << "=== Round Run ===" << endl;                        // 执行时间片轮转算法
    RoundRun<100>(ls);
    
    cout << endl << "=== Static Priority === " << endl;         // 执行静态优先级算法
    StaticPriority(ls2);
    
    cout << endl << "=== Dynamic Priority === " << endl;        // 执行动态优先级算法
    DynamicPriority(ls3);
    
    return 0;
}
