#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <string.h>

#include "mosquitto_client.h"
#include "mosquitto_parse.h"

/*config*/
#define CLIENT_ID "mosquitto_client_pub"
#define HOST_ADDRESS "192.168.199.244"
#define HOST_PORT 8883
#define KEEPALIVE 60
#define QOS_LEVEL 2

/*cafile  certfile  keyfile*/
#define TLS_VERSION "tlsv1"
#define CA_FILE_PATH "/test/ca.crt"
#define CERT_FILE_PATH "/test/client.crt"
#define KEY_FILE_PATH "/test/client.key"

/*user name & passwd*/
#define USER_NAME "001"
#define USER_PASSWD "001"

/*connect success callback*/
void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
//	int mid = 0;
//	mosquitto_subscribe(mosq, NULL, "qos0/test", 0);
//	mosquitto_message_callback_set(mosq, receive_message_callback);	//receive message callback
//	mosquitto_subscribe(mosq, &mid, "mqttt", QOS_LEVEL);
	printf("connect success 01\n");
}

/*disconnect callback*/
void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
	printf("disconnect 01\n");
}
/*receive message callback*/
void receive_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	printf("\nreceive message callback\n");
//	fwrite(message->payload, 1, message->payloadlen, stdout);

	printf("\n------------------------------------------------------\n");

	parse_msg((CONTEXT *)NULL, message->payload, message->payloadlen);
		
	printf("------------------------------------------------------\n");
}
/*on_log_callback*/
void on_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}


int main(int argc, char *argv[])
{
	int rc;
	struct mosquitto *mosq;
	int mid = 0;

	/*init*/
	mosquitto_lib_init();
	/*config*/
	mosq = mosquitto_new(CLIENT_ID, true, NULL);	//set new client id for mosq
	mosquitto_tls_opts_set(mosq, 1, TLS_VERSION, NULL);	//don`t have to set
	mosquitto_tls_set(mosq, CA_FILE_PATH, NULL, CERT_FILE_PATH, KEY_FILE_PATH, NULL);	//set ca file path
	mosquitto_username_pw_set(mosq, USER_NAME, USER_PASSWD);	//set user name passwd	
	mosquitto_connect_callback_set(mosq, on_connect);	//connect success callback
	mosquitto_disconnect_callback_set(mosq, on_disconnect);		//disconnect callback
	mosquitto_message_callback_set(mosq, receive_message_callback);	//receive message callback
	mosquitto_log_callback_set(mosq, on_log_callback);	//log callback

	/*connect*/
	rc = mosquitto_connect(mosq, HOST_ADDRESS, HOST_PORT, KEEPALIVE);

	/*publish test*/
//	mosquitto_publish(mosq, NULL, "mqtt", strlen("message"), "message", 2, false);

	printf("connect success 02\n");
	mosquitto_subscribe(mosq, &mid, "mqtt", QOS_LEVEL);

	mosquitto_loop_start(mosq);

	printf("connect success 03\n");
	if(rc == 0)
	{

	}
	while(1)	//test sub pub
	{
	//	mosquitto_subscribe(mosq, &mid, "mqtt", QOS_LEVEL);
		char buf[] = "{\"device_id\":\"888888\", \"cmd\":\"send_voice\",\"contact_id\":\"9999999\", \"voice_uri\":\"wwwwwwwwwwww\"}";
		mosquitto_publish(mosq, NULL, "mqtt", strlen(buf), buf, QOS_LEVEL, false);
		
		printf("publish test :%s\n", buf);
	//	mosquitto_loop_forever(mosq, 300, 1);
		sleep(3);
	}

	/*destroy & cleanup*/
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();


	return 0;
}
