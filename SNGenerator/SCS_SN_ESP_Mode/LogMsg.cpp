#include "stdafx.h"
#include "logmsg.h"

void CLogMsg::PrintLog(INT LogLevel,const char *fmt,...)
{
	if(!m_PrintEn){
		return;
	}

	int ret=0;
	int offset=0;
	char sprint_buf[LOGMSG_MAXLEN]; 
	va_list args;
	memset(sprint_buf,0,LOGMSG_MAXLEN);
	sprintf(sprint_buf,"[MultiAprog-MSG]");
	offset=(INT)strlen(sprint_buf);

	va_start(args,fmt);  
	vsprintf(sprint_buf+offset,fmt,args); 
	va_end(args); /* ½«argpÖÃÎªNULL */
	if(strlen(sprint_buf)>LOGMSG_MAXLEN-2){
		return;
	}
	strcat(sprint_buf,"\r\n");
	//OutputDebugString(sprint_buf);
}