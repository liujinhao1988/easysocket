#ifndef _CELLObjectPool_HPP_
#define _CELLObjectPool_HPP_
#include<stdlib.h>
#include<mutex>
#include<assert.h>


#ifdef _DEBUG//���Լ���
#ifndef xPrintf
#include<stdio.h>
#define xPrintf(...)  printf(__VA_ARGS__)//�ɱ����
#endif // !xPrintf	
#else
#ifndef xPrintf
#define xPrintf(...)
#endif // !xPrintf		
#endif // _DEBUG


template<class Type, size_t nPoolSize>
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		_pBuf = nullptr;
		initPool();
	}
	~CELLObjectPool()
	{
		if (_pBuf)
			delete[] _pBuf;

	}
private:
	class NodeHeader
	{
	public:

		//��һ��λ��
		NodeHeader* pNext;
		int nID;//�ڴ����
				//���ô���
		char nRef;
		//�Ƿ����ڴ����
		bool bPool;
	private:
		//Ԥ��
		char c1;
		char c2;

	};



public:
	//�ͷŶ����ڴ�
	void freeObjMemory(void* pMem)
	{

		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));//ָ��ƫ��,�ڴ�����������Ϣ��ʵ�ʿ����ڴ棬����ԭָ��ָ��ʵ�ʿ����ڴ濪ʼ����ǰƫ��һ��ָ����ڴ��Ŀ�ʼ
		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;

		}

	}
	//��������ڴ�
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);//�������ã�Ϊ������������_pHeader
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader*) new char[sizeof(NodeHeader) + sizeof(Type)];//���ڴ������new
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;

		}
		xPrintf("allocObjMemory: %llx,id=%d,size=%d\n", pReturn, pReturn->nID, nSize);
		return (((char*)pReturn) + sizeof(NodeHeader));//ָ��ʵ�ʿ����ڴ���׵�ַ
	}
	//��ʼ�������
private:
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//�������ش�С
		size_t realSzie = sizeof(NodeHeader) + sizeof(Type);
		size_t n = nPoolSize*realSzie;
		//����ص��ڴ�
		_pBuf = new char[n];//new
							//��ʼ�������
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//�����ڴ���ʼ��
		NodeHeader* pTemp1 = _pHeader;

		for (size_t n = 1; n < nPoolSize; n++)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n* realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
private:
	NodeHeader* _pHeader;
	//����ص�ַ
	char* _pBuf;

	std::mutex _mutex;
};


template<class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:
	void* operator new(size_t nSize)
	{

		return objectPool().allocObjMemory(nSize);
	}

	void operator delete(void*p)
	{
		objectPool().freeObjMemory(p);
	}
	//�������̶� �ɱ����
	template<typename ...Args>//
	static Type* createObject(Args ... args)
	{
		Type* obj = new Type(args...);

		//����������������
		return  obj;
	}
	static void destroyObject(Type* obj)
	{
		delete obj;
	}

private:
	typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;

	static ClassTypePool& objectPool()
	{	//��̬CELLObjectPool����
		static ClassTypePool sPool;
		return sPool;
	}

};









#endif // !_CELLObjectPool_HPP_

