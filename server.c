#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "net.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main () {
	if (!fork ()) {
		char *req, *reply, *host, *ip;
		struct sockaddr_rc server, client;
		struct sockaddr_in addr;
		struct hostent *he;
		int s_sd, c_sd, in_sd, i, size;
		size = sizeof (client);
		server.rc_family = AF_BLUETOOTH;
		server.rc_channel = 1;
		server.rc_bdaddr = *BDADDR_ANY;
		addr.sin_family = AF_INET;
		addr.sin_port = htons (80);
		s_sd = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		bind (s_sd, (struct sockaddr*)&server, size);
		listen (s_sd, 10);
		while (1) {
			c_sd = accept (s_sd, (struct sockaddr*)&client, &size);
			if (!fork ()) {
				req = remove_ua (_recv (c_sd, &i, FROM_BT));
				host = get_host (req);
				he = gethostbyname (host);
				if (!he) {
					ip = host2ip (host);
					if (!ip) {
						_send (c_sd, ERROR_HOST, strlen (ERROR_HOST), TO_BT);
						close (c_sd);
						close (s_sd);
						free (req);
						free (host);
						free (ip);
						return 0;
					}
					else {
						addr.sin_addr.s_addr = inet_addr (ip);
						free (ip);
					}
				}
				else
					memcpy ((char*)&addr.sin_addr.s_addr, (char*)he->h_addr_list[0], 4);
				in_sd = socket (AF_INET, SOCK_STREAM, 0);
				connect (in_sd, (struct sockaddr*)&addr, sizeof (addr));
				_send (in_sd, req, i, TO_NET);
				reply = _recv (in_sd, &i, FROM_NET);
				_send (c_sd, reply, i, TO_BT);
				close (c_sd);
				close (s_sd);
				close (in_sd);
				free (req);
				free (reply);
				free (host);
				return 0;
			}
			close (c_sd);
		}
		close (s_sd);
	}
	return 0;
}
