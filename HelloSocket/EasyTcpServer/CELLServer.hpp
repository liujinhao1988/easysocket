#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_



#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include"CELLThread.hpp"

#include<vector>
#include<map>

//网络消息发送任务
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

//执行任务
void doTask()
{
_pClient->SendData(_pHeader);
delete _pHeader;
}
};
*/

//网络消息接收处理服务类
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

	//关闭Socket 关闭任务处理对象 关闭线程
	void Close()
	{
		CELLLog::Info("CELLServer%d .Close  begin\n", _id);
		_taskServer.Close();

		_thread.Close();

		CELLLog::Info("CELLServer%d .Close  end\n", _id);
	}

	//是否工作中


	//处理网络消息

	void OnRun(CELLThread* pThread)//以task启动
	{

		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{//从缓冲队列里取出客户数据
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

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//心跳检测 更新时间戳
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			fd_set fdWrite;
			//fd_set fdExc;
			//获取最大socket
			//若改变 刷新fd_set 否则不刷新
			if (_clients_change)
			{
				_clients_change = false;
				//清理集合
				FD_ZERO(&fdRead);
				//将描述符（socket）加入集合
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
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));//fdRead 和 fdWrite都相同

			//memcpy(&fdExc, &_fdRead_bak, sizeof(fd_set));
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0

			timeval t{ 0,1 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);//读写非阻塞
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

	//心跳检测
	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			//心跳检测
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

			////定时发送检测
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
		for (int n = 0; n < fdRead.fd_count; n++)//遍历fdRead
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))//收数据
				{
					OnClientLeave(iter->second);//断开 清楚socket
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
	//接收数据 处理粘包 拆分包
	int RecvData(CELLClient* pClient)
	{
		//接收客户端数据		
		int nLen = pClient->RecvData();

		//CELLLog::Info("nLen=%d\n", nLen);
		if (nLen <= 0)
		{
			//CELLLog::Info("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//接收到网络数据事件
		_pNetEvent->OnNetRecv(pClient);
		//判断是否有网络消息需要处理
		while (pClient->hasMsg())
		{
			//处理网络消息
			OnNetMsg(pClient, pClient->front_msg());
			//移除消息队列（缓冲区） 最前的一条数据			
			pClient->pop_front_msg();
		}
		return 0;
	}

	//响应网络消息
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
		);//启动onrun函数
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
	void ClearClients()//清理客户端
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
	//正式客户队列
	std::map<SOCKET, CELLClient*> _clients;
	//缓冲客户队列
	std::vector<CELLClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//处理任务
	CELLTaskServer _taskServer;
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//
	SOCKET _maxSock;
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;

};


#endif // !_CELL_SERVER_HPP_
