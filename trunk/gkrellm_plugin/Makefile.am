
gkrellm_plugin_LTLIBRARIES = simkrell.la

simkrell_la_SOURCES =	\
 simkrell.c 	\
 options.c

INCLUDES = -I../sim/xpm
AM_CFLAGS  = $(CFLAGS) $(GLIB_CFLAGS)
simkrell_la_LDFLAGS = -module -no-undefined -avoid-version $(GLIB_LDFLAGS) $(GLIB_LIBADD) 
simkrell_la_METASOURCES = AUTO

noinst_HEADERS = simkrell.h

install-data-hook:
	rm -rf $(gkrellm_plugindir)/simkrell.la

