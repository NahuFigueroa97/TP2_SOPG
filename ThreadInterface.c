#include "ThreadInterface.h"

ThreadInterface *new_thread_interface(pthread_t serial_protocol_, pthread_t server_tpc_)
{
    ThreadInterface *threadInterface = NULL;
    threadInterface = (ThreadInterface*)malloc(sizeof(ThreadInterface));
    if(threadInterface == NULL)
    {
        return NULL;
    }
    threadInterface->mutex = PTHREAD_MUTEX_TIMED_NP;
    threadInterface->destructor = destructor;
    threadInterface->serial_protocol = serial_protocol;
    threadInterface->server_tcp = server_tcp;
    threadInterface->lock_signal = lock_signal;
    threadInterface->unock_signal = unlock_signal;
    return threadInterface;
}

void destructor(ThreadInterface *threadInterface)
{
    free(threadInterface);
    threadInterface = NULL;
}

void serial_protocol(ThreadInterface *threadInterface)
{
    char buffer [12];
	char *buffer_send_tcp;
	if(serial_open(PORT_EMU, 115200)!= 0)
	{
		exit(1);
	}
	else
	{
		while (true)
		{
			if( serial_receive(buffer,12) )
			{
				if(buffer[0]=='>' && buffer[1]=='S' && buffer[2]=='W' && buffer[3]==':' && buffer[5]==',')
				{
					buffer_send_tcp = buffer;
					pthread_mutex_lock(&threadInterface->mutex);
					if(threadInterface->is_connected)
					{
						if ( (write(threadInterface->newfd, buffer_send_tcp, 12)) == ERROR)
						{
							exit (1);
						}
					}
					pthread_mutex_unlock(&threadInterface->mutex);
				}	
			}
			usleep(100);	
		}
	}
}

void server_tcp(ThreadInterface *threadInterface)
{
    socklen_t addr_len;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	char buffer[10];
	int n;
	int s = socket(AF_INET,SOCK_STREAM, 0); 

    bzero((char*) &server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10000);

    if(inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0)
    {
		exit(1);
    }

	if (bind(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		close(s);
		exit(1);
	}

	if (listen (s, 10) == ERROR)
  	{
    	exit(1);
  	}

	while(true)
	{
		addr_len = sizeof(struct sockaddr_in);
    	if ( (threadInterface->newfd = accept(s, (struct sockaddr *)&client_addr,&addr_len)) == -1)
      	{
		    exit(1);
		}
		pthread_mutex_lock(&threadInterface->mutex);
	 	threadInterface->is_connected = true;
		pthread_mutex_unlock(&threadInterface->mutex);
		char ip_client[32];
		inet_ntop(AF_INET, &(client_addr.sin_addr), ip_client, sizeof(ip_client));

		while(threadInterface->is_connected) 
		{
			char *buffer_send_serial;

			if( (n = read(threadInterface->newfd, buffer, 10)) == ERROR )   
			{
				exit(1);
			}
			
			if (n == 0)  
			{
				pthread_mutex_lock(&threadInterface->mutex);
				threadInterface->is_connected = false;
				pthread_mutex_unlock(&threadInterface->mutex);
			}

			buffer[n]=0x00;
			if(buffer[0]=='>' && buffer[1]=='O' && buffer[2]=='U' && buffer[3]=='T' && buffer[4]==':' && buffer[6]==',')
			{
				buffer_send_serial = buffer;
				serial_send(buffer_send_serial, 10); 
			}
		}
    	close(threadInterface->newfd);   
	} 

}

void lock_signal()
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void unlock_signal()
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void sigint_handler(ThreadInterface *threadInterface)
{
    pthread_cancel(threadInterface->serial_protocol_t);
	pthread_cancel(threadInterface->server_tpc_t);
}

void sigterm_handler(ThreadInterface *threadInterface)
{
    pthread_cancel(threadInterface->serial_protocol_t);
	pthread_cancel(threadInterface->server_tpc_t);
}