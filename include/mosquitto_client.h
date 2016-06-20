#ifndef _MOSQUITTO_CLIENT_H_
#define _MOSQUITTO_CLIENT_H_

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



/*receive message callback*/
extern void receive_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

/*mosquitto pub & sub init*/
extern int msquitto_pub_sub_init(char * mosquitto_device_id);

/*mosquitto_publish_send_msg*/
extern int mosquitto_publish_send_msg(const char * topic, int payloadlen, const void * payload);




#endif //_MOSQUITTO_CLIENT_H_
