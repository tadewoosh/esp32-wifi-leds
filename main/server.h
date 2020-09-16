#ifndef TCP_SERVER_H
#define TCP_SERVER_H

extern volatile int connected;

void server_task(void *pvParameters);


#endif // TCP_SERVER_H
