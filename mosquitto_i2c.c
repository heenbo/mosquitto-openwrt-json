#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <json/json.h>
#include <json_object.h>
#include <connection.h>
#include <playlist.h>
#include <player.h>
#include <status.h>
#include <response.h>
#include <queue.h>
#include <database.h>
#include <mosquitto.h>
#include "mosquitto_i2c.h"
#include "mosquitto_mpc.h"
#include "mosquitto_client.h"

int i2c_file = -1;
bool bread_register = false;

int ncurrt_time = 0;
bool bRecord = false;
int nmpc_list = -1;
char *playlists[] = {"eg.lst", "gs.lst", "gx.lst", "yy.lst" , "yyqm.lst" , "teyy.lst", "gdyy.lst", "download.lst"};

bool bplay_audio = false;
/*sleep*/
bool bvoice_vod = false;

/********/
int xfchat_fifo = -1;
int read_xfchat_fifo = -1;
pthread_mutex_t mutex_sock = PTHREAD_MUTEX_INITIALIZER;

/*static*/
/*btn5_click*/
static void btn5_click(CONTEXT *ctx);
/*btn6_click*/
static void btn6_click(CONTEXT *ctx);
/*btn7_click*/
static void btn7_click(CONTEXT *ctx);
/*btn8_click*/
static void btn8_click(CONTEXT *ctx);

static KEY_DESC keys[MAX_BTNS] = {
	{{{0, 0}, 0, BTN_5, 0}, {btn5_click }, 0, 0, KEY_IDLE},
	{{{0, 0}, 0, BTN_6, 0}, {btn6_click }, 0, 0, KEY_IDLE},
	{{{0, 0}, 0, BTN_7, 0}, {btn7_click }, 0, 0, KEY_IDLE},
	{{{0, 0}, 0, BTN_8, 0}, {btn8_click }, 0, 0, KEY_IDLE},
};
#if 0
static KEY_DESC keys[MAX_BTNS] = {
	{{keys[0].ev.code = BTN_5,}, {btn5_click }, 0, 0, KEY_IDLE},
	{{keys[1].ev.code = BTN_6,}, {btn6_click }, 0, 0, KEY_IDLE},
	{{keys[2].ev.code = BTN_7,}, {btn7_click }, 0, 0, KEY_IDLE},
	{{keys[3].ev.code = BTN_8,}, {btn8_click }, 0, 0, KEY_IDLE},
};
#endif

/*extern */
extern pthread_mutex_t mutex;
extern struct mpd_connection *conn;
extern char * device_id;
extern char *ch_music_url;

/*parse_cmdstr*/
static int parse_cmdstr(char * buf, int len);
/*do_send_voice*/
static int do_send_voice(const char * did, const char * uri, const char * contact);
/*get_mpc_quere_len*/
static int get_mpc_quere_len(struct mpd_connection *conn);



/*Init I2CDev*/
int InitI2CDev(void)
{
	char filename[20];
	i2c_file = open(filename, O_RDWR);

	if (i2c_file < 0 && errno == ENOENT)
	{
		sprintf(filename, "/dev/i2c-%d", 0);
		i2c_file = open(filename, O_RDWR);
	}
	
	if (i2c_file < 0)
	{
		printf("open error\n");
		return -1;
	}
		
	if (ioctl(i2c_file, I2C_SLAVE, 0x27) < 0) {
		fprintf(stderr,"Error: Could not set address to : %s\n", strerror(errno));
		return -errno;
	}
	
	return 0;
}


/*Close Light*/
void CloseLight(void)
{
	int ret = 0;
	ret = i2c_smbus_write_byte_data(i2c_file, 0x00, 0x00);
	if(ret < 0)
	{
		printf("nd CloseLight i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", ret);
	}	
}


