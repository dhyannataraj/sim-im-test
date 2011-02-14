COMMON_IGNORE=\
'
Makefile
Makefile.in
configure.in
cmake_install.cmake
progress.make
CMakeFiles
CPackConfig.cmake
CPackSourceConfig.cmake
CMakeCache.txt
install_manifest.txt
cmake_uninstall.cmake
configure
configure.files
config.h.in
config.h
config.status
libtool
*.log
*.kdevelop
*.kdevelop.pcs
*.kdevses
build-stamp*
stamp-h.in
install-stamp-sim-data
make-common-stamp
stamp-h1
svn-commit.tmp
autom4te.cache
acinclude.m4
aclocal.m4
*.moc
*base.cpp
*base.cc
*base.h
.libs
.deps
*.jisp
*.jisp.files
Doxyfile
'

svn propset -R svn:ignore "$COMMON_IGNORE" .

svn propset -R svn:ignore "$COMMON_IGNORE"\
'html.cc
sim' \
    sim

svn propset -R svn:ignore "$COMMON_IGNORE"'rtf.cc' plugins/icq
svn propset -R svn:ignore "$COMMON_IGNORE"'parseurl.cc' plugins/navigate
svn propset -R svn:ignore "$COMMON_IGNORE"'simctrl' plugins/remote

svn propset -R svn:ignore "$COMMON_IGNORE"'*.gmo' po
