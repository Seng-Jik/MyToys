#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>
#include <optional>
#include <random>

using namespace std;
using semaphore = std::atomic_int;	// 使用原子量模拟信号量行为

constexpr int Max = 80;				// 最大有80个座位

void P(semaphore& s)				// 定义P操作
{
	while (s <= 0)					// 信号量小于等于0时
		this_thread::yield();		// 时间片提前结束
	s--;							// 当信号量大于0时，将信号量减去1
}

void V(semaphore& s)				// 定义V操作
{
	s++;							// 将信号量加上1
}

int main()
{
	semaphore door = 0, stop = 0;		// 门和开车状态信号量

	atomic_int people = 0;				// 车上的人数

	thread conductor{ [&]				// 售票员线程
	{
		while (true)
		{
			cout 
				<< "输入人数变动情况:" 
				<< endl;
			int n;
			cin >> n;									// 输入上车和下车人数
			people = clamp(people + n, 0, Max);			// 计算车上人数并处理超载和下溢的问题
			cout << "此时车上人数:" << people << endl;

			cout << "关门" << endl;
			V(door);									// 关门
			cout << "售票" << endl;
			P(stop);									// 当停车时开门
			cout << "开门" << endl;
		}
	} };

	thread driver{ [&]				// 司机线程
	{
		while (true)
		{
			P(door);				// 当关门时开车
			cout << "开车" << endl;
			V(stop);				// 停车
			cout << "停车" << endl;
		}
	} };

	driver.join();		// 等待司机线程结束
	conductor.join();	// 等待售票员线程结束

	return 0;
}

/* 流程图源码

# 售票员
```flow
st=>start: 当前处于开门状态
a=>operation: P(stop)
b=>operation: 当前处于关门状态
c=>operation: V(door)
d=>operation: 当前处于开门状态
st->a->b->c->d->a
```

# 司机线程
```flow
st=>start: 当前处于停车状态
b=>operation: P(door)
d=>operation: 当前处于开车状态
c=>operation: V(stop)
e=>operation: 当前处于停车状态
st->b->d->c->e->b
```

*/
