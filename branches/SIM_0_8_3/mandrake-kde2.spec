%define name  sim
%define version 0.8
%define release 1mdk 

Name: %{name} 
Summary: SIM - Simple Instant Messenger 
Version: %{version} 
Release: %{release} 
Source: %{name}-%{version}.tar.gz 
URL: http://sim-icq.sourceforge.net/ 
Group:  X11/KDE/Network
BuildRoot: %{_tmppath}/%{name}-buildroot 
License:  GPL

%description
A simple ICQ client with v8 protocol support (2001) for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.

%prep
rm -rf $RPM_BUILD_ROOT
export KDEDIR=/usr
export QTDIR=/usr/lib/qt2
export PATH=$KDEDIR/bin:$QTDIR/bin:$PATH
rm -f /usr/lib/libpng.so
ln -s /usr/lib/libpng.so.2 /usr/lib/libpng.so

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
