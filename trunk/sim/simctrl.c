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

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <pwd.h>

int main(int argc, char **argv)
{
   int s;
   char addr[256];
   uid_t uid = getuid();
   struct passwd *pwd = getpwuid(uid);
   if (pwd){
	snprintf(addr, sizeof(addr), "/tmp/sim.%s", pwd->pw_name);
   }else{
	snprintf(addr, sizeof(addr), "/tmp/sim.%u", uid);
   }
   s = socket(PF_UNIX, SOCK_STREAM, 0);
   return 0;
}

