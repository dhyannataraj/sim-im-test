%define rh_release %(rh_release="`rpm -q --queryformat='%{VERSION}' redhat-release 2>/dev/null`" ; if test $? != 0 ; then rh_release="" ; fi ; echo "$rh_release")
%define release %(release="`echo "%{rh_release} * 10" | bc 2>/dev/null`" ; if test $? != 0 ; then release="" ; fi ; echo "$release")

Name: 		sim
Version: 	0.9.2
Release: 	1.rh%{release}
Vendor: 	Vladimir Shutoff <shutoff@mail.ru>
Packager:	Robert Scheck <sim@robert-scheck.de>
Summary:  	SIM - Multiprotocol Instant Messenger
Summary(de):	SIM - Multiprotokoll Instant Messenger
License: 	GPL
Group: 		X11/KDE/Network
URL: 		http://sim-icq.sourceforge.net/
Source0: 	http://osdn.dl.sourceforge.net/sourceforge/sim-icq/%{name}-%{version}.tar.gz
BuildRequires:	autoconf >= 2.52, automake >= 1.5
BuildRequires:  gcc, gcc-c++, XFree86-devel, zlib-devel, libjpeg-devel, expat-devel, flex, libart_lgpl-devel, libpng-devel, gettext
BuildRequires:  kdelibs-devel >= 3.0.0, qt-devel >= 3.0.0, openssl-devel, pcre-devel >= 3.9, arts-devel >= 1.0
Requires:       kdebase >= 3.0.0, kdelibs >= 3.0.0, qt >= 3.0.0, openssl, arts >= 1.0
BuildRoot: 	/tmp/%{name}-%{version}-root
Distribution: 	Red Hat Linux %{rh_release}
Prefix:         /usr

%description -l de
SIM - Multiprotokoll Instant Messenger

SIM (Simple Instant Messenger) ist ein Plugin-basierender
open-source Instant Messenger, der verschiedene Protokolle
(ICQ, Jabber, AIM, MSN) unterstuetzt. Dafuer wird die QT-
Bibliothek und X11 (mit optionaler KDE-Unterstuetzung)
verwendet.

SIM hat sehr viele Features, viele von diesen sind
aufgelistet unter: http://sim-icq.sourceforge.net/de/

%description
SIM - Multiprotocol Instant Messenger

SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN). It uses the QT library and works
on X11 (with optional KDE-support).

SIM has a lot of features, many of them are listed
at: http://sim-icq.sourceforge.net/

%prep
export QTDIR=/usr/lib/qt-3.1

%setup

make -f admin/Makefile.common
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure $LOCALFLAGS

%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j $numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/sim*

%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README TODO INSTALL
%{_bindir}/sim
%{_libdir}/libsimapi*
%{_libdir}/libsimui*
%{_libdir}/menu/sim-kde.menu
%{_datadir}/applnk-redhat/Internet/sim.desktop
%dir %{_datadir}/apps/sim
%{_datadir}/apps/sim/COPYING
%dir %{_datadir}/apps/sim/plugins
%{_datadir}/apps/sim/plugins/*
%dir %{_datadir}/apps/sim/pict
%{_datadir}/apps/sim/pict/*
%dir %{_datadir}/apps/sim/sounds
%{_datadir}/apps/sim/sounds/*
%{_datadir}/icons/*/*/*/*

%changelog
* Sun Nov 23 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.2-1
- Upgrade to 0.9.2

* Wed Nov 05 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.1-1
- Upgrade to 0.9.1

* Tue Oct 28 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.0-1
- Upgrade to 0.9.0
- Fixed dependencie to autoconf and corrected something in %clean
- Removed KDE menu patch

* Sun Aug 03 2003 - Robert Scheck <sim@robert-scheck.de> - 0.8.3-1
- KDE menu patch for Red Hat Linux
- Changed description to a multi-language description
- Added new BuildRequires
- Fixed a lot of things in the spec file
- Upgrade to 0.8.3 

* Wed Jul 16 2003 - Robert Scheck <sim@robert-scheck.de> - 0.8.2-2
- Dependencies fixes for arts and arts-devel

* Sat Apr 03 2003 - Robert Scheck <sim@robert-scheck.de> - 0.8.2-1
- RPM should work with Red Hat Linux 9.0
- Upgrade to 0.8.2

* Sun Jan 12 2003 - Robert Scheck <sim@robert-scheck.de> - 0.8.1-1
- RPM should work with Red Hat Linux = 7.3
- Upgrade to 0.8.1

* Tue Aug 27 2002 - Robert Scheck <sim@robert-scheck.de> - 0.7.0-1
- RPM should work with Red Hat Linux >= 7.3
- Created new spec file from nothing
- Update to 0.7.0