/*The thread of I2C*/
void * I2CThread(void *arg) 
{ 
	int res = 0;
	
	while (1) 
	{  
		res = 0;
		if(i2c_file > 0)
		{
			if(!bread_register)
			{
				res = i2c_smbus_read_word_data(i2c_file, REGISTER3);
				switch (res) 
				{
					case SWITCH_BTN_SHUT_DWON:
					{
						printf("nd I2CThread SWITCH_BTN_SHUT_DWON: 0x%0*x\n", 2, res);		
						break;
					}
					case SHUT_DWON:
					{
						printf("nd I2CThread BTN3 SHUT_DWON: 0x%0*x\n", 2, res);		
						break;
					}
					case AUTO_SLEEP:
					{
						printf("nd I2CThread AUTO_SLEEP: 0x%0*x\n", 2, res);		
						break;
					}
				}
				
				bread_register = true;
			}
			
			res = i2c_smbus_read_word_data(i2c_file, REGISTER2);
			//printf("md I2CThread i2c_file res: 0x%0*x\n", 2, res);
			switch (res) 
			{
				case BTN1_SHORT_PRESS:
				{
					printf("nd I2CThread BTN1_SHORT_PRESS: 0x%0*x\n", 2, res);		
					btn1_short_press();
					break;
				}
				case BTN1_LONG_PRESS:
				{
					printf("nd I2CThread BTN1_LONG_PRESS: 0x%0*x\n", 2, res);
					btn1_long_press();
					break;
				}
				case BTN1_AUDIO_SPPEK_ARECORD:
				{	
					btn1_audio_sppek_arecord(res);
					break;
				}
				case BTN2_SHORT_PRESS:
				{
					printf("nd I2CThread BTN2_SHORT_PRESS: 0x%0*x\n", 2, res);
					btn2_short_press();
					break;
				}
				case BTN2_LONG_PRESS:
				{
					printf("nd I2CThread BTN2_LONG_PRESS: 0x%0*x\n", 2, res);
					btn2_long_press();
					break;
				}
				case BTN3_SHORT_PRESS:
				{
					printf("nd I2CThread BTN3_SHORT_PRESS: 0x%0*x\n", 2, res);
				  btn3_short_press();
					break;
				}
				case BTN3_LONG_PRESS:
				{
					printf("nd I2CThread BTN3_LONG_PRESS: 0x%0*x\n", 2, res);
					btn3_long_press();
					break;
				}
				case BTN4_SHORT_PRESS:
				{
					printf("nd I2CThread BTN4_SHORT_PRESS: 0x%0*x\n", 2, res);
					btn4_short_press();
					break;
				}
				case BTN4_LONG_PRESS:
				{
					printf("nd I2CThread BTN4_LONG_PRESS: 0x%0*x\n", 2, res); 
					btn4_long_press();
					break;
				}			
				case BTN5_SHORT_PRESS:
				{
					printf("nd I2CThread BTN5_SHORT_PRESS: 0x%0*x\n", 2, res);
					btn5_short_press();
					break;
				}
				case BTN5_LONG_PRESS:
				{
					printf("nd I2CThread BTN5_LONG_PRESS: 0x%0*x\n", 2, res);
					btn5_long_press();
					break;	
				}
				case BTN5_QUESTION_ANSWER_ARECORD:
				{
					btn5_question_answer_arecord(res);
					break;	
				}

				case PEN_START:
				{
#ifdef PEN_SUPPORT					
					printf("nd I2CThread PEN_START: 0x%0*x\n", 2, res);
					int nValue = 0, nValue8 = 0, nValue9 = 0, nValue10 = 0, nValue89 = 0;

					nValue10 = i2c_smbus_read_word_data(i2c_file, REGISTER10);
					printf("nd I2CThread REGISTER10: 0x%0*x, %d\n", 2, nValue10, nValue10);
					
					nValue9 = i2c_smbus_read_word_data(i2c_file, REGISTER9);
					printf("nd I2CThread REGISTER9: 0x%0*x, %d\n", 2, nValue9, nValue9);
					
					nValue8 = i2c_smbus_read_word_data(i2c_file, REGISTER8);
					printf("nd I2CThread REGISTER8: 0x%0*x, %d\n", 2, nValue8, nValue8);
					
					nValue89 = nValue8 | ((nValue9 << 8) & 0xff00);
					printf("nd I2CThread nValue89: %x, nValue89: %d, \n", nValue89, nValue89);
					
					nValue = nValue8 | ((nValue9 << 8) & 0xff00) | ((nValue10 << 16) & 0xff0000);
					printf("nd I2CThread nValue: %x, \n", nValue, nValue);
				
					if(nValue == 0x60fff8)
					{
						pthread_mutex_lock(&mutex);
						mpd_run_pause(conn, true);
						mpd_run_clear(conn);
						mpd_run_load(conn, "ljyy.lst");
						pthread_mutex_unlock(&mutex);
					}
					else if(nValue == 0x60fff7)
					{
						
					}
					else if (nValue89 > 52900)
					{
						int nPos = nValue89 - 52901;
						printf("nd I2CThread nPos: %d, \n", nPos);
						pthread_mutex_lock(&mutex);
						mpd_run_pause(conn, true);
						mpd_run_single(conn, true);
						mpd_run_play_pos(conn, nPos);
						pthread_mutex_unlock(&mutex);
					}
				
					res = i2c_smbus_read_word_data(i2c_file, REGISTER2);
					printf("nd I2CThread REGISTER2: 0x%0*x\n", 2, res);
#endif					
					
					break;	
				}
				case NOT_BTN_EVENT:
				{
					break;	
				}
			}
		}
		else
		{
			printf("nd I2CThread i2c_file: %d\n", i2c_file);
			InitI2CDev();
		}

		usleep(50*1000);	
	}   
}

