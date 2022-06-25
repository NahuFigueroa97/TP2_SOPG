/*
  ******************************************************************************
  * @file    SerialService
  
  
  * @author  HÉCTOR RAMÓN RIZO
  
  
  * @date    22/06/2022
  ******************************************************************************
  */

/* INCLUDES */

#include "ThreadInterface.h"

pthread_t serial_protocol_t, server_tcp_t;

void sigint_handler(int sig)
{
	pthread_cancel(serial_protocol_t);
	pthread_cancel(server_tcp_t);
}

void sigterm_handler(int sig)
{
	pthread_cancel(serial_protocol_t);
	pthread_cancel(server_tcp_t);
}

int main(void)
{
	ThreadInterface *threadInterface = new_thread_interface(serial_protocol_t, server_tcp_t);

	struct sigaction sa;
	struct sigaction sa2;	

	//Se configuran los handlers de las señales
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		exit(1);
	}

	sa2.sa_handler = sigterm_handler;
	sa2.sa_flags = 0;
	sigemptyset(&sa2.sa_mask);
	if (sigaction(SIGTERM,&sa2,NULL)==-1)
	{
		exit(1);
	}

	threadInterface->lock_signal();

	pthread_create(&serial_protocol_t, NULL, threadInterface->serial_protocol(threadInterface), NULL);
	pthread_create(&server_tcp_t, NULL, threadInterface->server_tcp(threadInterface), NULL);

	threadInterface->unock_signal();

	/*Se espera que los threads terminen y se liberan los espacios de memoria*///
	pthread_join (serial_protocol_t, NULL);
	pthread_join (server_tcp_t, NULL);
	pthread_mutex_lock(&threadInterface->mutex);
	if (threadInterface->is_connected)
	{
		close(threadInterface->newfd);
	}
	pthread_mutex_unlock(&threadInterface->mutex);
	serial_close();
	destructor(threadInterface);
	exit(EXIT_SUCCESS);
	return 0;
}
