#ifndef _MOSQUITTO_MPC_H_
#define _MOSQUITTO_MPC_H_

#include "connection.h"

#define ONLINE_LIST_PATH                          "/etc/config/.mpd/playlists/online.lst.m3u"


/*MPC init*/
extern void InitMPC(void);

/*my_mpd_run_pause*/
extern void my_mpd_run_pause(void);

/*new_connection*/
extern int new_connection(struct mpd_connection **conn);

#endif //_MOSQUITTO_MPC_H_
