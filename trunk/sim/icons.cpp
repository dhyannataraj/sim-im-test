/***************************************************************************
                          icons.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "icons.h"
#include "log.h"

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

#include <map>
#include <string>
using namespace std;

#ifdef USE_KDE
#include <kiconloader.h>
#endif

#include <qmime.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qiconset.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qstringlist.h>

#include "xpm/empty.xpm"
#include "xpm/licq.xpm"
#include "xpm/licq_big.xpm"
#include "xpm/away.xpm"
#include "xpm/na.xpm"
#include "xpm/dnd.xpm"
#include "xpm/occupied.xpm"
#include "xpm/ffc.xpm"
#include "xpm/invisible.xpm"
#include "xpm/message.xpm"
#include "xpm/url.xpm"
#include "xpm/sms.xpm"
#include "xpm/file.xpm"
#include "xpm/auth.xpm"
#include "xpm/contacts.xpm"
#include "xpm/added.xpm"
#include "xpm/web.xpm"
#include "xpm/pager.xpm"
#include "xpm/chat.xpm"
#include "xpm/grp_on.xpm"
#include "xpm/grp_off.xpm"
#include "xpm/online_on.xpm"
#include "xpm/online_off.xpm"
#include "xpm/expanded.xpm"
#include "xpm/collapsed.xpm"
#include "xpm/grp_expand.xpm"
#include "xpm/grp_collapse.xpm"
#include "xpm/grp_create.xpm"
#include "xpm/grp_rename.xpm"
#include "xpm/floating.xpm"
#include "xpm/cell.xpm"
#include "xpm/phone.xpm"
#include "xpm/nophone.xpm"
#include "xpm/fax.xpm"
#include "xpm/wpager.xpm"
#include "xpm/birthday.xpm"
#include "xpm/info.xpm"
#include "xpm/history.xpm"
#include "xpm/useronline.xpm"
#include "xpm/useroffline.xpm"
#include "xpm/userunknown.xpm"
#include "xpm/downarrow.xpm"
#include "xpm/spellcheck.xpm"
#include "xpm/exit.xpm"
#include "xpm/editclear.xpm"
#include "xpm/editdelete.xpm"
#include "xpm/editcopy.xpm"
#include "xpm/mail_reply.xpm"
#include "xpm/mail_generic.xpm"
#include "xpm/smile0.xpm"
#include "xpm/smile1.xpm"
#include "xpm/smile2.xpm"
#include "xpm/smile3.xpm"
#include "xpm/smile4.xpm"
#include "xpm/smile5.xpm"
#include "xpm/smile6.xpm"
#include "xpm/smile7.xpm"
#include "xpm/smile8.xpm"
#include "xpm/smile9.xpm"
#include "xpm/smileA.xpm"
#include "xpm/smileB.xpm"
#include "xpm/smileC.xpm"
#include "xpm/smileD.xpm"
#include "xpm/smileE.xpm"
#include "xpm/smileF.xpm"
#include "xpm/fgcolor.xpm"
#include "xpm/bgcolor.xpm"
#include "xpm/main.xpm"
#include "xpm/home.xpm"
#include "xpm/work.xpm"
#include "xpm/more.xpm"
#include "xpm/find.xpm"
#include "xpm/security.xpm"
#include "xpm/webaware.xpm"
#include "xpm/password.xpm"
#include "xpm/visiblelist.xpm"
#include "xpm/invisiblelist.xpm"
#include "xpm/ignorelist.xpm"
#include "xpm/configure.xpm"
#include "xpm/status.xpm"
#include "xpm/style.xpm"
#include "xpm/1downarrow.xpm"
#include "xpm/1uparrow.xpm"
#include "xpm/1rightarrow.xpm"
#include "xpm/1leftarrow.xpm"
#include "xpm/fileopen.xpm"
#include "xpm/sound.xpm"
#include "xpm/misc.xpm"
#include "xpm/text_bold.xpm"
#include "xpm/text_italic.xpm"
#include "xpm/text_under.xpm"
#include "xpm/text.xpm"
#include "xpm/interest.xpm"
#include "xpm/alert.xpm"
#include "xpm/past.xpm"
#include "xpm/remove.xpm"
#include "xpm/nonim.xpm"
#include "xpm/network.xpm"
#include "xpm/save_all.xpm"
#include "xpm/about_kde.xpm"
#include "xpm/screen.xpm"
#include "xpm/encrypted.xpm"
#include "xpm/key_bindings.xpm"

class str
{
public:
    str(const char *str) : data(str) {}
    bool operator < (const str&) const;
private:
    const char *data;
};

bool str::operator < (const str &str) const
{
    return strcmp(data, str.data) < 0;
}

typedef map<str, QIconSet> PIXMAP_MAP;

class Icons
{
public:
    Icons();
    QIconSet &getIcon(const char*);
    void setIcon(const char*, const QIconSet&);
    void init(const char *name);
protected:
    QIconSet *dummy;
    PIXMAP_MAP icons;
    void addIcon(const char *name, const char **xpm, const char **bigXpm, bool isSystem,
                 QIconSet *bgIcon, const QPixmap &dllIcon);
};

// ____________________________________________________________________________________________

#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned int

  #define EMAGIC	  0x5A4D	  /* Old magic number */
  #define ENEWEXE	  sizeof(DOSHdr)
