//包含心跳检测 定时发送数据
//操作收发数据
//发送数据为定时发送

//用法创建一个CELLClient 集合

#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "CELL.hpp"
#include "CELLBuffer.hpp"
//客户端心跳检测死亡计时时间 60miao
#define CLIENT_HEART_DEAD_TIME 60000
//在间隔时间内将发送缓冲区的数据 发送给客户端 0.2秒
#define CLIENT_SEND_BUFF_TIME 200
//心跳检测
//计时 超时即判定该连接 死亡
//设定方法重置该时间

//客户端数据类型
class CELLClient
{
public:
	int id = -1;
	//所属server的ID
	int serverId = -1;
public:
	//创建client 并创建收发缓冲区 分包拆包
	CELLClient(SOCKET sockfd = INVALID_SOCKET) :
		_sendBuff(SEND_BUFF_SZIE),
		_recvBuff(RECV_BUFF_SZIE)
	{
		//
		static int n = 1;//给每个客户端一个ID
		id = n++;
		//
		_sockfd = sockfd;


		//心跳检测
		//开始时要重置一下时间
		resetDTHeart();
		resetDTSend();
	}

	~CELLClient()
	{
		CELLLog::Info("s=%d,CELLClient%d .~CELLClient 1\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}

	}

	//取socket
	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);//收
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();//是否有msg
	}

	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();//去缓冲区首地址
	}

	void pop_front_msg()
	{
		if (hasMsg())
			_recvBuff.pop(front_msg()->dataLength);//弹出首msg

	}
	//立即发送数据

	/*void SendDataReal(netmsg_DataHeader* header)
	{
	SendData(header);
	SendDataReal();
	}*/

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{

		resetDTSend();

		return _sendBuff.write2socket(_sockfd);
	}
	//缓冲区控制根据业务需求
	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		//int ret = SOCKET_ERROR;
		////要发送的数据长度
		//int nSendLen = header->dataLength;
		////要发送的数据
		//const char* pSendData = (const char*)header;

		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}

		return SOCKET_ERROR;
	}

	//重置时间方法
	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	//重置时间方法
	void resetDTSend()
	{
		_dtSend = 0;
	}


	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%d,time = %d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//定时发送消息
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d,time = %d\n", _sockfd, _dtSend);
			//立即发送缓冲区数据数据
			SendDataReal();
			//重置发送计时
			resetDTSend();
			return true;
		}
		return false;
	}


private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//第二缓冲区 接收消息缓冲区
	CELLBuffer _recvBuff;
	//发送缓冲区
	CELLBuffer _sendBuff;

	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息数据的时间
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;
};
#endif // !_CellClient_hpp_




