%define rh_release %(rh_release="`rpm -q --queryformat='%{VERSION}' redhat-release | grep -v install 2>/dev/null`" ; if test $? != 0 ; then rh_release="0" ; fi ; echo "$rh_release")
%define fdr_release %(fdr_release="`rpm -q --queryformat='%{VERSION}' fedora-release | grep -v install 2>/dev/null`" ; if test $? != 0 ; then fdr_release="0" ; fi ; echo "$fdr_release")
%define release 1

Name: 		sim
Version: 	0.9.3
%if %{rh_release}
Release: 	%{release}.rh%(dist_release="`echo "%{rh_release} * 10" | bc 2>/dev/null`" ; echo "$dist_release")
Distribution:	Red Hat Linux %{rh_release}
%else
Release:	%{release}.fdr%(dist_release="`echo "%{rh_release} * 10" | bc 2>/dev/null`" ; echo "$dist_release")
Distribution:	Fedora Core %{fdr_release}
%endif
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
BuildRequires:  kdelibs-devel >= 3.0.0, qt-devel >= 3.0.0, openssl-devel, pcre-devel >= 3.9, arts-devel >= 1.0, sablotron-devel >= 1.0.1
Requires:       kdebase >= 3.0.0, kdelibs >= 3.0.0, qt >= 3.0.0, openssl, arts >= 1.0, sablotron >= 1.0.1
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root
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
rm -rf $RPM_BUILD_DIR/%{name}-%{version}

%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README TODO INSTALL
%{_bindir}/sim*
%{_libdir}/libsim*
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
%dir %{_datadir}/apps/sim/styles
%{_datadir}/apps/sim/styles/*
%{_datadir}/icons/*/*/*/*

%changelog
* Fri Jan 02 2004 - Robert Scheck <sim@robert-scheck.de> - 0.9.3-1
- Upgrade to 0.9.3
- Merged Red Hat spec file into Fedora spec file

* Fri Dec 26 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.2-1
- Upgrade to 0.9.2
- Added sablotron to requirements

* Wed Nov 05 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.1-1
- Upgrade to 0.9.1

* Tue Oct 28 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.0-1
- Upgrade to 0.9.0
- Adapted spec file from Red Hat Linux

