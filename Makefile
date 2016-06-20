CROSS_COMPILE = mips-openwrt-linux-
CC = gcc

CFLAGS = -Wall -ggdb -O2 -I./include
JSON_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/json-c
MPD_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/mpd
#I2C_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/include/linux
CLIENT_CFLAGS = ${CFLAGS} ${JSON_CFLAGS} ${MPD_CFLAGS} ${I2C_CFLAGS} -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/

CLIENT_LDFLAGS = ${CFLAGS} -L/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/lib -lssl -lcrypto -lcares -lmosquitto -lpthread -ljson-c -lmpdclient

.PHONY: all reallyclean clean

all : mosquitto_client

mosquitto_client : mosquitto_client.o mosquitto_parse.o mosquitto_init.o mosquitto_mpc.o mosquitto_i2c.o
	${CROSS_COMPILE}${CC} $^ -o $@ ${CLIENT_LDFLAGS}

mosquitto_client.o : mosquitto_client.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_parse.o : mosquitto_parse.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_init.o : mosquitto_init.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_mpc.o : mosquitto_mpc.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_i2c.o : mosquitto_i2c.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS} 

reallyclean : clean
	
clean : 
	-rm -f *.o mosquitto_client
