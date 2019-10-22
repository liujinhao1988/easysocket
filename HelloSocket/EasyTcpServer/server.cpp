#include "EasyTcpServer.hpp"

#include<thread>
/*
bool g_bRun = true;
void cmdThread()
{//
while (true)
{
char cmdBuf[256] = {};
scanf("%s", cmdBuf);
if (0 == strcmp(cmdBuf, "exit"))
{
g_bRun = false;
CELLLog::Info("退出cmdThread线程\n");
break;
}
else {
CELLLog::Info("不支持的命令。\n");
}
}
}
*/

class MyServer : public EasyTcpServer
{
public:

	//只会被一个线程触发 安全
	virtual void OnNetJoin(CELLClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(CELLClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//心跳检测重置_dtHeader

			//客户端有数据来认为他心跳了
			pClient->resetDTHeart();
			//send recv 
			netmsg_Login* login = (netmsg_Login*)header;
			//CELLLog::Info("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			netmsg_LoginR ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				CELLLog::Info("<Socket=%d>SEND FULL\n", pClient->sockfd());
				//发送缓冲区满了，消息没发出去
				//CELLLog::Info("<Socket=%d>SEND FULL\n", pClient->sockfd());
			}
			//pClient->SendData(&ret);
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pCellServer->addSendTask(pClient, ret);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			//CELLLog::Info("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			//心跳检测重置_dtHeader

			//客户端有数据来认为他心跳了
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		break;
		default:
		{
			CELLLog::Info("<socket=%d>收到未定义消息,数据长度：%d\n", pClient->sockfd(), header->dataLength);
			//netmsg_DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}
private:

};

int main()
{
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);

	////启动UI线程
	//std::thread t1(cmdThread);
	//t1.detach();

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			CELLLog::Info("undefine cmd\n");
		}
	}

	CELLLog::Info("exit.\n");
//#ifdef _WIN32
//	while (true)
//		Sleep(10);
//#endif
	return 0;
}