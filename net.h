#define	FROM_BT		0
#define	FROM_NET	1
#define	TO_BT		2
#define	TO_NET		3
#define MSG_WAITFIRST	((!i) ? MSG_WAITALL : MSG_DONTWAIT)
#define	TIMEOUT		1000 
#define	PORT		12345
#define	DEVICE		"00:00:00:00:00:00"
#define	ERROR_HOST	"HTTP/1.1 404 Not Found\r\n\r\n<h1>Host Not Found</h1> (lol)"
#define	ERROR_SEND	"HTTP/1.1 404 Not Found\r\n\r\nPlease try to reload the page."


char *_recv (int sd,  int *len, int from) {
	struct pollfd pollo;
	int i;
	char *ptr, ch;
	switch (from) {
		case FROM_BT:
			recv (sd, &i, sizeof (int), MSG_WAITALL);
			ptr = (char*) malloc (i+1*sizeof (char));
			recv (sd, ptr, i, MSG_WAITALL);
			ptr [i] = 0x00;
			break;
		case FROM_NET:
			pollo.fd=sd;
			pollo.events=POLLIN;
			ptr = (char*) malloc (1*sizeof(char));
			i = 0;
			while (recv (sd, &ch, 1, MSG_WAITFIRST)>0) {
				ptr = (char*) realloc (ptr, ++i*sizeof(char));
				ptr [i-1] = ch;
				poll ((struct pollfd*)&pollo, 1, TIMEOUT);
			}
			ptr = (char*) realloc (ptr, ++i*sizeof(char));
			ptr [i-1] = 0x00;
			break;
		default:
			break;
	}
	*len = i;
	return ptr;
}

int _send (int sd, char *msg, int len, int to) {
	switch (to) {
		case TO_BT:
			if (send (sd, &len, sizeof (int), 0)<0)
				return 0;
			send (sd, msg, len, 0);
			break;
		case TO_NET:
			send (sd, msg, len, 0);
			break;
		default:
			break;
	}
	return 1;
}

char *get_host (char* request) {
	char *host;
	int len = strchr (strstr (request, "Host: "), 0x0d)-strstr (request, "Host: ")-6;
	host = (char*) malloc (len+1);
	strncat (host, strstr (request, "Host: ")+6, len);
	return host;
}

char *host2ip (char *host) {
	char *ip, *line;
	FILE *pp;
	int i;
	line = (char*) malloc ((16+strlen(host))*sizeof(char));
	sprintf (line, "ping %s -c 1 -w 1", host);
	pp = popen (line, "r");
	line = (char*) realloc (line, 1024*sizeof(char));
	fgets (line, 1024, pp);
	fgets (line, 1024, pp);
	if (line [0] == 0x0a)
		return NULL;
	i = strchr (line, 0x29)-strchr (line, 0x28)-1;
	if (i<0)
		return NULL;
	ip = (char*)malloc((i+1)*sizeof(char));
	strncpy (ip, strchr (line, 0x28)+1, i);
	line [i] = 0x00;
	free (line);
	return ip;
}

char *remove_ua (char *request) {
        char *new;
        int i, d, n;
        if (!strstr (request, "User-Agent"))
                return request;
        i = strstr (request, "User-Agent")-request;
        n = strchr (strstr (request, "User-Agent"), 0x0d)-strstr (request, "User-Agent")+2;
        d = request+strlen(request)-strstr (request, "User-Agent");
        new = (char*) malloc (i+d-n+1);
        strncat (new, request, i);
        strncat (new, request+i+n, d);
        new [strlen(new)] = 0x00;
        return new;
}

