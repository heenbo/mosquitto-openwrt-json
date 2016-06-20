#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mosquitto_mpc.h"
#include "mosquitto_i2c.h"
#include "mosquitto_init.h"


/*Create the thread of I2C*/
int nI2C = -1;
pthread_t pthdI2C;

/*Create the thread of button*/
int nBtn = -1;
pthread_t pthdBtn;

/*Create the thread of button*/
int nSleepDevice = -1;
pthread_t pthdSleepDevice;

/*Create the thread of DownloadThread*/
int nDownload = -1;
pthread_t pthdDownload;

/*fd*/
extern int xfchat_fifo;
extern int read_xfchat_fifo;

/*Create the thread of ReadJSONThread*/
int nReadJSON = -1;
pthread_t pthdReadJSON;

/*system Init*/
int Init(void)
{
	printf("FUNC: %s LINE: %d Init test start \n", __FUNCTION__, __LINE__);
	system("dd if=/dev/zero of=/tmp/zero.dat bs=1024 count=10");

	
	/*MPC init*/
	InitMPC();

	/*Init I2CDev*/
	InitI2CDev();

	/*Close Light*/
	CloseLight();

	/*Create the thread of I2C*/
	nI2C = pthread_create(&pthdI2C, NULL, I2CThread, NULL);
	
	/*Create the thread of button*/
	nBtn = pthread_create(&pthdBtn, NULL, BtnThread, NULL);

	/*Create the thread of ReadJSONThread*/
	nReadJSON = pthread_create(&pthdReadJSON, NULL, ReadJSONThread, NULL);

	/*Create the thread of SleepDevice*/
	nSleepDevice = pthread_create(&pthdSleepDevice, NULL, SleepDevice, NULL);

	/*Create the thread of DownloadThread*/
	nDownload = pthread_create(&pthdDownload, NULL, DownloadThread, NULL);
	
	/*InitFifo*/
	InitFifo();

	/*init_gpio7*/
	init_gpio7();


	return 0;
}

int InitCommFifo(void)
{
	if (mkfifo("/var/run/comm.fifo", 0644) < 0)
	{
		perror("nd InitCommFifo mkfifo: ");
	}
	
	return 0;
}

int InitXfchatFifo(void)
{
	if (mkfifo("/tmp/xfchat.fifo", 0644) < 0)
	{
		perror("nd InitXfchatFifo mkfifo: ");
	}
	
	read_xfchat_fifo = open("/tmp/xfchat.fifo", O_RDONLY | O_NONBLOCK);
	if (xfchat_fifo < 0) 
	{
		perror("nd InitXfchatFifo open read fifo: ");
	}
	
	xfchat_fifo = open("/tmp/xfchat.fifo", O_WRONLY | O_NONBLOCK);
	if (xfchat_fifo < 0) 
	{
		perror("nd InitXfchatFifo open sck_write fifo: ");
	}
	
	return 0;
}

/*InitFifo*/
int InitFifo(void)
{
	InitCommFifo();
	InitXfchatFifo();
	
	return 0;
}

/*init_gpio7*/
int init_gpio7(void)
{
	system("echo 7 > /sys/class/gpio/export");
	system("echo in > /sys/class/gpio/gpio7/direction");
	
	return 0;
}


