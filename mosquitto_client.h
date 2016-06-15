#ifndef _MOSQUITTO_CLIENT_H_
#define  _MOSQUITTO_CLIENT_H_


/*receive message callback*/
extern void receive_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);





#endif //_MOSQUITTO_CLIENT_H_
