#ifndef _PACKAGE_H_
#define _PACKAGE_H_

/*TCPDHeader*/
struct psdhdr_tcp
{
        unsigned int saddr;/*32,IP from address*/
        unsigned int daddr;/*32,ip to address*/       
	unsigned char mbz;/*set empty*/
        unsigned char ptcl;/*protcol style*/
        unsigned short tcpl;/*TCP length*/ 
	struct tcphdr tcpheader;
};
//struct psdhdr_tcp psdtcp_header;
/*UDPDHeader*/
struct psdhdr_udp
{
	unsigned int saddr;/*32,IP from address*/
        unsigned int daddr;/*32,ip to address*/
        unsigned char mbz;/*set empty*/
        unsigned char ptcl;/*protcol style*/
    	unsigned short udpl;/*UDP length*/ 
    	struct udphdr udpheader;
};

//function:��֯ip_tcp���ݰ�
//input:ip_tcp���ã�����ip�������˿�
//output:ip_tcp���ݰ���ַ
char * ip_tcp_package(synCon * synConTemp,char * attackipstr,char * attackportstr);

//function:��֯ip_udp���ݰ�
//input:ip_udp���ã�����ip�������˿�
//output:ip_udp���ݰ���ַ
char * ip_udp_package(udpCon * udpConTemp,char * attackipstr,char * attackportstr);

//function:��֯ip_icmp���ݰ�
//input:ip_icmp���ã�����ip�������˿�
//output:ip_icmp���ݰ���ַ
char * ip_icmp_package(icmpCon * icmpConTemp,char * attackipstr,char * attackportstr);

//function:��֯http���ݰ�
//input:http���ã�����ip�������˿�
//output:http���ݰ���ַ
char * http_package(httpCon * httpConTemp);

#endif