/*btn1_short_press*/
void btn1_short_press(void)
{	
	int res = 0;
	ncurrt_time = 0;
	bRecord = false;
	system("killall arecord");
	res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
	if(res < 0)
	{
		printf("nd btn1_short_press i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
	}	
}

/*btn1_long_press*/
void btn1_long_press(void)
{
	int res = 0;
	ncurrt_time = 0;
	bRecord = false;
	system("killall arecord");
	res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
	if(res < 0)
	{
		printf("nd BTN1_LONG_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
	}
	json_object * msg_object = NULL;
	msg_object = json_object_new_object();
	json_object_object_add(msg_object, "cmd", json_object_new_string("send_voice"));
	json_object_object_add(msg_object, "contact_id", json_object_new_string("surfL"));
	
//	char *cmd_str = "{\"cmd\":\"send_voice\", \"contact_id\":\"surfL\"}";
//	parse_cmdstr(&ctx, cmd_str, strlen(cmd_str));
	parse_cmdstr((char *)json_object_to_json_string(msg_object), strlen(json_object_to_json_string(msg_object)));
	json_object_put(msg_object);
}


/*parse_cmdstr*/
static int parse_cmdstr(char * buf, int len)
{
	json_object  *json = NULL, *cmd = NULL, *did = NULL;
	enum json_tokener_error error = json_tokener_success;
	json = json_tokener_parse_verbose(buf, &error);
	char * str = NULL;
	//char * did_str = NULL;
	
	printf("parse_cmdstr Debug:parse fifo json string len:%d, :%s\n", len, buf);
	
	if(!json)
	{
		printf("parse_cmdstr Error, parse fifo json token :%s failed\n", buf);
		return -1;	
	}

	if(json_object_object_get_ex(json, "cmd", &cmd) == TRUE)
	{
		if(json_object_get_type(cmd) == json_type_string)
		{
			str = (char *)json_object_get_string(cmd);
			json_object_object_get_ex(json, "device_id", &did);
			if(did)
			{
				//did_str = (char *)json_object_get_string(did);
				json_object_get_string(did);
			}
			if(!strcmp(str, "send_voice"))
			{
				json_object *contact_id = NULL;
				char *contact_str = NULL;
				system("upvoice.sh");
				json_object_object_get_ex(json, "contact_id", &contact_id);
				if (contact_id)
				{
					contact_str = (char *)json_object_get_string(contact_id);
				}
				send_voice(contact_str);	
			}
			
			if(!strcmp(str, "xfchat_music"))
			{
				json_object *js_music_name = NULL, *js_music_url = NULL, *js_rc_type = NULL;
				char *ch_music_name = NULL;
				char *ch_rc_type = NULL;

				json_object_object_get_ex(json, "rc_type", &js_rc_type);
				if(js_rc_type)
				{
					ch_rc_type = (char *)json_object_get_string(js_rc_type);
				}
				json_object_object_get_ex(json, "name", &js_music_name);
				if(js_music_name)
				{
					ch_music_name = (char *)json_object_get_string(js_music_name);
				}	
				json_object_object_get_ex(json, "url", &js_music_url);
				if(js_music_url)
				{
					ch_music_url = (char *)json_object_get_string(js_music_url);
				}
				printf("ch_music_name: %s, ch_music_url: %s, ch_rc_type: %s\n", ch_music_name, ch_music_url, ch_rc_type);
				check_xfchat_music(ch_music_name, ch_rc_type);
				
			}
			
			if(!strcmp(str, "stop"))
			{
				system("/usr/bin/ais_led blue off");
			}		
		}
	}
	return 0;
}

/*send_voice*/
void send_voice(const char * contact)
{
	char uri[4096] = {0};
	FILE *fp = NULL;
	//size_t nmber;
	fp = fopen("/tmp/vouri.txt", "r");
	if(!fp)
	{
		perror("open vouri.txt ");
		return;
	}
	fread((void *)&uri[0], 4096, 1, fp);
	fclose(fp);
	
	uri[strlen(uri) - 1] = '\0';
	printf("send voice uri:%s\n", uri);
	do_send_voice(device_id, uri, contact);	
	
	return;

}

/*do_send_voice*/
static int do_send_voice(const char * did, const char * uri, const char * contact)
{
//	char json_str[4096]={0};
//	char str[4096] = {0};
//	int length = 0;

	json_object * msg_object = NULL;
	msg_object = json_object_new_object();
	json_object_object_add(msg_object, "cmd", json_object_new_string("send_voice"));
	json_object_object_add(msg_object, "device_id", json_object_new_string(did));
	json_object_object_add(msg_object, "contact_id", json_object_new_string(contact));
	json_object_object_add(msg_object, "uri", json_object_new_string(uri));

	mosquitto_publish_send_msg("mqtt", strlen(json_object_to_json_string(msg_object)), json_object_to_json_string(msg_object));

	json_object_put(msg_object);
#if 0	

	((struct msg_header*)json_str)->cmd = htons(0x0101);
	snprintf(&str[0], 4096, "{\"cmd\":\"send_voice\", \"device_id\":\"%s\", \"contact_id\":\"%s\", \"voice_uri\":\"%s\" }",
			 	did, contact,  uri);
	length = strlen(str);
	((struct msg_header*)json_str)->len  = htons(length);
	memcpy(&json_str[0] + sizeof(struct msg_header), &str[0], length);
	
	printf("send voice json string:%s\n", str);

	if(sfd){
		if (sck_write(sfd, &json_str[0], sizeof(struct msg_header) + length) < 0) {
			perror("send message");
			return -1;
		}
	} else
		printf("Error, sockfd null\n");
#endif
	return 0;

}

/*btn1_audio_sppek_arecord*/
void btn1_audio_sppek_arecord(int ret)
{
	ncurrt_time = 0;
	
	if(!bRecord)
	{
		int res = ret;
		printf("nd BTN1_AUDIO_SPPEK_ARECORD: 0x%0*x\n", 2, res);
		my_mpd_run_pause();
		system("killall aplay");
		system("killall xfchat");
		system("arecord -f S16_LE -r 8000 -c 2 /tmp/upvoice.wav &");
		res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_ON);
		if(res < 0)
		{
			printf("nd BTN1_AUDIO_SPPEK_ARECORD i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", res);
		}
	
		bRecord = true;
	}
}
/*btn2_short_press*/
void btn2_short_press(void)
{
	ncurrt_time = 0;
	pthread_mutex_lock(&mutex);
	mpd_run_previous(conn);
	pthread_mutex_unlock(&mutex);
}

/*btn2_long_press*/
void btn2_long_press(void)
{
	ncurrt_time = 0;
	system("killall aplay");
	system("killall xfchat");
	
	if(nmpc_list <= 0)
	{
		nmpc_list = 7;
	}
	else
	{
		nmpc_list--;
	}
	char* p = playlists[nmpc_list];
	
	pthread_mutex_lock(&mutex);
	mpd_run_clear(conn);
	//printf("nd btn4_long_press nmpc_list: %d, p1: %s\n", nmpc_list, p);
	mpd_run_load(conn, p);
	mpd_run_play(conn);
	pthread_mutex_unlock(&mutex);
}

/*btn3_short_press*/
void btn3_short_press(void)
{
	int res = 0;
#if 0
	XmlData xmldt;
#endif
	struct mpd_status *status = NULL;
	
	ncurrt_time = 0;
	system("killall aplay");
	system("killall xfchat");
#if 0	
	res = GetXmlData(PLAY_VOICE_XML_PATH, PLAY_VOICE_XML_NODE_NAME, xmldt);
#endif
	if((-1 != res && !bplay_audio))
	{
		printf("nd btn3_short_press ret: %d, bplay_audio: %d\n", res, bplay_audio);
		my_mpd_run_pause();
		bplay_audio = true;
	}	
	else
	{
		int quere_len = 0;
		int online_size = 0;
		bplay_audio = false;
		
		online_size = get_file_size(ONLINE_LIST_PATH);
		if(online_size > 0)
		{
			quere_len = get_mpc_quere_len(conn);
			
			pthread_mutex_lock(&mutex);
			if(quere_len > 0)
			{
				mpd_run_stop(conn);
				mpd_run_clear(conn);
			}
			
			bool mpc_load = mpd_run_load(conn, "online.lst");
			bool mpc_list_clear = mpd_run_playlist_clear(conn, "online.lst");
			printf("nd btn3_short_press mpc_load: %d, mpc_list_clear: %d\n", mpc_load, mpc_list_clear);
			pthread_mutex_unlock(&mutex);
			
			res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
			if(res < 0)
			{
				printf("nd btn3_short_press i2c_smbus_write_byte_data BLUE_OFF failed, ret: %d\n", res);
			}
		}
			
		quere_len = get_mpc_quere_len(conn);	
		if(quere_len > 0)
		{	
			printf("nd btn3_short_press get_mpc_quere_len quere_len: %d\n", quere_len);
			pthread_mutex_lock(&mutex);
			status = mpd_run_status(conn);
			if (!status) 
			{
				printf("nd btn3_short_press mpd_run_status2 %s \n", mpd_connection_get_error_message(conn));
			}
			else
			{
				if (mpd_status_get_state(status) == MPD_STATE_PLAY)
				{
					printf("nd btn3_short_press mpd_status_get_state MPD_STATE_PLAY\n");
					mpd_run_pause(conn, true);
				}
				else 
				{
					printf("nd btn3_short_press mpd_status_get_state other state\n");
					mpd_run_play(conn);
				}
			
				mpd_status_free(status);
			}
			pthread_mutex_unlock(&mutex);
		}
  }
}

/*btn3_long_press*/
void btn3_long_press(void)
{
	ncurrt_time = 0;	
}

/*btn4_short_press*/
void btn4_short_press(void)
{
	ncurrt_time = 0;
	pthread_mutex_lock(&mutex);
	mpd_run_next(conn);
	pthread_mutex_unlock(&mutex);
}

/*btn4_long_press*/
void btn4_long_press(void)
{
	ncurrt_time = 0;
	system("killall aplay");
	system("killall xfchat");
	
	if(nmpc_list >= 7)
	{
		nmpc_list = 0;
	}
	else
	{
		nmpc_list++;
	}
	char* p = playlists[nmpc_list];
	
	pthread_mutex_lock(&mutex);
	mpd_run_clear(conn);
	mpd_run_load(conn, p);
	mpd_run_play(conn);
	pthread_mutex_unlock(&mutex);
}

/*btn5_short_press*/
void btn5_short_press(void)
{
	int res = 0;
	
	ncurrt_time = 0;
	bRecord = false;
	res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
	if(res < 0)
	{
		printf("nd BTN5_SHORT_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
	}
	sck_write_xfchat_fifo();
}

/*btn5_long_press*/
void btn5_long_press(void)
{
	int res = 0;
	
	ncurrt_time = 0;
	bRecord = false;
 	res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
 	if(res < 0)
	{
		printf("nd BTN5_LONG_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
	}
 	sck_write_xfchat_fifo();
}

/*btn5_question_answer_arecord*/
void btn5_question_answer_arecord(int ret)
{
	ncurrt_time = 0;

	if(!bRecord)
	{
		int res = ret;
		printf("nd BTN5_QUESTION_ANSWER_ARECORD: 0x%0*x\n", 2, res);
	
		char buf[1024] = {0};							
		if(read(read_xfchat_fifo, buf, 1024) < 0)
		{
			perror("nd btn5_question_answer_arecord read erro: ");
		}
		printf("nd btn5_question_answer_arecord read_xfchat_fifo read buf: %s\n", buf);
		
		my_mpd_run_pause();
		system("killall xfchat");
		system("killall aplay");
		system("xfchat &");
		
		usleep(1000*150);
		res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_ON);
		if(res < 0)
		{
			printf("nd BTN5_QUESTION_ANSWER_ARECORD i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", res);
		}
		printf("nd BTN5_QUESTION_ANSWER_ARECORD2: 0x%0*x\n", 2, res);
		bRecord = true;
	}
}

/*check_xfchat_music*/
int check_xfchat_music(const char *chMusicName, const char *ch_rc_type)
{
	char json_str[4096]={0};
	char str[4096] = {0};
	int length = 0;

	snprintf(&str[0], 4096, "{\"cmd\":\"xfchatmusic\", \"rc_type\":\"%s\", \"name\":\"%s\" }", ch_rc_type, chMusicName);
	length=strlen(str);
	memcpy(&json_str[0], &str[0], length);
#if 0 // publish
	if (sfd){
		if (sck_write(sfd, &json_str[0], sizeof(struct msg_header) + length) < 0) {
			perror("check_xfchat_music send message");
			return -1;
		}
	} else
		printf("check_xfchat_music Error, sockfd null\n");
#endif
	return 0;
}

/*get_file_size*/
int get_file_size(const char *file_name)
{
	FILE * pFile;
	long size = 0;

   	pFile = fopen(file_name,"rb");
   	if(pFile == NULL)
  	{
		perror("nd get_file_size Error opening file");
   	}
    	else
	{
		fseek(pFile, 0, SEEK_END);
		size=ftell (pFile);
		fclose(pFile);
        //printf ("Size of file_name: %s size: %ld bytes.\n", file_name, size);
    	}
   	return size;
}

/*get_mpc_quere_len*/
static int get_mpc_quere_len(struct mpd_connection *conn)
{
	int quere_len = 0;
	pthread_mutex_lock(&mutex);
	struct mpd_status *status;
	status = mpd_run_status(conn);
	if(!status)
	{
		printf("nd get_mpc_quere_len erro %s\n", mpd_connection_get_error_message(conn));
		return -1;
	}

	quere_len = mpd_status_get_queue_length(status);

	mpd_status_free(status);
	mpd_response_finish(conn);
	//CHECK_CONNECTION(conn);
	pthread_mutex_unlock(&mutex);
	
	return quere_len;
}

/*sck_write_xfchat_fifo*/
void sck_write_xfchat_fifo(void)
{
	int ret = 0;
	char ch_xfchat[1024] = {0};
	snprintf(&ch_xfchat[0], 1024, "{\"cmd\":\"xfchat\", \"arecord\":\"stop\"}");
	printf("arecord stop2!\n");
	ret = sck_write_fifo(xfchat_fifo, ch_xfchat, 1024);
	if(ret < 0)
	{
		perror("nd sck_write_xfchat_fifo sck_write fifo: ");
	}	
	printf("nd sck_write_xfchat_fifo arecord stop1! ch_xfchat: %s\n", ch_xfchat);
}

/*sck_write*/
ssize_t sck_write_fifo(int fd, const void * buf, size_t count)
{
	ssize_t ret = 0;
	printf("mysck_write------------\n");
	pthread_mutex_lock(&mutex_sock);
	ret = write(fd, buf, count);
	pthread_mutex_unlock(&mutex_sock);
	
	return ret;
}

/*The thread of button*/
void* BtnThread(void *arg)  
{ 
	int i = 0, ret = 0;
	bool bBtn5Press = false;

	while(true)
	{
		struct timeval current_time;
		gettimeofday(&current_time, NULL);
		
		for(i = 0; i < MAX_BTNS; i++)
		{
			if(0 == i)     
			{
				if (keys[i].status == KEY_PRESS)
				{
					if(!bBtn5Press)
					{
						printf("nd BtnThread btn5 KEY_PRESS\n");
						ncurrt_time = 0;
						bBtn5Press = true;
						mpd_run_pause(conn, true);
						system("killall aplay");
						system("killall xfchat");
					}
					
					if((current_time.tv_sec *1000*1000 + current_time.tv_usec - keys[i].press_time)/1000 > 8000)
					{
						ret = i2c_smbus_write_byte_data(i2c_file, 0x00, RED_ON);
						if(ret < 0)
						{
							printf("nd BtnThread btn5 KEY_PRESS BLUE_ON i2c_smbus_write_byte_data failed, res: %d\n", ret);
						}
					}
				 	else
					{ 
				 		if((current_time.tv_sec *1000*1000 + current_time.tv_usec - keys[i].press_time)/1000 > 3000)
						{
							ret = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_ON);
							if(ret < 0)
							{
								printf("nd BtnThread btn5 KEY_PRESS i2c_smbus_write_byte_data RED_ON failed, res: %d\n", ret);
							}
						}	
					}
				}
				
				if (keys[i].status == KEY_RELEASE) 
				{
					printf("nd BtnThread btn5 KEY_RELEASE\n");
			
					if((keys[i].release_time - keys[i].press_time)/1000 >= 3000
						&& (keys[i].release_time - keys[i].press_time)/1000 < 6000)
					{
						printf("BTN5 net config diffetime: %lld\n", (keys[i].release_time - keys[i].press_time)/1000);
						
						ncurrt_time = 0;		
						system("killall air");
						//system("ifconfig wlan0 down");
						system("iw dev wlan0 interface add fish0 type monitor flags none");
						system("ifconfig fish0 up");
						system("echo 0 > /sys/class/gpio/gpio1/value");
						system("aplay -t raw -f S16_LE -r 16000 /usr/share/start-network-config.raw &");
						system("/sbin/air &");
						system("exit 0");			
					}
					
					if((keys[i].release_time - keys[i].press_time)/1000 >= 8000)
					{
						printf("BTN5 ap config diffetime: %lld\n", (keys[i].release_time - keys[i].press_time)/1000);
						system("/sbin/setap &");
					}
					
					bBtn5Press = false;
					keys[i].press_time = 0;		
					keys[i].release_time = 0;
					keys[i].status = KEY_IDLE;
				}
				
			}
			
		}	

		usleep(1000*100);		
	}
}

/*btn5_click*/
static void btn5_click(CONTEXT *ctx)
{
	struct input_event * ev = (struct input_event *)(ctx->pdata);
	if (ev->value == 1) {
		printf("###pressed btn5_click###\n");
		keys[0].status = KEY_PRESS;
		keys[0].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
	} else {
		printf("###released btn5_click###\n");
		keys[0].status = KEY_RELEASE;
		keys[0].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
	}
	return;
}

/*btn6_click*/
static void btn6_click(CONTEXT *ctx)
{
	struct input_event * ev = (struct input_event *)(ctx->pdata);
	if (ev->value == 1) {
		printf("###pressed btn6_click###\n");
		keys[1].status = KEY_PRESS;
		keys[1].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
		printf("press: press_time:%ld, %ld, keys[1].press_time:%lld\n", ev->time.tv_sec, ev->time.tv_usec, keys[1].press_time);
	} else {
		printf("###released btn6_click###\n");
		keys[1].status = KEY_RELEASE;
		keys[1].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
		printf("release: keys[1].release_time:%lld\n", keys[1].release_time);
	}
	return;
}

/*btn7_click*/
static void btn7_click(CONTEXT *ctx)
{
	struct input_event * ev = (struct input_event *)(ctx->pdata);
	if (ev->value == 1) {
		printf("###pressed btn7_click###\n");
		keys[2].status = KEY_PRESS;
		keys[2].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
	} else {
		printf("###released btn7_click###\n");
		keys[2].status = KEY_RELEASE;
		keys[2].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
	}

	return;
}

/*btn8_click*/
static void btn8_click(CONTEXT *ctx)
{
	printf("###pressed btn8_click###\n");
	return;
}

/*Create the thread of SleepDevice*/
void* SleepDevice(void *arg)
{
	while(1)
	{
		int res = 0;

  		pthread_mutex_lock(&mutex);
	  	struct mpd_status *status = NULL;
	  	status = mpd_run_status(conn);
		if (!status) 
		{
			printf("nd SleepDevice mpd_run_status1 %s \n", mpd_connection_get_error_message(conn));
			close_connection(conn);
			new_connection(&conn);
		}
		else
		{
			if (mpd_status_get_state(status) == MPD_STATE_PLAY/* || bBtnEvent*/)
			{
				ncurrt_time = 0;
			}
			
			mpd_status_free(status);
		}
		pthread_mutex_unlock(&mutex);
	
		if(ncurrt_time >= SLEEP_DEVICE_TIMES)
		{
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER12, 00);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER13, 00);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER14, 03);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER15, 84);
		
			usleep(50*1000);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER7, SHUT_DWON);
			if(res < 0)
			{
				printf("nd SleepDevice i2c_smbus_write_byte_data 0x07, 0x01 failed, res: %d\n", res);
			}
		}
		else if(bvoice_vod && ncurrt_time == FOREVER_SLEEP_DEVICE_TIMES)
		{
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER12, 00);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER13, 00);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER14, 255);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER15, 255);
			
			usleep(50*1000);
			res = i2c_smbus_write_byte_data(i2c_file, REGISTER7, SHUT_DWON);
			if(res < 0)
			{
				printf("nd SleepDevice i2c_smbus_write_byte_data 0x07, 0x01 failed, res: %d\n", res);
			}
		}
		
		ncurrt_time++;
		sleep(1);
	}
}

