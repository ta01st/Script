#ifndef _ATTACK_H_
#define _ATTACK_H_

#define MAXSIZE 1024
//#define getOnePackageTime 70 //��֯һ�����ݰ���ƽ��ʱ�䣬��Ӱ�����������з������ʱ��ľ�ȷ����λ΢��

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>

struct attackM_st
{
	char * attackIp;
	char * attackPort;
	int pthread_id;
};

/********************************************************************************/
/* ��ʼ������������                                                             */
/* �������:xml�ļ���·��                                                       */
/********************************************************************************/
void *startAttack(void * docname);

#endif