/* Value of E_LFARLC for new .EXEs */
  #define ENEWHDR	  0x003C	  /* Offset in old hdr. of ptr. to new */
  #define ERESWDS	  0x0010	  /* No. of reserved words (OLD) */
  #define ERES1WDS	  0x0004	  /* No. of reserved words in e_res */
  #define ERES2WDS	  0x000A	  /* No. of reserved words in e_res2 */
  #define ECP		  0x0004	  /* Offset in struct of E_CP */
  #define ECBLP 	  0x0002	  /* Offset in struct of E_CBLP */
  #define EMINALLOC	  0x000A	  /* Offset in struct of E_MINALLOC */

typedef struct exe_hdr			  /* DOS 1, 2, 3 .EXE header */
{
    U16	  e_magic;	  /* 00 Magic number */
    U16	  e_cblp;	  /* 02 Bytes on last page of file */
    U16	  e_cp; 	  /* 04 Pages in file */
    U16	  e_crlc;	  /* 06 Relocations */
    U16	  e_cparhdr;	  /* 08 Size of header in paragraphs */
    U16	  e_minalloc;	  /* 0A Minimum extra paragraphs needed */
    U16	  e_maxalloc;	  /* 0C Maximum extra paragraphs needed */
    U16	  e_ss; 	  /* 0E Initial (relative) SS value */
    U16	  e_sp; 	  /* 10 Initial SP value */
    U16	  e_csum;	  /* 12 Checksum */
    U16	  e_ip; 	  /* 14 Initial IP value */
    U16	  e_cs; 	  /* 16 Initial (relative) CS value */
    U16	  e_lfarlc;	  /* 18 File address of relocation table */
    U16	  e_ovno;	  /* 1A Overlay number */
    U16	  e_res[ERES1WDS];/* 1C Reserved words */
    U16	  e_oemid;	  /* 24 OEM identifier (for e_oeminfo) */
    U16	  e_oeminfo;	  /* 26 OEM information; e_oemid specific */
    U16	  e_res2[ERES2WDS];/*28 Reserved words */
    U32     e_lfanew;	  /* 3C File address of new exe header */
} DOSHdr;					  /* 40h size of structure */

  #define NEMAGIC	  0x454E	  /* New magic number */
  #define NERESWORDS	  3		  /* 6 bytes reserved */
  #define NECRC 	  8		  /* Offset into new header of NE_CRC */

typedef struct new_exe			  /* New .EXE header */
{
    U16	  ne_magic;	  /* 00 Magic number NE_MAGIC */
    U8	  ne_ver;	  /* 02 Linker Version number */
    U8	  ne_rev;	  /* 03 Linker Revision number */
    U16	  ne_enttab;	  /* 04 Offset of Entry Table */
    U16	  ne_cbenttab;	  /* 06 Number of bytes in Entry Table */
    U32		  ne_crc;	  /* 08 Checksum of whole file */
    U16	  ne_flags;	  /* 0C Flag word */
    U16	  ne_autodata;	  /* 0E Automatic data segment number */
    U16	  ne_heap;	  /* 10 Initial heap allocation */
    U16	  ne_stack;	  /* 12 Initial stack allocation */
    U32		  ne_csip;	  /* 14 Initial CS:IP setting */
    U32		  ne_sssp;	  /* 18 Initial SS:SP setting */
    U16	  ne_cseg;	  /* 1C Count of file segments */
    U16	  ne_cmod;	  /* 1E Entries in Module Reference Table */
    U16	  ne_cbnrestab;   /* 20 Size of non-resident name table */
    U16	  ne_segtab;	  /* 22 Offset of Segment Table */
    U16	  ne_rsrctab;	  /* 24 Offset of Resource Table */
    U16	  ne_restab;	  /* 26 Offset of resident name table */
    U16	  ne_modtab;	  /* 28 Offset of Module Reference Table */
    U16	  ne_imptab;	  /* 2A Offset of Imported Names Table */
    U32		  ne_nrestab;	  /* 2C Offset of Non-resident Names Table */
    U16	  ne_cmovent;	  /* 30 Count of movable entries */
    U16	  ne_align;	  /* 32 Segment alignment shift count */
    U16	  ne_cres;	  /* 34 Count of resource entries */
    U8	  ne_exetyp;	  /* 36 Target operating system */
    U8	  ne_addflags;	  /* 37 Additional flags */
    U16	  ne_res[NERESWORDS]; /* 38 3 reserved words */
    U8	  ne_sdkrev;	  /* 3E Windows SDK revison number */
    U8	  ne_sdkver;	  /* 3F Windows SDK version number */
} NEHdr;

typedef struct _PEHdr
{
    U32 signaturebytes;
    U16 Machine;
    U16 NumberOfSections;
    U32 TimeDateStamp;
    U32 PointerToSymbolTable;
    U32 NumberOfSymbols;
    U16 SizeOfOptionalHeader;
    U16 Characteristics;
} PEHdr;

typedef struct _DATA_DIRECTORY
{
    U32 VirtualAddress;
    U32 Size;
} DATA_DIRECTORY;

