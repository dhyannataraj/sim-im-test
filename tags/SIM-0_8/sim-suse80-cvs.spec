# spec created by Crissi
Summary:   SIM - Simple Instant Messenger
Name:      sim
Version:   0.7
Release:   19072002
Copyright: GPL
Vendor:    Vladimir Shutoff <shutoff@mail.ru>
Url:       http://sim.shutoff.spb.ru/
Packager:  Crissi <crissi99@gmx.de>
Group:     X11/KDE/Network
Source:    sim-cvs-19072002.tgz
BuildRoot: %{_tmppath}/sim-buildroot
Distribution: SuSE 8.0

# neededforbuild  kde3-devel-packages
# usedforbuild    aaa_base aaa_dir aaa_version arts arts-devel autoconf automake base bash bindutil binutils bison bzip compat cpio cpp cracklib cyrus-sasl db devs diffutils docbook-dsssl-stylesheets docbook_3 e2fsprogs fam file fileutils findutils flex freetype2 freetype2-devel gawk gcc gcc-c++ gdbm gdbm-devel gettext glibc glibc-devel glibc-locale gpm grep groff gzip iso_ent jade_dsl kbd kdelibs3 kdelibs3-devel less libgcc libjpeg liblcms libmng libmng-devel libpng libpng-devel libstdc++ libstdc++-devel libtiff libtool libxcrypt libxml2 libxml2-devel libxslt libxslt-devel libz m4 make man mesa mesa-devel mesaglu mesaglu-devel mesaglut mesaglut-devel mesasoft mktemp modutils ncurses ncurses-devel net-tools netcfg openssl openssl-devel pam pam-devel pam-modules patch perl ps qt3 qt3-devel rcs readline rpm sed sendmail sh-utils shadow sp sp-devel strace syslogd sysvinit tar texinfo textutils timezone unzip util-linux vim xdevel xf86 xshared


%description
A simple ICQ client with v8 protocol support (2001) for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.

%prep
export KDEDIR=/opt/kde3
export QTDIR=/usr/lib/qt3

%setup
#make -f Makefile.dist
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure $LOCALFLAGS

%build

# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.sim
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.sim
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.sim

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/sim*
rm -rf ../file.list.sim


%files -f ../file.list.sim
