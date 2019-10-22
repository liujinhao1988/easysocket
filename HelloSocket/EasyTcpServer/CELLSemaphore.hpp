//信号量
#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include<chrono>//时间相关头文件
#include<thread>//线程

//条件变量
#include<condition_variable>
class CELLSemaphore
{
public:
	/*CELLSemaphore()
	{

	}
	~CELLSemaphore()
	{

	}*/
	//阻塞当前线程
	void wait()//线程阻塞通常是指一个线程在执行过程中暂停，以等待某个条件的触发。
	{
		//std::lock_guard<std::mutex> lock(_mutex);//
		std::unique_lock<std::mutex> lock(_mutex);//比lock_guard灵活，可按条件控制锁还是不锁
		if (--_wait < 0)
		{
			//阻塞等待
			_cv.wait(lock, [this]()->bool {return _wakeup > 0; });//wait 当第二个参数false是 才会阻塞当前线程 true 几所阻塞
			--_wakeup;
		}
		///_isWaitExit = true;
		

		/*
		while (_isWaitExit)
		{//信号量
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
		*/
	}
	//唤醒当前线程
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait<=0)
		{
			++_wakeup;
			///_isWaitExit = false;
			_cv.notify_one();//notify_one   唤醒某个等待(wait)线程。如果当前没有等待线程，则该函数什么也不做，
		}
			
		
	}
private:
	
	//阻塞等待-条件变量
	std::condition_variable _cv;
	//
	std::mutex _mutex;
	//等待计数
	int _wait = 0;

	//唤醒计数
	int _wakeup = 0;

	//bool _isWaitExit = false;
};



#endif // !_CELL_SEMAPHORE_HPP_
//虚假唤醒