class PEOptHdr
{
public:
    PEOptHdr() : DataDirectory(NULL) {}
    ~PEOptHdr() { if (DataDirectory) delete[] DataDirectory; }
    /*
        Standard fields.
       */
    U16  Magic;
    U8   MajorLinkerVersion;
    U8   MinorLinkerVersion;
    U32   SizeOfCode;
    U32   SizeOfInitializedData;
    U32   SizeOfUninitializedData;
    U32   AddressOfEntryPoint;
    U32   BaseOfCode;
    U32   BaseOfData;
    /*
     NT additional fields.
    */
    U32   ImageBase;
    U32   SectionAlignment;
    U32   FileAlignment;
    U16  MajorOperatingSystemVersion;
    U16  MinorOperatingSystemVersion;
    U16  MajorImageVersion;
    U16  MinorImageVersion;
    U16  MajorSubsystemVersion;
    U16  MinorSubsystemVersion;
    U32   Reserved1;
    U32   SizeOfImage;
    U32   SizeOfHeaders;
    U32   CheckSum;
    U16  Subsystem;
    U16  DllCharacteristics;
    U32   SizeOfStackReserve;
    U32   SizeOfStackCommit;
    U32   SizeOfHeapReserve;
    U32   SizeOfHeapCommit;
    U32   LoaderFlags;
    U32   NumberOfRvaAndSizes;
    DATA_DIRECTORY *DataDirectory;
};

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _PESectHdr
{
    U8 Name[IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        U32 PhysicalAddress;
        U32 VirtualSize;
    } Misc;
    U32 VirtualAddress;
    U32 SizeOfRawData;
    U32 PointerToRawData;
    U32 PointerToRelocations;
    U32 PointerToLinenumbers;
    U16 NumberOfRelocations;
    U16 NumberOfLinenumbers;
    U32 Characteristics;
} PESectHdr;

/* Directory Entries */

/* Export Directory */
#define IMAGE_DIRECTORY_ENTRY_EXPORT         0
/* Import Directory */
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1
/* Resource Directory */
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2
/* Exception Directory */
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3
/* Security Directory */
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4
/* Base Relocation Table */
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5
/* Debug Directory */
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6
/* Description String */
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7
/* Machine Value (MIPS GP) */
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8
/* TLS Directory */
#define IMAGE_DIRECTORY_ENTRY_TLS            9
/* Load Configuration Directory */
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10

#define IMAGE_RESOURCE_DATA_IS_DIRECTORY 0x80000000L
#define IMAGE_RESOURCE_NAME_IS_STRING	0x80000000L

#define RT_CURSOR   	1
#define RT_BITMAP   	2
#define RT_ICON 		3
#define RT_MENU 		4
#define RT_DIALOG   	5
#define RT_STRING 		6
#define RT_FONTDIR  	7
#define RT_FONT 		8
#define RT_ACCELERATOR  9
#define RT_RCDATA 		10
#define RT_MESSAGELIST 	11
#define RT_GROUP_CURSOR 12

#define RT_GROUP_ICON   14

#define RT_VERSION		16
#define RT_DLGINCLUDE	17
#define RT_PLUGPLAY 	19
#define RT_VXD 			20
#define RT_ANICURSOR 	21

#define RT_NEWRESOURCE  0x2000
#define RT_NEWBITMAP    (RT_BITMAP|RT_NEWRESOURCE)
#define RT_NEWMENU      (RT_MENU|RT_NEWRESOURCE)
#define RT_NEWDIALOG    (RT_DIALOG|RT_NEWRESOURCE)
#define RT_ERROR        0x7fff

typedef struct _ResourceDirectory
{
    U32 Characteristics;
    U32 TimeDateStamp;
    U16 MajorVersion;
    U16 MinorVersion;
    U16 NumberOfNamedEntries;
    U16 NumberOfIdEntries;
} ResourceDirectory;

typedef struct _ResourceDirectoryEntry
{
    U32 Name;
    U32 OffsetToData;
} ResourceDirectoryEntry;

typedef struct _ResourceDataEntry
{
    U32 OffsetToData;
    U32 Size;
    U32 CodePage;
    U32 Reserved;
} ResourceDataEntry;

typedef map<int, int> INT_MAP;

class IconsDLL
{
public:
    IconsDLL(const char *name);
    ~IconsDLL();

    QPixmap getIcon(int id);

protected:
    INT_MAP icons;
    INT_MAP groups;
    void addIcon(int id, int offs);
    void addGroup(int id, int icon);

    QFile f;
    DOSHdr dh;
    NEHdr nh;
    PEHdr ph;
    PEOptHdr oh;
    void getDOSHeader();
    void getNEHeader();
    void getPEHeader();
    void getPEOptHeader();
    void getPESectHeader(PESectHdr*);
    void getDataDirectory(DATA_DIRECTORY*);
    void getResourceDirectory(ResourceDirectory*);
    void getResourceDirectoryEntry(ResourceDirectoryEntry*);
    void getResourceDataEntry(ResourceDataEntry*);
    void doResourceDir(U32 origoffset, int level,U32 resourceType, int id);
    U32 virtualToReal (U32 VAImageDir,PESectHdr *psh);
    U32 imageDirectoryOffset();
    U8  read_8ubit();
    U16 read_16ubit();
    U32 read_32ubit();
    PESectHdr *section;
};

