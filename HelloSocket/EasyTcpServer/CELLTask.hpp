//����������
//��Ҫִ�еĺ�������CELLTaskServer�д洢
//������ִ��
#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>

//#include"CELLSemaphore.hpp"
//#include"CELLLog.hpp"
#include"CELLThread.hpp"
//��������-����
/*
class CellTask
{
public:
CellTask()
{

}

//������
virtual ~CellTask()
{

}
//ִ������
virtual void doTask()
{

}
private:

};
*/




//ִ������ķ�������
class CELLTaskServer
{
public:
	//����server��ID
	int serverId = -1;
private:
	typedef std::function<void()> CellTask;//function ����ͨ������Ϊ������ָ�룬��������Ҳ���Գ�Ϊ���� �����е�Ϊ������������
private:
	//��������
	std::list<CellTask> _tasks;
	//�������ݻ�����
	std::list<CellTask> _tasksBuf;
	//�ı����ݻ�����ʱ��Ҫ����
	std::mutex _mutex;
	//
	CELLThread _thread;
public:
	//�������
	void addTask(CellTask task)
	{
		std::lock_guard<std::mutex> lock(_mutex);//����̹߳������� ����
		_tasksBuf.push_back(task);
	}
	//���������߳�
	void Start()
	{
		/*
		_isRun = true;
		//�߳�
		std::thread t(std::mem_fn(&CELLTaskServer::OnRun),this);
		t.detach();
		*/
		_thread.Start(nullptr, [this](CELLThread* pThread) {
			OnRun(pThread);
		});//�߳����� onrun ����
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
		_thread.Close();//�̹߳ر�

		//CELLLog::Info("CELLTaskServer%d .Close end\n", serverId);
	}
protected:
	//��������
	void OnRun(CELLThread* pThread)//
	{
		while (pThread->isRun())
		{
			//�ӻ�����ȡ������
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//���û������
			if (_tasks.empty())
			{
				//��ͣ ��ֹ�˷���Դ
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//��������
			for (auto pTask : _tasks)
			{
				pTask();

			}
			//�������
			_tasks.clear();
		}


		//����buff�����������
		for (auto pTask : _tasksBuf)
		{
			pTask();

		}
		//CELLLog::Info("CELLTaskServer%d .OnRun exit\n", serverId);
		//_sem.wakeup();
	}
};
#endif // !_CELL_TASK_H_
