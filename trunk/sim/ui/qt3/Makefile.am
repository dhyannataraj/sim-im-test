if QT2

lib_LTLIBRARIES = libsimuiqt.la

libsimuiqt_la_SOURCES = \
	qcomplextext.cpp \
	qkeysequence.cpp \
	qregexp.cpp \
	qrichtext.cpp \
	qrichtext_p.cpp \
	qsimplerichtext.cpp \
	qstylesheet.cpp \
	qtextedit.cpp \
	qsyntaxhighlighter.cpp 

libsimuiqt_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) $(LIBDL)

endif

# set the include path for X, qt and KDE
INCLUDES=-I../../api $(all_includes) 

METASOURCES = AUTO

# the library search path. 
libsimuiqt_la_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