// _____________________________________________________________________________________________

static QPixmap swapRG(const QPixmap &p)
{
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        int r = qRed(data[i]);
        int g = qGreen(data[i]);
        int b = qBlue(data[i]);
        int a = qAlpha(data[i]);
        data[i] = qRgba(g, r, b, a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QPixmap addPict(const QPixmap &pict, const QPixmap &add)
{
    QRegion empty;
    QPixmap res = pict;
    QBitmap mask;
    if (pict.mask() && add.mask()){
        mask = QBitmap(pict.width(), pict.height());
        QPainter p(&mask);
        p.fillRect(0, 0, pict.width(), pict.height(),
#ifdef WIN32
                   QColor(255, 255, 255)
#else
                   QColor(0, 0, 0)
#endif
                  );
        QRegion r(*add.mask());
        r.translate((pict.width() - add.width()) / 2, (pict.height() - add.height()) / 2);
        r += QRegion(*pict.mask());
        p.setClipRegion(r);
        p.fillRect(0, 0, pict.width(), pict.height(),
#ifdef WIN32
                   QColor(0, 0, 0)
#else
                   QColor(255, 255, 255)
#endif
                  );
        p.end();
    }
    QPainter p;
    p.begin(&res);
    p.drawPixmap((pict.width() - add.width()) / 2, (pict.height() - add.height()) / 2, add);
    p.end();
    if (!mask.isNull())
        res.setMask(mask);
    return res;
}

void Icons::addIcon(const char *name, const char **xpm, const char **bigXpm, bool isSystem,
                    QIconSet *bgIcon, const QPixmap &dllIcon)
{
    QIconSet pict;
    if (!dllIcon.isNull()){
        pict = QIconSet(dllIcon);
    }else{
#ifdef USE_KDE
#if QT_VERSION < 300
        if (bgIcon) isSystem = false;
#endif
        if (isSystem){
            pict = SmallIconSet(name);
            if (bigXpm){
                KIconLoader iconLoader;
                QStringList l = iconLoader.queryIcons(-32);
                QStringList::Iterator it;
                for (it = l.begin(); it != l.end(); ++it)
                    if (*it == name) break;
                QPixmap bigPict = QPixmap(bigXpm);
                QPixmap bigPictActive = QPixmap(bigXpm);
                if (it == l.end()){
                    QIconSet bigIcon = DesktopIconSet(name);
                    bigPict = bigIcon.pixmap(QIconSet::Large, QIconSet::Normal);
                    bigPictActive = bigIcon.pixmap(QIconSet::Large, QIconSet::Active);
                }
                pict.setPixmap(bigPict, QIconSet::Large);
                pict.setPixmap(bigPictActive, QIconSet::Large, QIconSet::Active);
            }
        }
#else
        isSystem = false;
#endif
        if (bgIcon){
            isSystem = true;
            QPixmap smallPict = bgIcon->pixmap(QIconSet::Small, QIconSet::Normal);
            QPixmap bigPict = bgIcon->pixmap(QIconSet::Large, QIconSet::Normal);
            QPixmap bigPictActive = bgIcon->pixmap(QIconSet::Large, QIconSet::Active);
            QPixmap add(xpm);
            smallPict = addPict(smallPict, add);
            bigPict = addPict(bigPict, add);
            bigPictActive = addPict(bigPictActive, add);
            pict = QIconSet(smallPict);
            pict.setPixmap(bigPict, QIconSet::Large);
            pict.setPixmap(bigPictActive, QIconSet::Large, QIconSet::Active);
        }
        if (!isSystem){
            pict = QPixmap(xpm);
            if (bigXpm)
                pict.setPixmap(QPixmap(bigXpm), QIconSet::Large);
        }
    }
    setIcon(name, pict);
    QString url("icon:");
    url += name;
    QMimeSourceFactory::defaultFactory()->setPixmap(url, pict.pixmap(QIconSet::Small, QIconSet::Normal));
}

#define ICON(A, id)	addIcon(#A, A, NULL, false, NULL, dll.getIcon(id));
#define KICON(A, id) addIcon(#A, p_##A, NULL, true, NULL, dll.getIcon(id));
#define KICON2(A, id) addIcon(#A, p_##A, b_##A, true, NULL, dll.getIcon(id));
#define SICON(A, id) addIcon(#A, A, NULL, false, &pict, dll.getIcon(id));

Icons::Icons()
{
    dummy = NULL;
}

void Icons::init(const char *name)
{
    IconsDLL dll(name);
    QPixmap::setDefaultOptimization( QPixmap::MemoryOptim );
    ICON(empty, 200)
    KICON2(licq, 102)
    ICON(message, 136)
    ICON(url, 135)
    ICON(sms, 103)
    ICON(file, 207)
    ICON(auth, 10001)
    ICON(contacts, 10002)
    ICON(added, 210)
    ICON(web, 10003)
    ICON(pager, 10004)
    ICON(chat, 10005)
    ICON(grp_on, 10006)
    ICON(grp_off, 10007)
    ICON(online_on, 10008)
    ICON(online_off, 10009)
    ICON(collapsed, 10010)
    ICON(expanded, 10011)
    ICON(grp_collapse, 10012)
    ICON(grp_expand, 10013)
    ICON(grp_create, 10014)
    ICON(grp_rename, 173)
    ICON(floating, 10015)
    ICON(cell, 10016)
    ICON(phone, 10017)
    ICON(nophone, 10018)
    ICON(fax, 10019)
    ICON(wpager, 10020)
    ICON(birthday, 10021)
    ICON(info, 160)
    KICON(history, 174)
    ICON(useronline, 10022)
    ICON(useroffline, 10023)
    ICON(userunknown, 10024)
    KICON(2downarrow, 10025)
    KICON(spellcheck, 10026)
    KICON(exit, 10027)
    KICON(editclear, 10028)
    KICON(editdelete, 175)
    KICON(editcopy, 10029)
    KICON(mail_reply, 10030)
    KICON(mail_generic, 193)
    ICON(smile0, 10031)
    ICON(smile1, 10032)
    ICON(smile2, 10032)
    ICON(smile3, 10033)
    ICON(smile4, 10034)
    ICON(smile5, 10035)
    ICON(smile6, 10036)
    ICON(smile7, 10037)
    ICON(smile8, 10038)
    ICON(smile9, 10039)
    ICON(smileA, 10040)
    ICON(smileB, 10041)
    ICON(smileC, 10042)
    ICON(smileD, 10043)
    ICON(smileE, 10044)
    ICON(smileF, 10045)
    ICON(bgcolor, 10046)
    ICON(fgcolor, 10047)
    ICON(main, 10048)
    ICON(home, 10049)
    ICON(work, 10050)
    ICON(more, 10051)
    KICON(find, 161)
    ICON(security, 10052)
    ICON(password, 10053)
    ICON(webaware, 10054)
    ICON(visiblelist, 10055)
    ICON(invisiblelist, 10056)
    ICON(ignorelist, 10057)
    KICON(configure, 163)
    ICON(status, 10058)
    ICON(style, 10059)
    KICON(1uparrow, 10060)
    KICON(1downarrow, 10061)
    KICON(1rightarrow, 10062)
    KICON(1leftarrow, 10077)
    KICON(fileopen, 10063)
    ICON(sound, 10064)
    ICON(misc, 10065)
    KICON(text_bold, 10066)
    KICON(text_italic, 10067)
    KICON(text_under, 10068)
    KICON(text, 10069)
    ICON(interest, 10070)
    ICON(alert, 10071)
    ICON(past, 10072)
    KICON(remove, 175)
    ICON(nonim, 10073)
    ICON(network, 10074)
    KICON(save_all, 10075)
    KICON(about_kde, 10076)
    ICON(screen, 10078)
    KICON(encrypted, 10079)
    KICON(key_bindings, 10080)

    QIconSet pict = getIcon("licq");
    QPixmap dllPict = dll.getIcon(104);
    if (!dllPict.isNull()) pict = QIconSet(dllPict);
    setIcon("online", pict);
    dllPict = dll.getIcon(105);
    QIconSet pictOffline;
    if (dllPict.isNull()){
        QPixmap smallPict = pict.pixmap(QIconSet::Small, QIconSet::Normal);
        QPixmap bigPict = pict.pixmap(QIconSet::Large, QIconSet::Normal);
        QPixmap bigPictActive = pict.pixmap(QIconSet::Large, QIconSet::Active);
        smallPict = swapRG(smallPict);
        bigPict = swapRG(bigPict);
        bigPictActive = swapRG(bigPictActive);
        pictOffline = QIconSet(smallPict);
        pictOffline.setPixmap(bigPict, QIconSet::Large);
        pictOffline.setPixmap(bigPictActive, QIconSet::Large, QIconSet::Active);
    }else{
        pictOffline = QIconSet(dllPict);
    }
    setIcon("offline", pictOffline);
    SICON(away, 128);
    SICON(na, 131)
    SICON(dnd, 158)
    SICON(occupied, 159)
    SICON(ffc, 129)
    SICON(invisible, 130)
    QPixmap::setDefaultOptimization( QPixmap::NormalOptim );
}

QIconSet &Icons::getIcon(const char *name)
{
    if (name == NULL) return getIcon("empty");
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        log(L_WARN, "Icon %s not found, use empty", name);
        it = icons.find("empty");
    }
    if (it == icons.end()){
        log(L_WARN, "Empty icon not found, use dummy");
        if (dummy == NULL) dummy = new QIconSet();
        return *dummy;
    }
    return (*it).second;
}

void Icons::setIcon(const char *name, const QIconSet &icon)
{
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        icons.insert(PIXMAP_MAP::value_type(name, icon));
    }else{
        (*it).second = icon;
    }
}

