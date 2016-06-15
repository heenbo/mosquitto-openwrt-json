CROSS_COMPILE = mips-openwrt-linux-
CC = gcc

CFLAGS = -Wall -ggdb -O2
JSON_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/json-c
CLIENT_CFLAGS = ${CFLAGS} ${JSON_CFLAGS} -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include

CLIENT_LDFLAGS = ${CFLAGS} -L/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/lib -lssl -lcrypto -lcares -lmosquitto -lpthread -ljson-c

.PHONY: all reallyclean clean

all : mosquitto_client

mosquitto_client : mosquitto_client.o mosquitto_parse.o 
	${CROSS_COMPILE}${CC} $^ -o $@ ${CLIENT_LDFLAGS}

mosquitto_client.o : mosquitto_client.c mosquitto_client.h
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_parse.o : mosquitto_parse.c mosquitto_parse.h
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

reallyclean : clean
	
clean : 
	-rm -f *.o mosquitto_client
