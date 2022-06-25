#ifndef THREADINTERFACE_H_
#define THREADINTERFACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "SerialManager.h"
#include <signal.h>

#define PORT_EMU  1 
#define ERROR	 -1

typedef struct ThreadInterface ThreadInterface;

typedef void (*ptr_serial_protocol)(ThreadInterface *);
typedef void (*ptr_server_tcp)(ThreadInterface *);
typedef void (*ptr_lock_signal)(void);
typedef void (*ptr_unlock_signal)(void);
typedef void (*ptr_destructor)(ThreadInterface *);

typedef struct ThreadInterface
{
    int newfd;
    bool is_connected;
    pthread_t serial_protocol_t;
    pthread_t server_tpc_t;
    pthread_mutex_t mutex;
    ptr_serial_protocol serial_protocol;
    ptr_server_tcp server_tcp;
    ptr_lock_signal lock_signal;
    ptr_unlock_signal unock_signal;
    ptr_destructor destructor;
};

ThreadInterface *new_thread_interface(pthread_t serial_protocol_, pthread_t server_tpc_);
void destructor(ThreadInterface *);
void serial_protocol(ThreadInterface *threadInterface);
void server_tcp(ThreadInterface *threadInterface);
void lock_signal();
void unlock_signal();

#endif