const char *app_file(const char *f);

IconsDLL::IconsDLL(const char *name)
{
    section = NULL;
    if (*name == 0) return;
    string s = "icons";
#ifdef WIN32
    s += "\\";
#else
    s += "/";
#endif
    s += name;
    s += ".dll";
    f.setName(QString::fromLocal8Bit(app_file(s.c_str())));
    if (!f.open(IO_Raw | IO_ReadOnly)) return;
    getDOSHeader();
    f.at(dh.e_lfanew);
    getNEHeader();
    if (nh.ne_magic == NEMAGIC)
    {
        if ( (nh.ne_restab - nh.ne_rsrctab) == 0)
        {
            log(L_WARN, "No resources");
            return;
        }

        if((nh.ne_rsrctab - nh.ne_segtab)%8!=0)
            log(L_WARN, "Extra 4 bytes in segment table.");


        f.at(nh.ne_rsrctab+dh.e_lfanew);
        //		getResourceTable();
    }
    else
    {
        f.at(dh.e_lfanew);
        getPEHeader();
        if (ph.signaturebytes != 0x00004550)
        {
            log(L_WARN, "file was neither a ne or pe file");
            return;
        }
        getPEOptHeader();
        section = new PESectHdr[ph.NumberOfSections];
        for (unsigned i=0;i<ph.NumberOfSections;i++)
            getPESectHeader(&section[i]);
        U32 offs = imageDirectoryOffset();
        f.at(offs);
        doResourceDir(offs, 0,0,0);
    }
}

