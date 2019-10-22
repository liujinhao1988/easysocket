#ifndef _CELLObjectPool_HPP_
#define _CELLObjectPool_HPP_
#include<stdlib.h>
#include<mutex>
#include<assert.h>


#ifdef _DEBUG//调试技巧
#ifndef xPrintf
#include<stdio.h>
#define xPrintf(...)  printf(__VA_ARGS__)//可变参数
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

		//下一块位置
		NodeHeader* pNext;
		int nID;//内存块编号
				//引用次数
		char nRef;
		//是否在内存池中
		bool bPool;
	private:
		//预留
		char c1;
		char c2;

	};



public:
	//释放对象内存
	void freeObjMemory(void* pMem)
	{

		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));//指针偏移,内存块包括描述信息和实际可用内存，所以原指针指向实际可用内存开始，向前偏移一下指向此内存块的开始
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
	//申请对象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);//锁的作用，为保护共享数据_pHeader
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader*) new char[sizeof(NodeHeader) + sizeof(Type)];//向内存池申请new
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
		return (((char*)pReturn) + sizeof(NodeHeader));//指向实际可用内存的首地址
	}
	//初始化对象池
private:
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		//计算对象池大小
		size_t realSzie = sizeof(NodeHeader) + sizeof(Type);
		size_t n = nPoolSize*realSzie;
		//申请池的内存
		_pBuf = new char[n];//new
							//初始化对象池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;
		//遍历内存块初始化
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
	//对象池地址
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
	//参数不固定 可变参数
	template<typename ...Args>//
	static Type* createObject(Args ... args)
	{
		Type* obj = new Type(args...);

		//可以做想做的事情
		return  obj;
	}
	static void destroyObject(Type* obj)
	{
		delete obj;
	}

private:
	typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;

	static ClassTypePool& objectPool()
	{	//静态CELLObjectPool对象
		static ClassTypePool sPool;
		return sPool;
	}

};









#endif // !_CELLObjectPool_HPP_

