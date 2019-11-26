/**************************************************************************/
/* �������ļ��ж�ȡ����������Ϣ                                           */
/* �����ļ�����                                                         */
/*              ÿ�н�һ������ֵ                                          */
/*              �����ַ�Ϊ # �ŵ���Ϊע����                               */
/*              ���һ���в����� = �� ��Ĭ�ϸ���Ϊע����                  */
/*              �������������ֵ�� = ������                               */
/*              ���������в��ܰ��� = ��                                   */
/*               = ��ǰ�����пո�                                       */
/*              ÿ������ַ���Ϊ LINE_MAX_LENGTH                          */
/*              ����������ַ���Ϊ NAME_MAX_LENGTH,�����������ò���       */
/*              ����ֵ����ַ���Ϊ VALUE_MAX_LENGTH,�������ض�            */
/**************************************************************************/
#include "getconfigfromtext.h"
/***************************************************************************/
/* �������ļ��ж�ȡ����һ��������Ӧ��ֵ                                    */
/* ������1,�����ļ�·��; 2,ƥ������; 3,����洢�ռ�                        */
/* ���أ�0,δ�ҵ�; 1,�ҵ��������Ƶ�ֵ                                      */
/***************************************************************************/
int GetConfigValue(char *conf_path,char *conf_name,char *config_buff)
{
    char config_linebuf[LINE_MAX_LENGTH];
    char line_name[NAME_MAX_LENGTH];
    char exchange_buf[LINE_MAX_LENGTH];
    char *config_sign = "=";
    char *leave_line;
    FILE *f;
	int flag=0,len;
    f = fopen(conf_path,"r");
    if(f == NULL){
        printf("OPEN CONFIG FALID\n");
        return 0;
    }
    fseek(f,0,SEEK_SET);
    while( fgets(config_linebuf,LINE_MAX_LENGTH,f) != NULL ){
        if(strlen(config_linebuf) < 4) //ȥ������ "=\r\n"
            continue;
		if(config_linebuf[0]=='#')//ȥ��ע���� "#"
			continue;
        if (config_linebuf[strlen(config_linebuf)-1] == 10){ //ȥ�����һλ��\n�����
            memset(exchange_buf,0,sizeof(exchange_buf));
            strncpy(exchange_buf,config_linebuf,strlen(config_linebuf)-1);
            memset(config_linebuf,0,sizeof(config_linebuf));
            strcpy(config_linebuf,exchange_buf);
        }
        memset(line_name,0,sizeof(line_name));
        leave_line = strstr(config_linebuf,config_sign);
        if(leave_line == NULL)//ȥ����"="�����
            continue;
        int leave_num = leave_line-config_linebuf;
		if(leave_num>NAME_MAX_LENGTH)
			continue;
        strncpy(line_name,config_linebuf,leave_num);
        if(strcmp(line_name,conf_name) ==0){
			len = strlen(config_linebuf)-leave_num-1;
			len = len>VALUE_MAX_LENGTH?VALUE_MAX_LENGTH:len;
            strncpy(config_buff,config_linebuf+(leave_num+1),strlen(config_linebuf)-leave_num-1);
			*(config_buff+len) = '\0';
			flag = 1;
            break;
        }
        if(fgetc(f)==EOF)
            break;
        fseek(f,-1,SEEK_CUR);
        memset(config_linebuf,0,sizeof(config_linebuf));
    }
    fclose(f);
	return flag;
}
/***************************************************************************/
/* �������ļ������ζ�ȡ���в�����Ӧ��ֵ                                    */
/* ������1,�����ļ�·��; 2,����洢�ռ�; 3,��������                        */
/* ���أ�0,�д���(���ļ�����;���ļ���ȡ�Ĳ�������С���趨Ҫ��ȡ�Ĳ�������);*/
/*       1,��ȡ���óɹ�                                                    */
/***************************************************************************/
int GetAllConfig(char *conf_path,char *config_buff,int param_num)
{
	char config_linebuf[LINE_MAX_LENGTH];
    //char exchange_buf[LINE_MAX_LENGTH];
    char *config_sign = "=";
    char *leave_line;
    FILE *f;
	int param_i=0,len;
    f = fopen(conf_path,"r");
    if(f == NULL){
        printf("OPEN CONFIG FALID\n");
        return 0;
    }
    fseek(f,0,SEEK_SET);
    while( fgets(config_linebuf,LINE_MAX_LENGTH,f)!=NULL && param_i<param_num ){
        if(strlen(config_linebuf) < 4) //ȥ������ "=\r\n"
            continue;
		if(config_linebuf[0]=='#')//ȥ��ע���� "#"
			continue;
        if (config_linebuf[strlen(config_linebuf)-1] == 10){ //ȥ�����һλ��\n
            //memset(exchange_buf,0,sizeof(exchange_buf));
            //strncpy(exchange_buf,config_linebuf,strlen(config_linebuf)-1);
            //memset(config_linebuf,0,sizeof(config_linebuf));
            //strcpy(config_linebuf,exchange_buf);
			config_linebuf[strlen(config_linebuf)-1]='\0';
        }
		leave_line = strstr(config_linebuf,config_sign);
		if(leave_line == NULL)//ȥ����"="�����
            continue;
        int leave_num = leave_line-config_linebuf;
		if(leave_num>NAME_MAX_LENGTH)
			continue;
		len = strlen(config_linebuf)-leave_num-1;
		len = len>VALUE_MAX_LENGTH?VALUE_MAX_LENGTH:len;
        strncpy(config_buff+param_i*VALUE_MAX_LENGTH,leave_line+1,len);
		*(config_buff+param_i*VALUE_MAX_LENGTH+len) = '\0';
		param_i++;
        if(fgetc(f)==EOF)
            break;
        fseek(f,-1,SEEK_CUR);
        memset(config_linebuf,0,sizeof(config_linebuf));
    }
	fclose(f);
	if(param_i!=param_num)
		return 0;
	return 1;
}