IconsDLL::~IconsDLL()
{
    if (section) delete[] section;
}

void IconsDLL::addIcon(int id, int offs)
{
    INT_MAP::iterator it = icons.find(id);
    if (it == icons.end()){
        icons.insert(INT_MAP::value_type(id, offs));
    }else{
        (*it).second = offs;
    }
}

void IconsDLL::addGroup(int id, int grp)
{
    INT_MAP::iterator it = groups.find(id);
    if (it == groups.end()){
        groups.insert(INT_MAP::value_type(id, grp));
    }else{
        (*it).second = grp;
    }
}

typedef struct tagBITMAPINFOHEADER
{
    U32  biSize;
    U32  biWidth;
    U32  biHeight;
    U16   biPlanes;
    U16   biBitCount;
    U32  biCompression;
    U32  biSizeImage;
    U32  biXPelsPerMeter;
    U32  biYPelsPerMeter;
    U32  biClrUsed;
    U32  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    U8    rgbBlue;
    U8    rgbGreen;
    U8    rgbRed;
    U8    rgbReserved;
} RGBQUAD;

QPixmap IconsDLL::getIcon(int id)
{
    if (id == 0) return QPixmap();
    INT_MAP::iterator it = groups.find(id);
    if (it == groups.end())
        return QPixmap();
    it = icons.find((*it).second);
    if (it == groups.end())
        return QPixmap();
    BITMAPINFOHEADER bh;
    f.at((*it).second);
    f.readBlock((char*)&bh, sizeof(bh));
    int w = bh.biWidth;
    int h = bh.biHeight / 2;

    if ((w > 22) || (h > 22)) return QPixmap();

    int bits = bh.biBitCount;
    int depth = bits;
    int numColors = bh.biClrUsed;
    if (bits == 8){
        if (numColors == 0) numColors = 256;
    }else if (bits == 24){
        depth = 32;
    }
    QImage img(w, h, depth, numColors, QImage::BigEndian);
    if ((bits == 2) || (bits == 4)){
        QRgb *p = img.colorTable();
        for (int i = 0; i < numColors; i++){
            RGBQUAD rgb;
            f.readBlock((char*)&rgb, sizeof(rgb));
            (*p++) = (rgb.rgbRed << 16) | (rgb.rgbGreen << 8) | rgb.rgbBlue;
        }
    }else if (bits == 8){
        QRgb *p = img.colorTable();
        for (int i = 0; i < numColors; i++){
            RGBQUAD rgb;
            f.readBlock((char*)&rgb, sizeof(rgb));
            (*p++) = (rgb.rgbRed << 16) | (rgb.rgbGreen << 8) | rgb.rgbBlue;
        }
    }else if (bits == 32){
        img.setAlphaBuffer(true);
    }
    unsigned lineBytes = ((w * bits) + 7) >> 3;
    int i;
    for (i = 0; i < h; i++){
        uchar *data = img.scanLine(h - i - 1);
        f.readBlock((char*)data, lineBytes);
        if (bits == 24){
            for (int j = (lineBytes / 3) - 1; j >= 0; j--){
                memmove(data + (j * 4 + 1), data + (j * 3), 3);
                data[j*4] = 0;
            }
        }
    }
    QPixmap res;
    res.convertFromImage(img);

    if (!img.hasAlphaBuffer()){
        QImage imgMask(w, h, 1, 0, QImage::BigEndian);
        lineBytes = (w + 7) >> 3;
        unsigned fileBytes = ((w + 31) >> 3) & (~3);
        for (i = 0; i < h; i++){
            uchar *data = imgMask.scanLine(h - i -1);
            f.readBlock((char*)data, lineBytes);
#ifndef WIN32
            unsigned j = 0;
            for (j = 0; j < lineBytes; j++){
                data[j] ^= 0xFF;
            }
#endif
            if (fileBytes > lineBytes) f.at(f.at() + fileBytes - lineBytes);
        }
        QBitmap mask;
        mask.convertFromImage(imgMask);
        res.setMask(mask);
    }
    return res;
}

