#include <iostream>
#include <string>
#include <cstdint>
#include <forward_list>
#include <algorithm>

using namespace std;

enum class ProcessState : uint8_t
{
    Running,
    Ready,
    Finished
};

struct PCB final
{
    string name;
    uint8_t priority;
    ProcessState state = ProcessState::Ready;
    uint64_t cputime = 0;
    uint64_t needtime;
    uint64_t count = 0;
    
    PCB(string&& name,uint8_t priority,uint64_t needtime):
        name { std::move(name) },
        priority { priority },
        needtime { needtime }
    {}
};

ostream& operator << (ostream& o,const PCB& pcb)
{
    o   << "Process:"<< pcb.name << '\t'
        << "Priority:" << static_cast<int>(pcb.priority) << '\t'
        << "State:" << static_cast<int>(pcb.state) << '\t'
        << "CPUTime:" << pcb.cputime << '\t'
        << "NeedTime:" << pcb.needtime << '\t'
        << "Count: " << pcb.count;
    return o;
}

using PCBList = forward_list<PCB>;


PCBList processes;

void Run(PCB& process,uint64_t time)
{
    if (process.state == ProcessState::Finished) return;
    
    // Startup
    process.state = ProcessState::Running;
    const auto runningTime = clamp(time,0ul,process.needtime);
    process.needtime -= runningTime;
    process.cputime += runningTime;
    process.count ++;
    
    // Shutdown
    process.state = process.needtime <= 0 ? ProcessState::Finished : ProcessState::Ready;
    
    cout<<"Running:"<<process<<" for "<<runningTime<<"ms.";
    if(process.state == ProcessState::Finished) cout << "Finished." << endl;
    else cout << endl;
}

bool AllProcessFinished(PCBList& processes)
{
    return all_of(processes.begin(),processes.end(),[](const PCB& p){
        return p.state == ProcessState::Finished;
    });
}

using SchedulingMethod = function<void(PCBList&)>;

template <uint64_t TimeSlice>
SchedulingMethod RoundRun = [](PCBList& ls){
    while(!AllProcessFinished(ls))
        for(auto& pcb:ls)
            Run(pcb,TimeSlice);
};

template <uint8_t PrioritySub,uint64_t TimeSlice>
SchedulingMethod PriorityMethod = [](PCBList& ls){
    while(!AllProcessFinished(ls))
    {
        PCB* toRun = nullptr;
        for(auto& p : ls)
        {
            if(p.state != ProcessState::Finished)
            {
                if(toRun)
                {
                    if(toRun->priority < p.priority)
                        toRun = &p;
                }
                else
                    toRun = &p;
            }
        }
        Run(*toRun,TimeSlice);
        toRun->priority -= std::clamp(PrioritySub,static_cast<uint8_t>(0),toRun->priority);
    }
};

const auto StaticPriority = PriorityMethod<0u,100u>;
const auto DynamicPriority = PriorityMethod<3u,100u>;

int main(void)
{
    PCBList ls;
    
    for(int i = 0;i < 10;++i)
        ls.emplace_front("process"+std::to_string(i),rand() % 8,rand() % 500 + 500);
    
    cout << "Processes:" << endl;
    for(auto& p:ls)
        cout << p << endl;
    
    auto ls2 = ls;
    auto ls3 = ls;
    
    cout << "=== Round Run ===" << endl;
    RoundRun<100>(ls);
    
    cout << endl << "=== Static Priority === " << endl;
    StaticPriority(ls2);
    
    cout << endl << "=== Dynamic Priority === " << endl;
    DynamicPriority(ls3);
    
    return 0;
}
