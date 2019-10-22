#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CELLSemaphore.hpp"
class CELLThread
{
private:
	typedef std::function<void(CELLThread*)> EventCall;//function ����ͨ������Ϊ������ָ�룬��������Ҳ���Գ�Ϊ���� �����е�Ϊ������������
public:
	//���� ���� ������ָ��
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
			//�߳�
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);//mem_fn ����Ա����ת��Ϊ�������� ����һ���������󣬸ú�������ĺ������õ���pmָ��ĳ�Ա������
			t.detach();//�����߳�

			//����OnWork���� �����߳�
		}


	}
	//�ر�
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			//�ȴ��߳������˳�
			_isRun = false;
			_sem.wait();////�߳�����ͨ����ָһ���߳���ִ�й�������ͣ���Եȴ�ĳ�������Ĵ�����
			//�������߳�
		}
	}
	//�ڹ����������Ƴ�
	//����Ҫʹ���ź����������ȴ�
	//���ʹ�û�����ס
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			//�ȴ��߳������˳�
			_isRun = false;


		}
	}

	//�߳��Ƿ���������
	bool isRun()
	{
		return _isRun;
	}
protected:
	//�߳����� ��������
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		_sem.wakeup();//�̻߳���
	}
private:

	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;

	//�ı�����ʱ��Ҫ����
	std::mutex _mutex;
	//�����߳��Ƴ�
	CELLSemaphore _sem;
	//�߳��Ƿ�����
	bool _isRun = false;
};



#endif // !_CELL_THREAD_HPP_