/*close_connection*/
int close_connection(struct mpd_connection *conn)
{
	if(conn != NULL)
	{
		mpd_connection_free(conn);
		conn = NULL;
	}
	
	return 0;
}

/*Create the thread of DownloadThread*/
void* DownloadThread(void *arg)
{
	while (1)   
	{
		int ret = 0;
#if 0 //get json from file xxxxxx
		XmlData xmldata;

		ret = GetXmlData(DOWNLOAD_XML_PATH, DOWNLOAD_XML_NODE_NAME, xmldata);
		if(-1 != ret && xmldata.strUrl != "")
#endif
		if(-1 != ret )
		{
			char cmdstr[1024];
#if 0	//string error
			string strMusicName = xmldata.strName + ".mp3";
			snprintf(cmdstr, 1024, "curl \"%s\" -o /tmp/music/download/\"%s\"", xmldata.strUrl.c_str(), strMusicName.c_str());
#endif
			system(cmdstr);
			pthread_mutex_lock(&mutex);
			int ret = mpd_run_update(conn, "download");
			printf("nd DownloadThread mpd_run_update ret: %d\n", ret);
			system("rm -r /etc/config/.mpd/playlists/download.lst.m3u");
			bool b_ret = mpd_run_playlist_add (conn, "download.lst", "download");
			printf("nd DownloadThread mpd_run_playlist_add b_ret: %d\n", b_ret);
			pthread_mutex_unlock(&mutex);
			
		  	//printf("nd DownloadThread download cmdstr: %s, cmdstr: %s sucsse!\n", cmdstr, strMusicName.c_str());
#if 0	// del json from file
			XMLElement*  Elemt = GetXmlNodeByUrl(DOWNLOAD_XML_PATH, DOWNLOAD_XML_NODE_NAME, xmldata.strUrl);
			bool bDelXml = DelXmlNodeByUrl(DOWNLOAD_XML_PATH, Elemt/*, strNewNum*/);
			if(!bDelXml)
			{
				printf("delete %s : url: %s\n", DOWNLOAD_XML_PATH, xmldata.strUrl.c_str());
			}
#endif				
			ncurrt_time = 0;	
		}
	
		sleep(1);
	}
}

