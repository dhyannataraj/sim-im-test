Name: 		sim
Version: 	0.8.2
Release: 	2.kde31.rh90
Vendor: 	Vladimir Shutoff <shutoff@mail.ru>
Packager:	Robert Scheck <sim@robert-scheck.de>
Summary:  	SIM - Simple Instant Messenger
Copyright: 	GPL
Group: 		X11/KDE/Network
URL: 		http://sim-icq.sourceforge.net/
Source0: 	http://osdn.dl.sourceforge.net/sourceforge/sim-icq/%{name}-%{version}.tar.gz
Requires:	kdebase >= 3.1, kdelibs >= 3.1, qt >= 3.1.1, openssl, arts >= 1.1
BuildRequires:	kdelibs-devel >= 3.1, qt-devel >= 3.1.1, openssl-devel, pcre-devel >= 3.9, arts-devel >= 1.1
BuildRoot: 	/tmp/%{name}-%{version}-root
Distribution: 	Red Hat Linux 9.0
Prefix:         /usr

%description
SIM - Simple Instant Messenger

SIM is a open-source ICQ client, using QT with enhanced 
features. Special support for KDE3.

Features:
- receiving and sending SMS
- server-side contact list
- receiving and the sending messages in RTF-format
- phone directory support (it is possible to specify an access 
  to the owner phone number - public or friends only)
- file transfers
- chat
- user search
- non-ICQ contacts
- support Miranda icon themes
- Floating
- Spam filter
- user encoding
- secure direct connection with SIM and Licq
- message or chat mode for user window
- On-Screen display notification about messages and user status
- keyboard shortcuts
- Message forwarding
- Sending of the messages to several users


%prep
export QTDIR=/usr/lib/qt-3.1

%setup
make -f admin/Makefile.common
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure $LOCALFLAGS

%build
export QTDIR=/usr/lib/qt-3.1
%configure

# clean up
make clean

# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

#mv $RPM_BUILD_ROOT/%{_bindir}/i386-redhat-linux-sim 
#$RPM_BUILD_ROOT/%{_bindir}/sim

%find_lang %{name}

%clean

%files -f %{name}.lang
%defattr(-, root, root, 755)
%doc AUTHORS COPYING README TODO INSTALL
%{_bindir}/sim
%{_bindir}/simctrl
/usr/lib/menu/sim-kde.menu
%{_datadir}/applnk/Internet/sim.desktop
%dir %{_datadir}/apps/sim
%{_datadir}/apps/sim/icons
%dir %{_datadir}/apps/sim/pict
%{_datadir}/apps/sim/pict/*
%dir %{_datadir}/apps/sim/sounds
%{_datadir}/apps/sim/sounds/*
%{_datadir}/icons/*/*/*/*


%changelog
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

