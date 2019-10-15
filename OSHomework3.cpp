#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>
#include <optional>
#include <random>

using namespace std;
using semaphore = std::atomic_int;	// 使用原子量模拟信号量行为

constexpr int BufferSize = 3;		// 缓冲区最大大小

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
	mutex m;									// 定义锁，用于保护缓冲区和标准输出流
	semaphore full = 0;							// 定义full信号量
	semaphore empty = BufferSize - 1;			// 定义empty信号量
	array<optional<int>, BufferSize> buffer;	// 定义缓冲区

	auto producerFunction = [&]					// 生产者函数
	{
		random_device r;						// 随机数发生器
		while (true)
		{
			this_thread::sleep_for(666ms);		// 模拟需要666毫秒生产一个产品
			const int product = r() % 10000;	// 生产一个产品，为0~10000的一个函数

			P(empty);							// 对full进行P操作
			{
				lock_guard g(m);				// 使此复合语句为锁m的临界区
				cout							// 打印日志
					<< "Producing:" 
					<< product 
					<< endl;
				for (auto& s : buffer)			// 寻找缓冲区中一个空的槽位
				{
					if (!s.has_value())			// 如果发现一个空的槽位
					{
						s = product;			// 写入产品到缓冲区
						break;
					}
				}
				V(full);						// 对empty进行V操作
			}
		}
	};

	auto consumerFunction = [&]				// 消费者函数
	{
		while (true)
		{
			this_thread::sleep_for(1s);		// 模拟需要1秒消耗一个商品

			P(full);						// 对full执行P操作
			{
				lock_guard g(m);			// 使此复合语句为锁m的临界区
				for (auto& s : buffer)		// 从缓冲区查找一个有商品的槽位
				{
					if (s.has_value())		// 如果有商品
					{
						cout				// 打印日志
							<< "Consuming:" 
							<< *s 
							<< endl;
						s = nullopt;		// 消耗此商品
						break;
					}
				}
				V(empty);					// 对empty进行V操作
			}
		}
	};

	thread producers[3] = {					//创建3个生产者线程
		thread{producerFunction},
		thread{producerFunction},
		thread{producerFunction}
	};

	thread consumer{ consumerFunction };	// 创建两个消费者线程

	for (auto& s : producers) s.join();		// 等待生产者线程结束
	consumer.join();						// 等待消费者线程结束

	return 0;
}

/*
流程图源码

# 生产者
```flow
st=>start: 开始
create=>operation: 创建商品
wait=>operation: P(empty)
lock=>operation: 加锁
take=>operation: 写入商品
release=>operation: V(full)
unlock=>operation: 解锁
st->create->wait->lock->take->release->unlock->create
```


# 消费者
```flow
st=>start: 开始
wait=>operation: P(full)
lock=>operation: 加锁
take=>operation: 取出商品
release=>operation: V(empty)
unlock=>operation: 解锁
use=>operation: 消费商品
st->wait->lock->take->release->unlock->use->wait
```



*/
