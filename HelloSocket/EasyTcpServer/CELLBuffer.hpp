#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

//缓冲buff


//memcpy函数指的是C和C++使用的内存拷贝函数  void *memcpy(void *destin, void *source, unsigned n);
//destin-- 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针。
//source-- 指向要复制的数据源，类型强制转换为 void* 指针。
//n-- 要被复制的字节数。



#include"CELL.hpp"
class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)//申请
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];//释放
	}
	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data() //取地址
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		////写入大量数据不一定要放到内存中
		////也可以存储到数据库或者磁盘等存储器中
		//if (_nLast + nLen > _nSize)
		//{
		//	//需要写入的数据大于可用空间
		//	int n = (_nLast + nLen) - _nSize;
		//	//拓展BUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}
		if (_nLast + nLen <= _nSize)//再写入nLen内容  缓冲区没满
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SZIE)
			{
				++_fullCount;
			}

			return true;
		}
		else {
			++_fullCount;
		}
		return false;
	}

	void pop(int nLen)//将缓冲区头部nLen长度内容弹出缓冲区
	{
		int n = _nLast - nLen;//先计算的弹出后尾部位置
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);//弹出头部nLen数据

		}
		_nLast = n;//更新韩冲去尾部位置
		if (_fullCount > 0)
		{
			--_fullCount;
		}
	}

	int write2socket(SOCKET sockfd)//将缓冲区所有数据发送跟sockfd//写入socket
	{
		int ret = 0;
		//缓冲区有数据
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _nLast, 0);
			//数据尾部位置清零
			_nLast = 0;
			//
			_fullCount = 0;

		}
		return ret;
	}


	int read4socket(SOCKET sockfd)//从socket那里读取数据
	{
		if (_nSize - _nLast > 0)//判断缓冲区是否收满
		{
			//未满
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);

			//CELLLog::Info("nLen=%d\n", nLen);
			if (nLen <= 0)
			{
				//CELLLog::Info("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
				return nLen;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()//判断缓冲区里是否有数据
	{
		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			//这时就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度

			return _nLast >= header->dataLength;
		}
		//原来是return true;
		return false;
	}


private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//发送缓冲区的数据尾部位置 已有数据长度
	int _nLast=0;
	//缓冲区总的空间字节长度
	int _nSize=0;
	//缓冲区写满计数
	int _fullCount = 0;//用于判断缓冲区够不够用
};








#endif // !_CELL_BUFFER_HPP_
