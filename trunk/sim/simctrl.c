#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <pwd.h>

static void usage(char *s)
{
    uid_t uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    char uid_buf[256];
    char *name = NULL;
    if (pwd){
        name = pwd->pw_name;
    }else{
        snprintf(uid_buf, sizeof(uid_buf), "%u", uid);
        name = uid_buf;
    }
    fprintf(stderr,
            "usage: %s [options] command [command args...]\n"
            "Options are: [followed by default value]:\n"
            "      -s socket	[/tmp/sim.%s] Control socket\n"
	    "      -d                         Debug mode\n"
            "\n",
            s, name);
}

char *local_name = NULL;

static void sig_handler()
{
}

static void sim_cleanup()
{
    if (local_name != NULL)
        unlink(local_name);
}

int main(int argc, char **argv)
{
    int i;
    int s;
    int debug = 0;
    char addr_buf[256];
    char local_name_buf[256];
    char line[1024];
    uid_t uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    char *addr = NULL;
    char *l;
    char c;
    struct sockaddr_un sun_local;
    struct sockaddr_un sun_remote;
    FILE *f;

    while ((c = getopt(argc, argv, "s:d")) != -1){
        switch (c){
        case 's':
            addr = optarg;
	    break;
        case 'd':
	    debug = 1;
	    break;
        default:
            usage(argv[0]);
        }
    }
    argc -= optind;
    if (argc == 0) usage(argv[0]);
    argv += optind;

    signal(SIGPIPE, sig_handler);

    if (pwd){
        snprintf(addr, sizeof(addr), "/tmp/sim.%s", pwd->pw_name);
    }else{
        snprintf(addr, sizeof(addr), "/tmp/sim.%u", uid);
    }
    s = socket(PF_UNIX, SOCK_STREAM, 0);
    if (s < 0){
        fprintf(stderr, "Can't create socket: %s\n", strerror(errno));
        exit(1);
    }
    if ((addr == NULL) || (*addr == 0)){
        if (pwd){
            snprintf(addr_buf, sizeof(addr_buf), "/tmp/sim.%s", pwd->pw_name);
        }else{
            snprintf(addr_buf, sizeof(addr_buf), "/tmp/sim.%u", uid);
        }
        addr = addr_buf;
    }
    atexit(sim_cleanup);
    strcpy(local_name_buf, "/tmp/sim.XXXXX");
    local_name = mktemp(local_name_buf);

    sun_local.sun_family = AF_UNIX;
    strcpy(sun_local.sun_path, local_name);
    if (bind(s, (struct sockaddr*)&sun_local, sizeof(sun_local)) < 0){
        fprintf(stderr, "Can't bind %s: %s\n", local_name, strerror(errno));
        exit(1);
    }
    sun_remote.sun_family = AF_UNIX;
    strcpy(sun_remote.sun_path, addr);
    if (connect(s, (struct sockaddr*)&sun_remote, sizeof(sun_remote)) < 0){
        fprintf(stderr, "Can't connect to %s: %s\n", addr, strerror(errno));
        exit(1);
    }
    unlink(local_name);
    local_name = NULL;
    f = fdopen(s, "rw");
    for (;;){
    	l = fgets(line, sizeof(line), f);
    	if (l == NULL){
		fprintf(stderr, "Can't get prompt\n");
		exit(1);
    	}
	for (i = strlen(l); i >= 0; i--){
		if (l[i] >= ' ') break;
		l[i] = 0;		
	}
	if (debug) printf("SRV: %s\n", l);
        if (strcmp(l, "SIM ready") == 0) break;
    }
    if (debug) printf("CLN: ");
    for (; argc > 0; argc--, argv++){
	if (debug) printf("\"%s\" ", *argv);
	fprintf(f, "\"%s\" ", *argv);
    }
    if (debug) printf("\n");
    fprintf(f, "\r\n");
    for (;;){
        l = fgets(line, sizeof(line), f);
        if (l == NULL){
                fprintf(stderr, "Can't get prompt\n");
                exit(1);
        }
        for (i = strlen(l); i >= 0; i--){
                if (l[i] >= ' ') break;
                l[i] = 0;
        }
        if (debug) printf("SRV: %s\n", l);
    }
    return 0;
}

