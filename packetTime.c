/********************************************************************************/
/* ���ʱ����                                                                 */
/********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#include "packetTime.h"
/********************************************************************************/
/* packetTime struct return                                                     */
/********************************************************************************/
packetTime * get_packetTime(
	int packetTimeRandom,
	int packetTimeValue,
	int packetTimeScopeFrom,
	int packetTimeScopeTo,
	int packetTimeMeth)
{
	packetTime * packTimeTemp;
	packTimeTemp = (struct packetTime_st *)malloc(sizeof(struct packetTime_st));
	if(packetTimeRandom==2) { //random is false
		packTimeTemp->packetTimeRandom = 2;
		packTimeTemp->packetTimeValue = packetTimeValue;
	}
	else { //random is true
		packTimeTemp->packetTimeRandom = 1;
		packTimeTemp->packetTimeScopeFrom = packetTimeScopeFrom;
		packTimeTemp->packetTimeScopeTo = packetTimeScopeTo;
		packTimeTemp->packetTimeMeth = packetTimeMeth;
	}
	return packTimeTemp;
}
/********************************************************************************/
/* packetTime destroy                                                           */
/********************************************************************************/
void destroy_packetTime(packetTime * packtime)
{
	if(packtime)
		free(packtime);
}
/********************************************************************************/
/* �������                                                                     */
/********************************************************************************/
void outputPacketTime(packetTime * ptTemp)
{
	printf("packetTimeRandom:%d\n",ptTemp->packetTimeRandom);
	if(ptTemp->packetTimeRandom==1){
		printf("packetTimeScopeFrom:%d\n",ptTemp->packetTimeScopeFrom);
		printf("packetTimeScopeTo:%d\n",ptTemp->packetTimeScopeTo);
		printf("packetTimeMeth:%d\n",ptTemp->packetTimeMeth);
	}
	else
		printf("packetTimeValue:%d\n",ptTemp->packetTimeValue);	

	return;
}
/********************************************************************************/
/* ��ָ����Ŀ����n--m�У��о����������һ������                                 */
/********************************************************************************/
unsigned int getRandomNumberFT(unsigned int n,unsigned int m)
{
	unsigned int result;
	//srand((unsigned)time(NULL)+rand());
	//n = rand()%(Y-X+1)+X;
	result = rand()%(m-n+1)+n;
	return result;
}
/********************************************************************************/
/* ����packetTime�õ����ڰ�֮��ļ��                                           */
/********************************************************************************/
int getSleepTime(packetTime * ptTemp)
{
	int timeTemp;//�����ʱ��
	if(ptTemp->packetTimeRandom==1) { //ʱ�����
		if(ptTemp->packetTimeMeth==1) { //���ȷֲ�
			//printf("from:%d,to:%d\n",ptTemp->packetTimeScopeFrom,ptTemp->packetTimeScopeTo);
			timeTemp = getRandomNumberFT(ptTemp->packetTimeScopeFrom,ptTemp->packetTimeScopeTo);
		}
		else { //�Ǿ��ȷֲ�
			//
		}
	}
	else { //ʱ��̶�
		timeTemp=ptTemp->packetTimeValue;
	}
	//printf("from:%d,to:%d\n",ptTemp->packetTimeScopeFrom,ptTemp->packetTimeScopeTo);
	//printf("getTime:%d\n",timeTemp);
	return timeTemp;
}