U8 IconsDLL::read_8ubit()
{
    U8 r;
    f.readBlock((char*)&r, sizeof(r));
    return r;
}

U16 IconsDLL::read_16ubit()
{
    U16 r;
    f.readBlock((char*)&r, sizeof(r));
    return r;
}

U32 IconsDLL::read_32ubit()
{
    U32 r;
    f.readBlock((char*)&r, sizeof(r));
    return r;
}

void IconsDLL::getDOSHeader()
{
    int i;
    dh.e_magic = read_16ubit();
    dh.e_cblp = read_16ubit();
    dh.e_cp = read_16ubit();
    dh.e_crlc = read_16ubit();
    dh.e_cparhdr = read_16ubit();
    dh.e_minalloc = read_16ubit();
    dh.e_maxalloc = read_16ubit();
    dh.e_ss = read_16ubit();
    dh.e_sp = read_16ubit();
    dh.e_csum = read_16ubit();
    dh.e_ip = read_16ubit();
    dh.e_cs = read_16ubit();
    dh.e_lfarlc = read_16ubit();
    dh.e_ovno = read_16ubit();
    for(i=0;i<ERES1WDS;i++)
        dh.e_res[i] = read_16ubit();
    dh.e_oemid = read_16ubit();
    dh.e_oeminfo = read_16ubit();
    for (i=0;i<ERES2WDS;i++)
        dh.e_res2[i] = read_16ubit();
    dh.e_lfanew = read_32ubit();
}

void IconsDLL::getNEHeader()
{
    int i;
    nh.ne_magic = read_16ubit();
    nh.ne_ver = read_8ubit();
    nh.ne_rev = read_8ubit();
    nh.ne_enttab = read_16ubit();
    nh.ne_cbenttab = read_16ubit();
    nh.ne_crc = read_32ubit();
    nh.ne_flags = read_16ubit();
    nh.ne_autodata = read_16ubit();
    nh.ne_heap = read_16ubit();
    nh.ne_stack = read_16ubit();
    nh.ne_csip = read_32ubit();
    nh.ne_sssp = read_32ubit();
    nh.ne_cseg = read_16ubit();
    nh.ne_cmod = read_16ubit();
    nh.ne_cbnrestab = read_16ubit();
    nh.ne_segtab = read_16ubit();
    nh.ne_rsrctab = read_16ubit();
    nh.ne_restab = read_16ubit();
    nh.ne_modtab = read_16ubit();
    nh.ne_imptab = read_16ubit();
    nh.ne_nrestab = read_32ubit();
    nh.ne_cmovent = read_16ubit();
    nh.ne_align = read_16ubit();
    nh.ne_cres = read_16ubit();
    nh.ne_exetyp = read_8ubit();
    nh.ne_addflags = read_8ubit();
    for (i=0;i<NERESWORDS;i++)
        nh.ne_res[i] = read_16ubit();
    nh.ne_sdkrev = read_8ubit();
    nh.ne_sdkver = read_8ubit();
}

void IconsDLL::getPEHeader()
{
    ph.signaturebytes = read_32ubit();
    ph.Machine = read_16ubit();
    ph.NumberOfSections = read_16ubit();
    ph.TimeDateStamp = read_32ubit();
    ph.PointerToSymbolTable = read_32ubit();
    ph.NumberOfSymbols = read_32ubit();
    ph.SizeOfOptionalHeader = read_16ubit();
    ph.Characteristics = read_16ubit();
}

void IconsDLL::getPEOptHeader()
{
    U32 i;
    oh.Magic = read_16ubit();
    oh.MajorLinkerVersion = read_8ubit();
    oh.MinorLinkerVersion = read_8ubit();
    oh.SizeOfCode = read_32ubit();
    oh.SizeOfInitializedData = read_32ubit();
    oh.SizeOfUninitializedData = read_32ubit();
    oh.AddressOfEntryPoint = read_32ubit();
    oh.BaseOfCode = read_32ubit();
    oh.BaseOfData = read_32ubit();

    /* extra NT stuff */
    oh.ImageBase = read_32ubit();
    oh.SectionAlignment = read_32ubit();
    oh.FileAlignment = read_32ubit();
    oh.MajorOperatingSystemVersion = read_16ubit();
    oh.MinorOperatingSystemVersion = read_16ubit();
    oh.MajorImageVersion = read_16ubit();
    oh.MinorImageVersion = read_16ubit();
    oh.MajorSubsystemVersion = read_16ubit();
    oh.MinorSubsystemVersion = read_16ubit();
    oh.Reserved1 = read_32ubit();
    oh.SizeOfImage = read_32ubit();
    oh.SizeOfHeaders = read_32ubit();
    oh.CheckSum = read_32ubit();
    oh.Subsystem = read_16ubit();
    oh.DllCharacteristics = read_16ubit();
    oh.SizeOfStackReserve = read_32ubit();
    oh.SizeOfStackCommit = read_32ubit();
    oh.SizeOfHeapReserve = read_32ubit();
    oh.SizeOfHeapCommit = read_32ubit();
    oh.LoaderFlags = read_32ubit();
    oh.NumberOfRvaAndSizes = read_32ubit();
    oh.DataDirectory = new DATA_DIRECTORY[oh.NumberOfRvaAndSizes];
    for (i=0;i<oh.NumberOfRvaAndSizes;i++)
        getDataDirectory(&oh.DataDirectory[i]);
}

