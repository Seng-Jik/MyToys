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
        << "Count: " << pcb.count << endl;
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

int main(void)
{
    PCB pcb
    {
        "super",0,1000
    };
    
    Run(pcb,1500);
    
    PCBList ls;
    ls.emplace_front(std::move(pcb));
    
    cout << AllProcessFinished(ls);
    return 0;
}
