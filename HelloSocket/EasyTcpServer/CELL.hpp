//����ͷ�ļ�
//����ȫ�ֱ���
//���������ļ�����Ҫ���������
#ifndef _CELL_HPP_
#define _CELL_HPP_

//socket���
#ifdef _WIN32   //windows������Щ
#define FD_SETSIZE      2506//FD_SETSIZE select�����socket����
#define WIN32_LEAN_AND_MEAN   //�ӿ�����ٶ�
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>   //windows �� socket
#pragma comment(lib,"ws2_32.lib")  //�������API��֧��  //˳���ܱ䣬��������
#else           //linux�е�����Щ
#include<unistd.h> //uni std  //unistd.h �� C �� C++ ��������������ṩ�� POSIX ����ϵͳ API �ķ��ʹ��ܵ�ͷ�ļ������ơ�
#include<arpa/inet.h>//Linux socket���ͷ�ļ�
#include<string.h>
#include<signal.h>//ͷ�ļ�<signal.h>���ṩ��һЩ���ڴ�����������ڼ����������쳣�����Ĺ��ܣ��紦����Դ���ⲿ���ж��źŻ����ִ���ڼ���ֵĴ�����¼���

#define SOCKET int                      //win ��linux ���Ͳ�ͬ
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif


//�Զ���


#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include"CELLTask.hpp"
#include"CELLLog.hpp"
//
#include<stdio.h>
//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 8192
#define SEND_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

#endif // !_CELL_HPP_
