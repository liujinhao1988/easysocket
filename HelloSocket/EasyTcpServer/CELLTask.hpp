//该类是用于
//把要执行的函数打入CELLTaskServer中存储
//在慢慢执行
#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>

//#include"CELLSemaphore.hpp"
//#include"CELLLog.hpp"
#include"CELLThread.hpp"
//任务类型-基类
/*
class CellTask
{
public:
CellTask()
{

}

//虚析构
virtual ~CellTask()
{

}
//执行任务
virtual void doTask()
{

}
private:

};
*/




//执行任务的服务类型
class CELLTaskServer
{
public:
	//所属server的ID
	int serverId = -1;
private:
	typedef std::function<void()> CellTask;//function 绑定普通函数作为函数的指针，这样函数也可以成为参数 （）中的为函数返回类型
private:
	//任务数据
	std::list<CellTask> _tasks;
	//任务数据缓冲区
	std::list<CellTask> _tasksBuf;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//
	CELLThread _thread;
public:
	//添加任务
	void addTask(CellTask task)
	{
		std::lock_guard<std::mutex> lock(_mutex);//多个线程共用数据 加锁
		_tasksBuf.push_back(task);
	}
	//启动工作线程
	void Start()
	{
		/*
		_isRun = true;
		//线程
		std::thread t(std::mem_fn(&CELLTaskServer::OnRun),this);
		t.detach();
		*/
		_thread.Start(nullptr, [this](CELLThread* pThread) {
			OnRun(pThread);
		});//线程启动 onrun 函数
	}

	void Close()
	{
		//CELLLog::Info("CELLTaskServer%d .Close begin\n", serverId);
		/*
		if (_isRun)
		{

		_isRun = false;
		_sem.wait();

		}
		*/
		_thread.Close();//线程关闭

		//CELLLog::Info("CELLTaskServer%d .Close end\n", serverId);
	}
protected:
	//工作函数
	void OnRun(CELLThread* pThread)//
	{
		while (pThread->isRun())
		{
			//从缓冲区取出数据
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//如果没有任务
			if (_tasks.empty())
			{
				//暂停 防止浪费资源
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for (auto pTask : _tasks)
			{
				pTask();

			}
			//清空任务
			_tasks.clear();
		}


		//处理buff缓冲队列任务
		for (auto pTask : _tasksBuf)
		{
			pTask();

		}
		//CELLLog::Info("CELLTaskServer%d .OnRun exit\n", serverId);
		//_sem.wakeup();
	}
};
#endif // !_CELL_TASK_H_
