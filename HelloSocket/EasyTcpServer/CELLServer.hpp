#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_



#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include"CELLThread.hpp"

#include<vector>
#include<map>

//������Ϣ��������
/*
class CellSendMsg2ClientTask :public CellTask
{
CELLClient* _pClient;
netmsg_DataHeader* _pHeader;
public:
CellSendMsg2ClientTask(CELLClient* pClient, netmsg_DataHeader* header)
{
_pClient = pClient;
_pHeader = header;
}

//ִ������
void doTask()
{
_pClient->SendData(_pHeader);
delete _pHeader;
}
};
*/

//������Ϣ���մ��������
class CELLServer
{
public:
	CELLServer(int id)
	{
		_id = id;
		_pNetEvent = nullptr;
		_taskServer.serverId = id;
	}

	~CELLServer()
	{
		//_isRun = false;//
		CELLLog::Info("CELLServer%d .~CELLServer exit begin\n", _id);
		Close();
		CELLLog::Info("CELLServer%d .~CELLServer exit end\n", _id);
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//�ر�Socket �ر���������� �ر��߳�
	void Close()
	{
		CELLLog::Info("CELLServer%d .Close  begin\n", _id);
		_taskServer.Close();

		_thread.Close();

		CELLLog::Info("CELLServer%d .Close  end\n", _id);
	}

	//�Ƿ�����


	//����������Ϣ

	void OnRun(CELLThread* pThread)//��task����
	{

		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{

					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
					{
						_pNetEvent->OnNetJoin(pClient);
					}

				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//������� ����ʱ���
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			fd_set fdWrite;
			//fd_set fdExc;
			//��ȡ���socket
			//���ı� ˢ��fd_set ����ˢ��
			if (_clients_change)
			{
				_clients_change = false;
				//������
				FD_ZERO(&fdRead);
				//����������socket�����뼯��
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));//fdRead �� fdWrite����ͬ

			//memcpy(&fdExc, &_fdRead_bak, sizeof(fd_set));
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0

			timeval t{ 0,1 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);//��д������
			if (ret < 0)
			{
				CELLLog::Info("CELLServer%d .OnRun select Error exit\n", _id);

				pThread->Exit();
				break;
			}

			/*else if (ret == 0)
			{
			continue;
			}*/

			ReadData(fdRead);
			WriteData(fdWrite);
			//WriteData(fdExc);
			//CELLLog::Info("CELLServer%d .OnRun.select : fdRead= %d,fdWrite = %d\n",_id, fdRead.fd_count, fdWrite.fd_count);
			/*if (fdExc.fd_count > 0)
			{
			CELLLog::Info("###fdExc=%d\n", fdExc.fd_count);
			}*/

			CheckTime();
		}
		CELLLog::Info("CELLServer%d .OnRun  exit\n", _id);


	}

	//�������
	void CheckTime()
	{
		//��ǰʱ���
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			//�������
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}

			////��ʱ���ͼ��
			//iter->second->checkSend(dt);

			iter++;
		}
	}

	void OnClientLeave(CELLClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	void WriteData(fd_set& fdWrite)
	{
#ifdef _WIN32
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		for (int n = 0; n < fdRead.fd_count; n++)//����fdRead
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))//������
				{
					OnClientLeave(iter->second);//�Ͽ� ���socket
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(CELLClient* pClient)
	{
		//���տͻ�������		
		int nLen = pClient->RecvData();

		//CELLLog::Info("nLen=%d\n", nLen);
		if (nLen <= 0)
		{
			//CELLLog::Info("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//���յ����������¼�
		_pNetEvent->OnNetRecv(pClient);
		//�ж��Ƿ���������Ϣ��Ҫ����
		while (pClient->hasMsg())
		{
			//����������Ϣ
			OnNetMsg(pClient, pClient->front_msg());
			//�Ƴ���Ϣ���У��������� ��ǰ��һ������			
			pClient->pop_front_msg();
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(CELLClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		/*
		if (!_isRun)
		{
		_isRun = true;
		std::thread t = std::thread(std::mem_fn(&CELLServer::OnRun), this);
		t.detach();

		}
		*/
		_taskServer.Start();
		_thread.Start(
			//onCreate
			nullptr,
			//onRun
			[this](CELLThread* pThread) {
			OnRun(pThread);
		},
			//onDestory
			[this](CELLThread* pThread) {
			ClearClients();
		}
		);//����onrun����
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	/*
	void addSendTask(CELLClient* pClient, netmsg_DataHeader* header)
	{
	//CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
	_taskServer.addTask([pClient, header]() {pClient->SendData(header); delete header; });
	}
	*/

private:
	void ClearClients()//����ͻ���
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
private:
	//��ʽ�ͻ�����
	std::map<SOCKET, CELLClient*> _clients;
	//����ͻ�����
	std::vector<CELLClient*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	//�����¼�����
	INetEvent* _pNetEvent;
	//��������
	CELLTaskServer _taskServer;
	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;
	//
	SOCKET _maxSock;
	//�ɵ�ʱ���
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
	//
	int _id = -1;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change = true;

};


#endif // !_CELL_SERVER_HPP_