void IconsDLL::getDataDirectory(DATA_DIRECTORY *id)
{
    id->VirtualAddress = read_32ubit();
    id->Size = read_32ubit();
}

void IconsDLL::getPESectHeader(PESectHdr *h)
{
    int i;
    for (i=0;i<IMAGE_SIZEOF_SHORT_NAME;i++)
        h->Name[i] = read_8ubit();
    h->Misc.VirtualSize=read_32ubit();
    h->VirtualAddress=read_32ubit();
    h->SizeOfRawData=read_32ubit();
    h->PointerToRawData=read_32ubit();
    h->PointerToRelocations=read_32ubit();
    h->PointerToLinenumbers=read_32ubit();
    h->NumberOfRelocations=read_16ubit();
    h->NumberOfLinenumbers=read_16ubit();
    h->Characteristics=read_32ubit();
}

void IconsDLL::doResourceDir(U32 origoffset, int level, U32 resourceType, int res_id)
{
    ResourceDirectory id;
    getResourceDirectory(&id);

    for (int i=0; i<id.NumberOfNamedEntries + id.NumberOfIdEntries; i++)
    {
        ResourceDirectoryEntry ie;
        getResourceDirectoryEntry(&ie);
        unsigned pos = f.at();

        U32 offset = ie.OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY;
        if (offset)
        {
            offset = ie.OffsetToData & 0x7FFFFFFFL;
            offset = offset+origoffset;
            f.at(offset);
            doResourceDir(origoffset,level+1,level ? resourceType : ie.Name, ie.Name);
        }
        else if (resourceType == RT_ICON)
        {
            f.at(ie.OffsetToData+origoffset);
            ResourceDataEntry de;
            getResourceDataEntry(&de);
            if (de.Size > 0){
                addIcon(res_id, virtualToReal(de.OffsetToData,section));
            }
        }
        else if (resourceType == RT_GROUP_ICON)
        {
            f.at(ie.OffsetToData+origoffset);
            ResourceDataEntry de;
            getResourceDataEntry(&de);
            if (de.Size >= 20){
                f.at(virtualToReal(de.OffsetToData,section) + 18);
                int icon_id = read_16ubit();
                addGroup(res_id, icon_id);
            }
        }
        f.at(pos);
    }
}

U32 IconsDLL::virtualToReal (U32 VAImageDir,PESectHdr *psh)
{
    U16 nSections = ph.NumberOfSections;
    int i = 0;

    /* Locate section containing image directory. */
    while (i++<nSections)
    {
        if (psh->VirtualAddress <= VAImageDir &&
                psh->VirtualAddress +
                psh->SizeOfRawData > VAImageDir)
            break;
        psh++;
    }

    if (i > nSections)
        return 0xffffffff;

    /* Return image import directory offset. */
    return (VAImageDir - psh->VirtualAddress + psh->PointerToRawData);
}

void IconsDLL::getResourceDirectory(ResourceDirectory *id)
{
    id->Characteristics = read_32ubit();
    id->TimeDateStamp = read_32ubit();
    id->MajorVersion = read_16ubit();
    id->MinorVersion = read_16ubit();
    id->NumberOfNamedEntries = read_16ubit();
    id->NumberOfIdEntries = read_16ubit();
}

void IconsDLL::getResourceDirectoryEntry(ResourceDirectoryEntry *ie)
{
    ie->Name = read_32ubit();
    ie->OffsetToData = read_32ubit();
}


void IconsDLL::getResourceDataEntry(ResourceDataEntry *de)
{
    de->OffsetToData = read_32ubit();
    de->Size = read_32ubit();
    de->CodePage = read_32ubit();
    de->Reserved = read_32ubit();
}

U32 IconsDLL::imageDirectoryOffset ()
{
    U16 nSections = ph.NumberOfSections;
    int i = 0;
    U32 VAImageDir;

    /* Must be 0 thru (NumberOfRvaAndSizes-1). */
    if (IMAGE_DIRECTORY_ENTRY_RESOURCE >= oh.NumberOfRvaAndSizes)
        return 0xffffffff;

    /* Locate image directory's relative virtual address. */
    VAImageDir = oh.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

    /* Locate section containing image directory. */
    PESectHdr *psh = section;
    while (i++<nSections)
    {
        if (psh->VirtualAddress <= VAImageDir &&
                psh->VirtualAddress +
                psh->SizeOfRawData > VAImageDir)
            break;
        psh++;
    }

    if (i > nSections)
        return 0xffffffff;

    /* Return image import directory offset. */
    return (VAImageDir - psh->VirtualAddress + psh->PointerToRawData);
}


static Icons gIcons;

const QIconSet &Icon(const char *name)
{
    return gIcons.getIcon(name);
}

QPixmap Pict(const char *name)
{
    return Icon(name).pixmap(QIconSet::Automatic, QIconSet::Normal);
}

void initIcons(const char *name)
{
    gIcons.init(name);
}




