#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <json/json.h>
#include <json_object.h>

#include "mosquitto_client.h"
#include "mosquitto_parse.h"
#include "mosquitto_init.h"
#include "mosquitto_i2c.h"

struct mosquitto *mosq = NULL;

char * device_id = "123456789";
/*extern*/
extern int read_xfchat_fifo;
extern struct mpd_connection *conn;

int main(int argc, char *argv[])
{
	int rc = 0;
	/*mosquitto pub & sub init*/
	//input device id
	rc = msquitto_pub_sub_init(device_id);
	if(0 != rc)
	{
		printf("mosquitto pub & sub init");
		return -1;
	}
	/*system Init*/
	Init();


	close (read_xfchat_fifo);

	close_connection(conn);
	return 0;
}

/*connect success callback*/
void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	int mid = 0;
//	mosquitto_subscribe(mosq, NULL, "qos0/test", 0);
//	mosquitto_message_callback_set(mosq, receive_message_callback);	//receive message callback
	mosquitto_subscribe(mosq, &mid, "mqtt", QOS_LEVEL);
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
	printf("\n------------------------------------------------------\n");
	printf("\nreceive message callback: length: %d \n", message->payloadlen);
	fwrite(message->payload, 1, message->payloadlen, stdout);

	parse_msg((CONTEXT *)NULL, message->payload, message->payloadlen);
		
	printf("------------------------------------------------------\n");
}
/*on_log_callback*/
void on_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

/*mosquitto pub & sub init*/
int msquitto_pub_sub_init(char * mosquitto_device_id)
{
	int rc;
//	int mid = 0;

	/*init*/
	mosquitto_lib_init();
	/*config*/
	mosq = mosquitto_new(mosquitto_device_id, true, NULL);	//set new client id for mosq
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

	printf("connect success 02\n");
//	mosquitto_subscribe(mosq, &mid, "mqtt", QOS_LEVEL);

	mosquitto_loop_start(mosq);

	printf("connect success 03\n");
	if(rc == 0)
	{

	}
	while(1)	//test sub pub
	{
		printf("publish send test -------------------------------------------------------------\n");
	//	mosquitto_subscribe(mosq, &mid, "mqtt", QOS_LEVEL);
	//	char buf[] = "{\"device_id\":\"888888\", \"cmd\":\"send_voice\",\"contact_id\":\"9999999\", \"voice_uri\":\"wwwwwwwwwwww\"}";
	//	mosquitto_publish(mosq, NULL, "mqtt", strlen(buf), buf, QOS_LEVEL, false);
		json_object *my_object = NULL;
		my_object = json_object_new_object();
		json_object_object_add(my_object, "device_id",	json_object_new_string(mosquitto_device_id));
		json_object_object_add(my_object, "cmd",	json_object_new_string("send_voice"));
		json_object_object_add(my_object, "contact_id",	json_object_new_string("32323232"));
		json_object_object_add(my_object, "voice_uri",	json_object_new_string("www.www.www"));
		
		printf("my_object_string: %s, length: %d \n", json_object_to_json_string(my_object), strlen(json_object_to_json_string(my_object)));

		mosquitto_publish(mosq, NULL, "mqtt", strlen(json_object_to_json_string(my_object)), json_object_to_json_string(my_object), QOS_LEVEL, false);
		
		json_object_put(my_object);
		
		printf("publish send test -------------------------------------------------------------\n");
		sleep(3);
	}

	/*destroy & cleanup*/
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}

/*mosquitto_publish_send_msg*/
int mosquitto_publish_send_msg(const char * topic, int payloadlen, const void * payload)
{
	mosquitto_publish(mosq, NULL, topic, payloadlen, payload, QOS_LEVEL, false);
	return 0;
}
