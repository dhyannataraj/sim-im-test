%undefine __libtoolize
%define	gccver %(gcc -dumpversion |cut -d. -f-2)
%define qtdir /usr/lib/qt3

Name: sim
Version: 0.9.0
Release: alt0.7
Serial:	1

Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (ICQ,KDE)
License: GPL
Packager: Albert R. Valiev <darkstar@altlinux.ru>

Url: http://sim-icq.sourceforge.net/

Source1: sim-kde.menu

%define simsrc %_usrsrc/sim-source-%version/sim-source-%version.tar.bz2

Patch1: sim-translation.patch
Patch2: sim-makefile-fix-alt.patch

Requires: %name-data kdebase kdelibs-gcc_compiled = %gccver
Conflicts: sim-qt

%add_findprov_lib_path %_datadir/apps/%name/plugins

BuildRequires: XFree86-devel XFree86-libs flex fontconfig freetype2 gcc-c++
BuildRequires: kdelibs-devel libGLU libGLwrapper libart_lgpl-devel 
BuildRequires: libaudio libexpat-devel libjpeg-devel liblcms libmng libpng-devel 
BuildRequires: libqt3-devel libssl-devel libstdc++-devel pkgconfig zlib-devel
BuildRequires: sim-source = %version

%description
A simple ICQ client for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.
Supported protocols: ICQ v8 (2001), Jabber, MSN, AIM.

%package -n lib%name
Group: Networking/Instant messaging
Summary: SIM - libraries
Conflicts: libsim-qt

%description -n lib%name
This package contains libraries for Sim ICQ client

%package data
Group: Networking/Instant messaging
Summary: SIM - data files
Conflicts: sim-qt-data

%description data
This package contains data, sounds, pictures,
lanuage translations for Sim ICQ client

%prep
%__rm -rf sim-source-%version

echo "Extracting SIM source tarball..."

%__tar -jxvf %simsrc 2>&1>>/dev/null

%setup -D -T -n sim-source-%version

%patch1 -p1
%patch2 -p1

export PATH=./:$PATH

[ -f admin/Makefile.common ] && make -f admin/Makefile.common

%build
export CXX=g++
export KDEDIR=%prefix
export QTDIR=%qtdir

%configure \
    --enable-shared \
    --disable-static \
    --disable-rpath \
    --program-transform-name="" \
    --with-qt-dir=%qtdir \
    --with-xinerama

%make_build CXX=$CXX

%install
%make DESTDIR=$RPM_BUILD_ROOT install

install -d %buildroot/%_menudir
%__install %SOURCE1 %buildroot/%_menudir/%name

%find_lang %name

%post
%update_menus
%postun
%clean_menus

%files
%_bindir/%name
%attr(644,root,root) %_menudir/%name
%_datadir/applnk/Internet/%name.desktop

%files -n lib%name
%_libdir/*.so*
%_libdir/*.la

%files data -f %name.lang
%_datadir/apps/%name
%_iconsdir/*/*/*/*.png

%changelog
* Fri Sep 19 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.7
- Fixed depends

* Thu Sep 18 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.6
- Fixed buildrequires
- Enabled Jabber protocol build

* Thu Sep 18 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.5
- Build from CVS 20030918
- translation fixes
- Makefile fixes

* Sun Sep 14 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.4
- Build from CVS 20030914

* Tue Sep 09 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.3
- Build from CVS 20030909

* Wed Sep 03 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.2
- Build from CVS 20030903

* Fri Aug 29 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.1
- Build from CVS 20030828

* Tue Aug 12 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.3-alt1
- 0.8.3 Release build

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
