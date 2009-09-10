#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "net.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main () {
	if (!fork ()) {
		char *req, *reply;
		struct sockaddr_in client, server;
		struct sockaddr_rc addr;
		int c_sd, s_sd, bt_sd, i, size;
		size = sizeof (client);
		server.sin_family = AF_INET;
		server.sin_port = htons (PORT);
		server.sin_addr.s_addr = INADDR_ANY;
		addr.rc_family = AF_BLUETOOTH;
		addr.rc_channel = 1;
		str2ba (DEVICE, &addr.rc_bdaddr);
		s_sd = socket (AF_INET, SOCK_STREAM, 0);
		bind (s_sd, (struct sockaddr*)&server, size);
		listen (s_sd, 10);
		while (1) {
			c_sd = accept (s_sd, (struct sockaddr*)&client, &size);
			if (!fork ()) {
				bt_sd = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
				connect (bt_sd, (struct sockaddr*)&addr, sizeof (addr));
				req = _recv (c_sd, &i, FROM_NET);
				// This is going to be a temporary solution, 
				// I'll fix it soon (first of all I'll try to find the problem xD)
				if (!_send (bt_sd, req, i, TO_BT)) {
					_send (c_sd, ERROR_SEND, strlen (ERROR_SEND), TO_BT);
					close (bt_sd);
					close (c_sd);
					close (s_sd);
					free (req);
					return 0;
				}
				reply = _recv (bt_sd, &i, FROM_BT);
				_send (c_sd, reply, i, TO_NET);
				close (bt_sd);
				close (c_sd);
				close (s_sd);
				free (req);
				free (reply);
				return 0;
			}
			close (c_sd);
		}
		wait (NULL);
		close (s_sd);
	}
	return 0;
}


