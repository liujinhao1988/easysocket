#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CELLSemaphore.hpp"
class CELLThread
{
private:
	typedef std::function<void(CELLThread*)> EventCall;//function 绑定普通函数作为函数的指针，这样函数也可以成为参数 （）中的为函数返回类型
public:
	//启动 参数 函数的指针
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;
			//线程
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);//mem_fn 将成员函数转换为函数对象 返回一个函数对象，该函数对象的函数调用调用pm指向的成员函数。
			t.detach();//启动线程

			//调用OnWork函数 唤醒线程
		}


	}
	//关闭
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			//等待线程正常退出
			_isRun = false;
			_sem.wait();////线程阻塞通常是指一个线程在执行过程中暂停，以等待某个条件的触发。
			//阻塞该线程
		}
	}
	//在工作函数中推出
	//不需要使用信号量来组赛等待
	//如果使用会阻塞住
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			//等待线程正常退出
			_isRun = false;


		}
	}

	//线程是否启动运行
	bool isRun()
	{
		return _isRun;
	}
protected:
	//线程运行 工作函数
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		_sem.wakeup();//线程唤醒
	}
private:

	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;

	//改变数据时需要加锁
	std::mutex _mutex;
	//控制线程推出
	CELLSemaphore _sem;
	//线程是否启动
	bool _isRun = false;
};



#endif // !_CELL_THREAD_HPP_
