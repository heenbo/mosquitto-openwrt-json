#ifndef _MOSQUITTO_PARSE_H_
#define _MOSQUITTO_PARSE_H_

typedef struct _CONTEXT{
	int mode;
	int state;
	void *pdata;	
}CONTEXT;

extern int parse_msg(CONTEXT * ctx, char * buf, unsigned short len);

#endif //_MOSQUITTO_PARSE_H_
