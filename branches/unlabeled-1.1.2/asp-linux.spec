
%define subversion 1
%define date 20021009
Summary:   SIM - Simple Instant Messenger
Name:      sim
Version:   0.8
Release:   1
License:   GPL
Url:       http://sim-icq.sourceforge.net/
Group:     Applications/Communications
Source:    sim-%{version}.tar.bz2
BuildRoot: %{_tmppath}/sim-buildroot
BuildRequires:  qt-devel kdelibs-devel arts-devel


%description
A simple ICQ client with v8 protocol support (2001) for X win system
(requires QT, can be build for KDE). It also runs under MS Windows.

%prep

%setup
make -f admin/Makefile.common
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure $LOCALFLAGS

%build
%configure

#clean up
make clean

# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

mv $RPM_BUILD_ROOT/%{_bindir}/i386-redhat-linux-sim
$RPM_BUILD_ROOT/%{_bindir}/sim

%find_lang %{name}

%clean

%files -f %{name}.lang
%defattr(-, root, root, 755)
%doc AUTHORS COPYING README TODO INSTALL
%{_bindir}/sim
%{_datadir}/applnk/Internet/sim.desktop
%dir %{_datadir}/apps/sim
%{_datadir}/apps/sim/icons
%dir %{_datadir}/apps/sim/pict
%{_datadir}/apps/sim/pict/*
%dir %{_datadir}/apps/sim/sounds
%{_datadir}/apps/sim/sounds/*
%{_datadir}/icons/*/*/*/*


%changelog

* Wed Oct 09 2002 Leon Kanter <leon@asplinux.ru>
- 0.8 release

* Tue Sep 24 2002 Leon Kanter <leon@asplinux.ru>
- this cvs snapshot should fix font-related problems

* Wed Sep 11 2002 Leon Kanter <leon@asplinux.ru>
- Spec cleanup, built for asplinux%{_bindir}/sim