/*Create the thread of ReadJSONThread*/
void* ReadJSONThread(void *arg) 
{ 
	while(true)
	{
#if 0
		int ret = 0;
		int nNum = 0;
		int quere_len = 0;
		int online_size = 0;
		XmlData xmldt;
		
		ret = GetXmlData(PLAY_VOICE_XML_PATH, PLAY_VOICE_XML_NODE_NAME, xmldt);
		online_size = get_file_size(ONLINE_LIST_PATH);
		if((-1 != ret && !bplay_audio) || online_size > 0)
		{
			ret = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_BLIHK);
			if(ret < 0)
			{
				printf("nd ReadXMLThread i2c_smbus_write_byte_data BLUE_BLIHK failed, res: %d\n", ret);
			}
			
			bvoice_vod = true;
		}	
		else
		{
			bvoice_vod = false;	
		}
		
		while(bplay_audio)
		{
			ret = 0;
			nNum = 0;
			XmlData xmldata;

			ret = GetXmlData(PLAY_VOICE_XML_PATH, PLAY_VOICE_XML_NODE_NAME, xmldata);
			if(-1 != ret )
			{
				char cmdstr[1024];
				if(xmldata.strUrl != "")
				{
					snprintf(cmdstr, 1024, "curl %s -o /tmp/dwvoice.amr", xmldata.strUrl.c_str());
					system("rm /tmp/dwvoice.wav");
					system(cmdstr);
					//printf("curl---------------\n");
					system("amrnb-dec /tmp/dwvoice.amr /tmp/dwvoice.wav");
					system("cat /tmp/zero.dat >> /tmp/dwvoice.wav");
					system("rm_wav_head	 /tmp/dwvoice.wav");
					system("aplay -t raw -f S16_LE -r 8000 -c 1 /tmp/dwvoice.raw");
					//system("aplay /tmp/dwvoice.wav");
				}
				
				XMLElement*  Elemt = GetXmlNodeByUrl(PLAY_VOICE_XML_PATH, PLAY_VOICE_XML_NODE_NAME, xmldata.strUrl);
				bool bDelXml = DelXmlNodeByUrl(PLAY_VOICE_XML_PATH, Elemt/*, strNewNum*/);
				if(!bDelXml){
					printf("delete %s : url: %s\n", PLAY_VOICE_XML_PATH, xmldata.strUrl.c_str());
					}
					
				ret = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
				if(ret < 0)
				{
					printf("nd ReadXMLThread i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", ret);
				}
			
				bplay_audio = false;
			}
			else
			{
				bplay_audio = false;
			}
		}		
		
		usleep(600*1000);
#endif
	}
	return NULL;
}

