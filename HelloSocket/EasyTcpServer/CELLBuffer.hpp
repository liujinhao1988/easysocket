#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

//����buff


//memcpy����ָ����C��C++ʹ�õ��ڴ濽������  void *memcpy(void *destin, void *source, unsigned n);
//destin-- ָ�����ڴ洢�������ݵ�Ŀ�����飬����ǿ��ת��Ϊ void* ָ�롣
//source-- ָ��Ҫ���Ƶ�����Դ������ǿ��ת��Ϊ void* ָ�롣
//n-- Ҫ�����Ƶ��ֽ�����



#include"CELL.hpp"
class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)//����
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];//�ͷ�
	}
	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data() //ȡ��ַ
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		////д��������ݲ�һ��Ҫ�ŵ��ڴ���
		////Ҳ���Դ洢�����ݿ���ߴ��̵ȴ洢����
		//if (_nLast + nLen > _nSize)
		//{
		//	//��Ҫд������ݴ��ڿ��ÿռ�
		//	int n = (_nLast + nLen) - _nSize;
		//	//��չBUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}
		if (_nLast + nLen <= _nSize)//��д��nLen����  ������û��
		{
			//��Ҫ���͵����� ���������ͻ�����β��
			memcpy(_pBuff + _nLast, pData, nLen);
			//��������β��λ��
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

	void pop(int nLen)//��������ͷ��nLen�������ݵ���������
	{
		int n = _nLast - nLen;//�ȼ���ĵ�����β��λ��
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);//����ͷ��nLen����

		}
		_nLast = n;//���º���ȥβ��λ��
		if (_fullCount > 0)
		{
			--_fullCount;
		}
	}

	int write2socket(SOCKET sockfd)//���������������ݷ��͸�sockfd//д��socket
	{
		int ret = 0;
		//������������
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//��������
			ret = send(sockfd, _pBuff, _nLast, 0);
			//����β��λ������
			_nLast = 0;
			//
			_fullCount = 0;

		}
		return ret;
	}


	int read4socket(SOCKET sockfd)//��socket�����ȡ����
	{
		if (_nSize - _nLast > 0)//�жϻ������Ƿ�����
		{
			//δ��
			//���տͻ�������
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);

			//CELLLog::Info("nLen=%d\n", nLen);
			if (nLen <= 0)
			{
				//CELLLog::Info("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
				return nLen;
			}
			//��Ϣ������������β��λ�ú���
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()//�жϻ��������Ƿ�������
	{
		//�ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����

			return _nLast >= header->dataLength;
		}
		//ԭ����return true;
		return false;
	}


private:
	//�ڶ������� ���ͻ�����
	char* _pBuff = nullptr;
	//�������������������������ݿ�
	//list<char*> _pBuffList;
	//���ͻ�����������β��λ�� �������ݳ���
	int _nLast=0;
	//�������ܵĿռ��ֽڳ���
	int _nSize=0;
	//������д������
	int _fullCount = 0;//�����жϻ�������������
};








#endif // !_CELL_BUFFER_HPP_
