//公共头文件
//放置全局变量
//包含其他文件都需要导入的内容
#ifndef _CELL_HPP_
#define _CELL_HPP_

//socket相关
#ifdef _WIN32   //windows调用这些
#define FD_SETSIZE      2506//FD_SETSIZE select的最大socket数量
#define WIN32_LEAN_AND_MEAN   //加快编译速度
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>   //windows 的 socket
#pragma comment(lib,"ws2_32.lib")  //网络相关API的支持  //顺序不能变，否则会出错
#else           //linux中调用这些
#include<unistd.h> //uni std  //unistd.h 是 C 和 C++ 程序设计语言中提供对 POSIX 操作系统 API 的访问功能的头文件的名称。
#include<arpa/inet.h>//Linux socket编程头文件
#include<string.h>
#include<signal.h>//头文件<signal.h>中提供了一些用于处理程序运行期间所引发的异常条件的功能，如处理来源于外部的中断信号或程序执行期间出现的错误等事件。

#define SOCKET int                      //win 和linux 类型不同
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif


//自定义


#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include"CELLTask.hpp"
#include"CELLLog.hpp"
//
#include<stdio.h>
//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 8192
#define SEND_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

#endif // !_CELL_HPP_
