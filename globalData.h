#ifndef _GLOBALDATA_H_
#define _GLOBALDATA_H_

#include <arpa/inet.h>

#define VERSION "2.1.0"//�汾��
#define DDOS_NO_CONTROL_ON //���ܿ��ƿ���
#define DDOS_NO_SEND_IP//���Զ�����ƶ˷���IP����

#define getOnePackageTime 70 //����������ʹ��:��֯һ�����ݰ���ƽ��ʱ�䣬��Ӱ�����������з������ʱ��ľ�ȷ����λ΢�� 

#define LENGTH_OF_LISTEN_QUEUE 1//1�����ƶ�
#define BUFFER_SIZE 1024
#define MAX_SIZE 512
#define MAXLENGTH 1486 //���IP����1500-14(14Byte��̫��ͷ)
#define MAX_PTHREAD_NUM 10 //ÿ�ο��Կ�������󹥻��߳����������������

#define int_ntoa(x)     inet_ntoa(*((struct in_addr *)&x))
#define int_aton(x)     inet_addr(x)

/********************************************************************************/
/* ȫ�����ñ���                                                                 */
/********************************************************************************/
char server_ip[16];
int port_server;
int port_server_sendip;
int port_server_sendresult;
int wait_time;

#endif

