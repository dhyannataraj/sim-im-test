%undefine __libtoolize
%define	gccver %(gcc -dumpversion |cut -d. -f-2)
%define qtdir /usr/lib/qt3
%define distcc 0

%if %distcc
%define distcchosts "svr localhost"
%endif

Name: sim
Version: 0.8.2
Release: alt1
Serial:	1

Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (ICQ,KDE)
License: GPL
Packager: Albert R. Valiev <darkstar@altlinux.ru>

Url: http://sim-icq.sourceforge.net/

Source: sim-%version.tar.bz2
Source1: sim-kde.menu
Source2: sim-qt.menu

Patch1:  sim-makefile.patch

# Automatically added by buildreq on їдв сав 15 2003
#BuildRequires: 

BuildRequires: libqt3-devel libssl-devel libstdc++-devel pkgconfig zlib-devel
BuildRequires: libjpeg-devel liblcms libmng libpango-devel libpng-devel kdelibs-devel 
BuildRequires: libGLU libXft libart_lgpl-devel libatk-devel libexpat libgtk+2-devel
BuildRequires: XFree86-devel XFree86-libs flex fontconfig freetype2 gcc-c++ glib2-devel 

%if %distcc
BuildRequires: distcc
%endif

Requires: %name-data kdebase kdelibs-gcc_compiled = %gccver
#
%description
A simple ICQ client with v8 protocol support (2001) for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.

%package qt
Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (ICQ)
Requires: %name-data libqt3-gcc_compiled = %gccver
#
%description qt
A simple ICQ client with v8 protocol support (2001) for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.

%package data
Group: Networking/Instant messaging
Summary: SIM - data files
#
%description data
This package contains data, sounds, pictures,
lanuage translations for Sim ICQ client


%prep
%setup -q

%patch1 -p1

export PATH=./:$PATH

[ -f Makefile.dist ] && make -f Makefile.dist

%build
%if %distcc
export CXX=distc++
export DISTCC_HOSTS=%distcchosts
%else
export CXX=g++
%endif
export KDEDIR=%prefix
export QTDIR=%qtdir

#CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure $LOCALFLAGS
%configure \
    --enable-shared \
    --disable-static \
    --disable-rpath \
    --program-transform-name="" \
    --disable-kde \
    --with-xinerama
%if %distcc
%make_build CXX=$CXX -j 3
%else
%make_build CXX=$CXX
%endif
mv sim/sim sim/sim-qt


%make clean
%configure \
    --enable-shared \
    --disable-static \
    --disable-rpath \
    --program-transform-name="" \
    --with-xinerama

%if %distcc
%make_build CXX=$CXX -j 3
%else
%make_build CXX=$CXX
%endif

%install
%make DESTDIR=$RPM_BUILD_ROOT install
install -m 0755 sim/sim-qt %buildroot/%_bindir
pushd %buildroot/%_datadir/applnk/Internet
    cp %name.desktop %name-qt.desktop
    subst "s|^Name\[.*=Sim.*$||" %{name}.desktop
    subst "s|^Name\[.*=Sim.*$||" %{name}-qt.desktop
    #
    subst "s|^Name=Sim|Name=Sim (for KDE)|" %{name}.desktop
    subst "s|^Exec=sim|Exec=sim-qt|" %{name}-qt.desktop
popd

# Create LMDK menus
install -d %buildroot/%_menudir
%__install %SOURCE1 %buildroot/%_menudir/%name
%__install %SOURCE2 %buildroot/%_menudir/%{name}-qt

%find_lang %name

%post
%update_menus
%postun
%clean_menus

%post qt
%update_menus
%postun qt
%clean_menus


%files
%_bindir/%name
%attr(644,root,root) %_menudir/%name
%_datadir/applnk/Internet/%name.desktop

%files qt
%_bindir/%name-qt
%attr(644,root,root) %_menudir/%name-qt

%files data -f %name.lang
%_datadir/apps/%name
%_iconsdir/*/*/*/*.png

%changelog
* Sat Jun 14 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt1
- Release build

* Mon Apr 21 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.6
- BuildRequires fix

* Tue Apr 15 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.5
- Fixed crash at startup

* Sun Apr 13 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.4
- New cvs snapshot build
- Fixed bug #0002387

* Fri Apr 04 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.3
- Test for bug with libqt3-3.1.2

* Mon Mar 17 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.2
- Build cvs snapshot for Sisyphus
- Add Bulgarian translation
- Add Hebrew translation
- Add random chat
- Add settings for groups
- Add GKrellM2 plugin
- Add French translation

* Mon Feb 24 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.1
- Build cvs snapshot for Daedalus

* Sun Feb 02 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.1-alt1
- downgrade to sim-0.8.1

* Mon Jan 20 2003 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.5
- Build cvs snapshot (2003.01.20)

* Sat Jan 04 2003 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.4
- Build cvs snapshot (2003.01.05)

* Thu Nov 28 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.3
- Build CVS Snaphot (fixed some bugs)

* Sat Nov 16 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.2
- menu files attr corrections

* Tue Nov 12 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.1
- Build CVS snapshot (because current version incompatible with glibc)

* Sat Oct 26 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.8-alt1
- New version

* Wed Oct 9 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.7-alt3
- Spec corrections

* Wed Oct 9 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.7-alt2
- Rebuild with new KDE requirements

* Thu Sep 12 2002 Sergey V Turchin <zerg@altlinux.ru> 0.7-alt1
- initial spec
