#ifndef _PACKETTIME_H_
#define _PACKETTIME_H_

struct packetTime_st 
{
	int packetTimeRandom;
	int packetTimeValue;
	int packetTimeScopeFrom;
	int packetTimeScopeTo;
	int packetTimeMeth;
};

typedef struct packetTime_st packetTime;

/********************************************************************************/
/* д������õ��ṹ��packetTime_st                                              */
/* �������:�������                                                            */
/* ���:������ýṹ��packetTime_stͷָ��                                       */
/********************************************************************************/
packetTime * get_packetTime(
	int packetTimeRandom,
	int packetTimeValue,
	int packetTimeScopeFrom,
	int packetTimeScopeTo,
	int packetTimeMeth);
/********************************************************************************/
/* ����������                                                                 */
/* �������:������ýṹ��packetTime_st                                         */
/********************************************************************************/
void outputPacketTime(packetTime * ptTemp);
/********************************************************************************/
/* �õ����ʱ��                                                                 */
/* �������:������ýṹ��packetTime_st                                         */
/* ���:���ʱ��                                                                */
/********************************************************************************/
int getSleepTime(packetTime * ptTemp);
/********************************************************************************/
/* �ͷż��ʱ��                                                                 */
/* ����:������ýṹ��packetTime_st                                             */
/********************************************************************************/
void destroy_packetTime(packetTime * packtime);

/********************************************************************************/
/* ��ָ����Ŀ����n--m�У��о����������һ������                                 */
/********************************************************************************/
unsigned int getRandomNumberFT(unsigned int n,unsigned int m);

#endif
