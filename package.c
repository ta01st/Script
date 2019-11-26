/********************************************************************************/
/* �������                                                                     */
/********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "synCon.h"
#include "udpCon.h"
#include "icmpCon.h"
#include "httpCon.h"
#include "dataList.h"
#include "package.h"
#include "globalData.h"
#include "packetTime.h"
/********************************************************************************/
/* checkУ���                                                                  */
/********************************************************************************/
unsigned short checksum(unsigned short * data,unsigned short length)
{
    unsigned long sum=0;
    while (length > 1){
        sum += *data++;
        length -= sizeof(unsigned short);
    }
    if (length){
        sum += *(unsigned char *)data;
    }  
    sum = (sum >>16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}
/********************************************************************************/
/* ��֯ip_tcp��                                                                 */
/********************************************************************************/
char *ip_tcp_package(synCon * synConTemp,char * attackipstr,char * attackportstr)
{
	struct iphdr * ip_header;
	struct tcphdr * tcp_header;
	char dataArrary[MAXLENGTH];//������
	char *datap;//ָ������������ָ��
	char dataPac[MAXLENGTH];//��������
	char tcp_temp[MAXLENGTH+12];//��¼tcp��У������(����αͷ����Ҫ����У��,12λαͷ����
	int packageLength = synConTemp->packetLength;
	int dataLength = packageLength-sizeof(struct iphdr)-sizeof(struct tcphdr);//������ݳ��ȣ������ȣ�����ͷ����
	struct psdhdr_tcp psdtcp_header;//tcpα��ͷ+tcp��ͷ
	//init
	bzero(&dataPac,MAXLENGTH);
	bzero(&dataArrary,MAXLENGTH);
	bzero(&tcp_temp,MAXLENGTH+12);
	ip_header = (struct iphdr *)malloc(sizeof(struct iphdr));
	tcp_header = (struct tcphdr *)malloc(sizeof(struct tcphdr));
	//set IPHedaer
	ip_header->ihl=sizeof(struct iphdr)/4;//ipͷ����
	ip_header->version=4;//�汾��
	ip_header->tos=0;
	ip_header->tot_len = packageLength;
	ip_header->id=htons(random());
	ip_header->frag_off=0;
	ip_header->ttl=synConTemp->ipHeader->ttl;//����ʱ��
	ip_header->protocol=IPPROTO_TCP; //����Э��
	ip_header->check=0;
	if(synConTemp->srcIpAddressRandom==1) { //Դip��ַ���
		if(synConTemp->srcIpMeth==1) { //���ȷֲ�
			//ip_header->saddr=inet_addr(getSomeone_arr(synConTemp->srcIpls,getRandomNumber(getLinkLength_arr(synConTemp->srcIpls))));//������α��Դip��ַ
            ip_header->saddr = htonl(getRandomNumberFT(synConTemp->srcip_s,synConTemp->srcip_e));
			//printf("[%d]\n",getRandomNumber(getLinkLength_arr(synConTemp->srcIpls)));
		}
		else{
			////�Ǿ��ȷֲ�
		}
	}
	else { //Դip��ַָ��
		ip_header->saddr=inet_addr(synConTemp->srcIpValue);
	}
	ip_header->daddr = inet_addr(attackipstr);
	
	//set TCPHedaer
	//tcp_header = synConTemp->tcpHeader;
	if(synConTemp->srcPortRandom==1) { //Դ�˿����
		if(synConTemp->srcPortMeth==1) { //���ȷֲ�
			tcp_header->source=htons(getRandomNumberFT(synConTemp->srcport_s,synConTemp->srcport_e));//������α��Դ�˿ں�
		}
		else{
			////�Ǿ��ȷֲ�
		}	
	}
	else { //Դ�˿ں�ָ��
		tcp_header->source=htons(synConTemp->srcPortValue);
	}
	unsigned int a = random();
	tcp_header->seq = htonl(a);
	tcp_header->ack_seq = synConTemp->tcpHeader->ack_seq;
	tcp_header->doff = synConTemp->tcpHeader->doff;
	tcp_header->urg = synConTemp->tcpHeader->urg;
	tcp_header->ack = synConTemp->tcpHeader->ack;
	tcp_header->psh = synConTemp->tcpHeader->psh;
	tcp_header->rst = synConTemp->tcpHeader->rst;
	tcp_header->syn = synConTemp->tcpHeader->syn;
	tcp_header->fin = synConTemp->tcpHeader->fin;
	tcp_header->window = synConTemp->tcpHeader->window;
	tcp_header->check = synConTemp->tcpHeader->check;
	tcp_header->urg_ptr = synConTemp->tcpHeader->urg_ptr;
	tcp_header->dest = htons(atoi(attackportstr));
	//set tcpαͷ
	psdtcp_header.saddr = ip_header->saddr;
	psdtcp_header.daddr = ip_header->daddr;
	psdtcp_header.mbz=0;
	psdtcp_header.ptcl = IPPROTO_TCP;
	psdtcp_header.tcpl = htons(packageLength-sizeof(struct iphdr));//tcp������(����tcpͷ��+tcp����������)
	bcopy((char *)tcp_header,(char *)&psdtcp_header.tcpheader,sizeof(struct tcphdr));
	//set data/
	int i=0;
	char * getdataTemp;
	int datatemplen;
	if(dataLength>0){
		if(synConTemp->dataRandom==1) { //�������
			if(synConTemp->dataMeth==1) { //���ȷֲ�
				int linklen = getLinkLength_arr(synConTemp->datals);
				bzero(dataPac,sizeof(dataPac));
				while(i<dataLength){
					getdataTemp=getSomeone_arr(synConTemp->datals,getRandomNumber(linklen));//����������
					datatemplen = strlen(getdataTemp);
					if(i+datatemplen<dataLength){
						memcpy(dataPac+i,getdataTemp,datatemplen);
						i=i+datatemplen;
					}
					else{
						break;
					}
				}
			}
			else{
				////�Ǿ��ȷֲ�
			}
		}
		else { //����ָ��
			memset(dataPac,'1',sizeof(dataPac));//�������ֶ�ȫ����ʼΪ1
			memcpy(dataPac,synConTemp->dataValue,strlen(synConTemp->dataValue));
		}
	}
	//printf("dataPac:%s\n",dataPac);
	memcpy(tcp_temp,&psdtcp_header,sizeof(struct psdhdr_tcp));
	memcpy(tcp_temp+sizeof(struct psdhdr_tcp),&dataPac,dataLength);
	//get tcp checksum
	if(synConTemp->tcpHeader->check==0)
		tcp_header->check=checksum((unsigned short *)&tcp_temp,sizeof(struct psdhdr_tcp)+dataLength);
	else
		tcp_header->check=synConTemp->tcpHeader->check;
	//get ip checksum
	if(synConTemp->ipHeader->check!=0)
		ip_header->check=synConTemp->ipHeader->check;
	else
		ip_header->check=checksum((unsigned short *)&ip_header,sizeof(struct iphdr));
	memcpy(dataArrary,ip_header,sizeof(struct iphdr));
	memcpy(dataArrary+sizeof(struct iphdr),tcp_header,sizeof(struct tcphdr));
	//free
	if(tcp_header)
		free(tcp_header);
	if(ip_header)
		free(ip_header);
	
	if(dataLength>0)
		memcpy(dataArrary+sizeof(struct iphdr)+sizeof(struct tcphdr),dataPac,dataLength);
	datap = dataArrary;
	return datap;
}
/********************************************************************************/
/* ��֯ip_udp��                                                                 */
/********************************************************************************/
char * ip_udp_package(udpCon * udpConTemp,char * attackipstr,char * attackportstr)
{
	struct iphdr * ip_header;
	struct udphdr * udp_header;
	int packageLength = udpConTemp->packetLength;
	int dataLength = packageLength-sizeof(struct iphdr)-sizeof(struct udphdr);//������ݳ��ȣ������ȣ�����ͷ����
	struct psdhdr_udp psdudp_header;
	char dataArrary[MAXLENGTH];//������
	char *datap;//ָ������������ָ��
	char dataPac[MAXLENGTH];//���ڸ�������
	char udp_temp[MAXLENGTH+12];//��¼udp��������(����αͷ,��Ҫ����У��)
	//init
	bzero(&dataPac, MAXLENGTH);
	bzero(&dataArrary, MAXLENGTH);
	bzero(&udp_temp,MAXLENGTH+12);
	ip_header = (struct iphdr *)malloc(sizeof(struct iphdr));
	udp_header = (struct udphdr *)malloc(sizeof(struct udphdr));
	//set IPHedaer
	//�˴�����ipheader�����ֶ�ֵ,��Ҫ��ȷ���ֶκ���,���򽫵��·��Ͱ��޷��յ�
	ip_header->ihl=sizeof(struct iphdr)/4;//ipͷ����
	ip_header->version=4;//�汾��
	ip_header->tos=0;
	ip_header->tot_len = packageLength;
	ip_header->id=htons(random()); 
	ip_header->frag_off=0;
	ip_header->ttl=udpConTemp->ipHeader->ttl;//����ʱ��
	ip_header->protocol=IPPROTO_UDP; 
	ip_header->check=0;
	
	if(udpConTemp->srcIpAddressRandom==1) { //���
		if(udpConTemp->srcIpMeth==1) { //���ȷֲ�
			ip_header->saddr=htonl(getRandomNumberFT(udpConTemp->srcip_s,udpConTemp->srcip_e));//���Դip��ַ
		}
		else{
			////�Ǿ��ȷֲ�
		}	
	}
	else { //ָ��Դip��ַ
		ip_header->saddr=inet_addr(udpConTemp->srcIpValue);
	}
	ip_header->daddr = inet_addr(attackipstr);
	//set UDPHedaer
	if(udpConTemp->srcPortRandom==1) { //���
		if(udpConTemp->srcPortMeth==1) { //���ȷֲ�
			udp_header->source=htons(getRandomNumberFT(udpConTemp->srcport_s,udpConTemp->srcport_e));
            
		}
		else{
			////�Ǿ��ȷֲ�
		}
	}
	else { //Դ�˿ں�ָ��
		udp_header->source=htons(udpConTemp->srcPortValue);
	}
	udp_header->dest = htons(atoi(attackportstr));
	//printf("udp_header->dest::%s\n",attackportstr);
	udp_header->len = htons(packageLength-sizeof(struct iphdr));//udp length
	udp_header->check = 0;
	//set udpDheader
	psdudp_header.saddr = ip_header->saddr;
	psdudp_header.daddr = ip_header->daddr;
	psdudp_header.mbz = 0;
	psdudp_header.ptcl = IPPROTO_UDP;
	psdudp_header.udpl = htons(packageLength-sizeof(struct iphdr));
	bcopy((char *)udp_header,(char *)&psdudp_header.udpheader,sizeof(struct udphdr));
	//set data
	int i=0;
	char * getdataTemp;
	int datatemplen;
	if(udpConTemp->dataRandom==1) { //�������
		if(udpConTemp->dataMeth==1) { //���ȷֲ�
			bzero(dataPac,sizeof(dataPac));
			int linklen = getLinkLength_arr(udpConTemp->datals);
			while(i<dataLength){
				getdataTemp=getSomeone_arr(udpConTemp->datals,getRandomNumber(linklen));//����������
				datatemplen = strlen(getdataTemp);
				if(i+datatemplen<dataLength){
					memcpy(dataPac+i,getdataTemp,datatemplen);
					i=i+datatemplen;
				}
				else
					break;
			}
		}
		else{
			////�Ǿ��ȷֲ�
		}
	}
	else{//����ָ��
		memset(dataPac,'1',sizeof(dataPac));
		memcpy(dataPac,udpConTemp->dataValue,strlen(udpConTemp->dataValue));
	}
	//printf("dataPac:%s\n",dataPac);
	memcpy(udp_temp,&psdudp_header,sizeof(psdudp_header));
	memcpy(udp_temp+sizeof(psdudp_header),&dataPac,dataLength);
	/*get udp checksum*/
	if(udpConTemp->udpHeader->check!=0)//����û�û����������ΪcheckΪ0
		udp_header->check = checksum((unsigned short *) &udp_temp,sizeof(psdudp_header)+dataLength);
	else
		udp_header->check =0;
	/*get ip checksum*/
	if(udpConTemp->ipHeader->check!=0)
		ip_header->check=udpConTemp->ipHeader->check;
	else
		ip_header->check=checksum((unsigned short *)&ip_header,sizeof(struct iphdr));
	//�������������
	memcpy(dataArrary,ip_header,sizeof(struct iphdr));
	memcpy(dataArrary+sizeof(struct iphdr),udp_header,sizeof(struct udphdr));
	//free
	if(udp_header)
		free(udp_header);
	if(ip_header)
		free(ip_header);
	if(dataLength>0)
		memcpy(dataArrary+sizeof(struct iphdr)+sizeof(struct udphdr),dataPac,dataLength);
	datap = dataArrary;
	return datap;
}
/********************************************************************************/
/* ��֯ip_icmp��                                                                */
/********************************************************************************/
char * ip_icmp_package(icmpCon * icmpConTemp,char * attackipstr,char * attackportstr)
{
	struct iphdr * ip_header;
	struct icmphdr * icmp_header;
	int packageLength = icmpConTemp->packetLength;
	int dataLength = packageLength-sizeof(struct iphdr)-sizeof(struct icmphdr);//������ݳ��ȣ������ȣ�����ͷ����
	char dataArrary[MAXLENGTH];//������
	char *datap;//ָ������������ָ��
	char icmp_temp[MAXLENGTH];//icmpУ������(icmpͷ+��������)
	char dataPac[MAXLENGTH];
	bzero(&dataPac, MAXLENGTH);
	bzero(&dataArrary, MAXLENGTH);
	bzero(&icmp_temp,MAXLENGTH);
	ip_header = (struct iphdr *)malloc(sizeof(struct iphdr));
	icmp_header = (struct icmphdr *)malloc(sizeof(struct icmphdr));
	//set IPHedaer
	//�˴�����ipheader�����ֶ�ֵ,��Ҫ��ȷ���ֶκ���,���򽫵��·��Ͱ��޷��յ�
	//���IP�ײ�
	ip_header->ihl=sizeof(struct iphdr)/4;//ipͷ����
	ip_header->version=4;//�汾��
	ip_header->tos=0;
	ip_header->tot_len = packageLength;
	ip_header->id=htons(random()); 
	ip_header->frag_off=0;
	ip_header->ttl=icmpConTemp->ipHeader->ttl;//����ʱ�� 
	ip_header->protocol=IPPROTO_ICMP; 
	ip_header->check=0;
	if(icmpConTemp->srcIpAddressRandom==1) { //���
		if(icmpConTemp->srcIpMeth==1) { //���ȷֲ�
			ip_header->saddr=htonl(getRandomNumberFT(icmpConTemp->srcip_s,icmpConTemp->srcip_e));/*���Դip��ַ*/
			//printf("ip_header->saddr::%d\n",ip_header->saddr);
			//printf("[%d]\n",getRandomNumber(getLinkLength_arr(icmpConTemp->srcIpls)));
			
		}
		else{
			//�Ǿ��ȷֲ�
		}
	}
	else { //ָ��Դip��ַ
		ip_header->saddr=inet_addr(icmpConTemp->srcIpValue);
	}
	ip_header->daddr = inet_addr(attackipstr);
	//printf("attackipstr::%s\n",attackipstr);
	//printf("ip_header->daddr::%d\n",ip_header->daddr);
	//set ICMPHedaer
	icmp_header->type = icmpConTemp->icmpHeader->type;
	icmp_header->code = icmpConTemp->icmpHeader->code;
	icmp_header->checksum = icmpConTemp->icmpHeader->checksum;
	//set data
	int i=0;
	char * getdataTemp;
	int datatemplen;
	if(icmpConTemp->dataRandom==1) { //�������
		if(icmpConTemp->dataMeth==1) { //���ȷֲ�
			int linklen=getLinkLength_arr(icmpConTemp->datals);
			bzero(dataPac,sizeof(dataPac));
			while(i<dataLength){
				getdataTemp=getSomeone_arr(icmpConTemp->datals,getRandomNumber(linklen));//����������
				datatemplen = strlen(getdataTemp);
				if(i+datatemplen<dataLength){
					memcpy(dataPac+i,getdataTemp,datatemplen);
					i=i+datatemplen;
				}
				else
					break;
			}
		}
		else{
			////�Ǿ��ȷֲ�
		}
	}
	else { //����ָ��
		memset(dataPac,'1',sizeof(dataPac));
		memcpy(dataPac,icmpConTemp->dataValue,strlen(icmpConTemp->dataValue));
	}
	//printf("dataPac:%s\n",dataPac);
	//get icmp checksum
	if(icmpConTemp->icmpHeader->checksum==0) { //��Ҫ������ȷ��checksum
		memcpy(icmp_temp,icmp_header,sizeof(struct icmphdr));
		memcpy(icmp_temp+sizeof(struct icmphdr),dataPac,dataLength);
		icmp_header->checksum = checksum((unsigned short *)&icmp_temp,packageLength-sizeof(struct iphdr));
	}
	else
		icmp_header->checksum = icmpConTemp->icmpHeader->checksum;
	//get ip checksum
	if(icmpConTemp->ipHeader->check!=0)
		ip_header->check=icmpConTemp->ipHeader->check;
	else
		ip_header->check=checksum((unsigned short *)&ip_header,sizeof(struct iphdr));
	memcpy(dataArrary,ip_header,sizeof(struct iphdr));
	memcpy(dataArrary+sizeof(struct iphdr),icmp_header,sizeof(struct icmphdr));
	memcpy(dataArrary+sizeof(struct iphdr)+sizeof(struct icmphdr),dataPac,dataLength);
	//free
	if(icmp_header)
		free(icmp_header);
	if(ip_header)
		free(ip_header);
	datap = dataArrary;
	return datap;
}
/********************************************************************************/
/* ��֯http��                                                                   */
/********************************************************************************/
char * http_package(httpCon * httpConTemp)
{
	int packageLength = httpConTemp->packetLength;
	int requestLength = strlen(httpConTemp->requestStr);
	char tempArrary[MAXLENGTH];
	char tempArrary2[MAXLENGTH];
	bzero(&tempArrary, sizeof(tempArrary));
	memcpy(tempArrary,httpConTemp->requestStr,strlen(httpConTemp->requestStr));
	int m=0,j=0;
	while(m<requestLength){
		if(tempArrary[m]=='\\'){
			if(tempArrary[m+1]=='r'){
				tempArrary2[j]='\r';
				m=m+2;
				j++;
			}
			else if(tempArrary[m+1]=='n'){
				tempArrary2[j]='\n';
				m=m+2;
				j++;
			}
		}
		else{
			tempArrary2[j]=tempArrary[m];
			j++;
			m++;
		}
	}
	int dataLength = packageLength-j-40;//������ݳ��ȣ������ȣ�����ͷ����
	char dataArrary[MAXLENGTH];//������
	char *datap;//ָ������������ָ��
	char dataPac[MAXLENGTH];
	bzero(&dataPac, sizeof(dataPac));
	bzero(&dataArrary, sizeof(dataArrary));
	memcpy(dataArrary,tempArrary2,j);
	//set data
	int i=0;
	char * getdataTemp;
	int datatemplen;
	if(httpConTemp->dataRandom==1) { //�������
		if(httpConTemp->dataMeth==1) { //���ȷֲ�
			int linklen = getLinkLength_arr(httpConTemp->datals);
			bzero(dataPac,sizeof(dataPac));
			while(i<dataLength){
				getdataTemp=getSomeone_arr(httpConTemp->datals,getRandomNumber(linklen));//����������
				datatemplen = strlen(getdataTemp);
				if(i+datatemplen<dataLength){
					memcpy(dataPac+i,getdataTemp,datatemplen);
					i=i+datatemplen;
				}
				else
					break;
			}
		}
		else{
			//�Ǿ��ȷֲ�
		}
	}
	else { //����ָ��
		memset(dataPac,'1',sizeof(dataPac));
		memcpy(dataPac,httpConTemp->dataValue,strlen(httpConTemp->dataValue));
	}
	memcpy(dataArrary+j,dataPac,dataLength);
	datap = dataArrary;
	return datap;
}
