#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <json_object.h>
#include "mosquitto_parse.h"

char *ch_music_url = NULL;

int parse_msg(CONTEXT * ctx, char * buf, unsigned short len)
{
	json_object *json = NULL, *cmd = NULL, *did = NULL;
	enum json_tokener_error error = json_tokener_success;
	json = json_tokener_parse_verbose(buf, &error);
	char * str = NULL;
	//char * did_str = NULL;
	
	printf("FUNC:%s LINE: %d parse_cmdstr Debug:parse fifo json string len:%d, :%s\n", __FUNCTION__, __LINE__, len, buf);
	
	if (!json) {
		printf("parse_cmdstr Error, parse fifo json token :%s failed\n", buf);
		return -1;	
	}

	if (json_object_object_get_ex(json, "cmd", &cmd) == TRUE)
	{
		if (json_object_get_type(cmd) == json_type_string)
		{
			str = (char *)json_object_get_string(cmd);
			printf("FUNC:%s LINE:%d str: %s \n", __FUNCTION__, __LINE__, str);
			json_object_object_get_ex(json, "device_id", &did);

			if (did)
			{
				//did_str = (char *)json_object_get_string(did);
				json_object_get_string(did);
			}

			if (!strcmp(str, "send_voice"))
			{
				json_object *contact_id = NULL;
				char *contact_str = NULL;

//				system("upvoice.sh");
				json_object_object_get_ex(json, "contact_id", &contact_id);
				if (contact_id)
				{
					contact_str = (char *)json_object_get_string(contact_id);
					printf("FUNC:%s LINE:%d contact_id: %s \n", __FUNCTION__, __LINE__, contact_str);
				}
//				send_voice(contact_str);	
			}
			
			if (!strcmp(str, "xfchat_music"))
			{
				json_object *js_music_name = NULL, *js_music_url = NULL, *js_rc_type = NULL;
				char *ch_music_name = NULL;
				char *ch_rc_type = NULL;

				json_object_object_get_ex(json, "rc_type", &js_rc_type);
				if (js_rc_type)
				{
					ch_rc_type = (char *)json_object_get_string(js_rc_type);
				}	

				json_object_object_get_ex(json, "name", &js_music_name);
				if (js_music_name)
				{
					ch_music_name = (char *)json_object_get_string(js_music_name);
				}
	
				json_object_object_get_ex(json, "url", &js_music_url);
				if (js_music_url)
				{
					ch_music_url = (char *)json_object_get_string(js_music_url);
				}

				printf("ch_music_name: %s, ch_music_url: %s, ch_rc_type: %s\n", ch_music_name, ch_music_url, ch_rc_type);
//				check_xfchat_music(ch_music_name, ch_rc_type);
				
			}
			
			if (!strcmp(str, "stop"))
			{
//				system("/usr/bin/ais_led blue off");
			}		
		}
	}

	return 0;
}
