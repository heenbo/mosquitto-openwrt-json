#include <stdio.h>
#include <pthread.h>
#include <connection.h>
#include <status.h>
#include <player.h>

#include "mosquitto_mpc.h"

struct mpd_connection *conn = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*MPC init*/
void InitMPC(void)
{
	FILE * pFile;
	pFile = fopen (ONLINE_LIST_PATH,"wb");
	if (pFile==NULL)
	{
		perror ("nd InitMPC create online.lst.m3u Error\n");
	}
	fclose(pFile);
    
	new_connection(&conn);
}

/*new_connection*/
int new_connection(struct mpd_connection **conn)
{
	*conn = mpd_connection_new(NULL, 0, 30000);
	if (*conn == NULL)
	{
		printf("nd new_connection %s\n", "Out of memory");
		return -1;
	}

	if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
	{
		printf("nd new_connection %s\n", mpd_connection_get_error_message(*conn));
		mpd_connection_free(*conn);
		*conn = NULL;
		return -1;
	}
	return 0;
}

/*my_mpd_run_pause*/
void my_mpd_run_pause(void)
{
	struct mpd_status *status = NULL;

	pthread_mutex_lock(&mutex);
	status = mpd_run_status(conn);
	if (!status) 
	{
		printf("nd my_mpd_run_pause mpd_run_status1 %s \n", mpd_connection_get_error_message(conn));
	}
	else
	{
		if (mpd_status_get_state(status) == MPD_STATE_PLAY)
		{
			mpd_run_pause(conn, true);
		}
	
		mpd_status_free(status);
	}
	
	pthread_mutex_unlock(&mutex);
}
