//�ź���
#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include<chrono>//ʱ�����ͷ�ļ�
#include<thread>//�߳�

//��������
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
	//������ǰ�߳�
	void wait()//�߳�����ͨ����ָһ���߳���ִ�й�������ͣ���Եȴ�ĳ�������Ĵ�����
	{
		//std::lock_guard<std::mutex> lock(_mutex);//
		std::unique_lock<std::mutex> lock(_mutex);//��lock_guard���ɰ��������������ǲ���
		if (--_wait < 0)
		{
			//�����ȴ�
			_cv.wait(lock, [this]()->bool {return _wakeup > 0; });//wait ���ڶ�������false�� �Ż�������ǰ�߳� true ��������
			--_wakeup;
		}
		///_isWaitExit = true;
		

		/*
		while (_isWaitExit)
		{//�ź���
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
		*/
	}
	//���ѵ�ǰ�߳�
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait<=0)
		{
			++_wakeup;
			///_isWaitExit = false;
			_cv.notify_one();//notify_one   ����ĳ���ȴ�(wait)�̡߳������ǰû�еȴ��̣߳���ú���ʲôҲ������
		}
			
		
	}
private:
	
	//�����ȴ�-��������
	std::condition_variable _cv;
	//
	std::mutex _mutex;
	//�ȴ�����
	int _wait = 0;

	//���Ѽ���
	int _wakeup = 0;

	//bool _isWaitExit = false;
};



#endif // !_CELL_SEMAPHORE_HPP_
//��ٻ���