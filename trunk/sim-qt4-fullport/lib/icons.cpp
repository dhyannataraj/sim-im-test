/***************************************************************************
                          icons.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sax.h"
#include "icons.h"
#include "qzip.h"

#include <QIcon>
#include <QMimeSource>
#include <QImage>
#include <QPainter>
#include <QBitmap>
#include <QMimeData>
#include <QFile>
#include <QApplication>
#include <QImageReader>

#include <QPixmap>

#ifdef USE_KDE
#include <kapp.h>
#include <kipc.h>
#include <kiconloader.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <map>
#include <iostream>
using namespace std;

const unsigned ICON_COLOR_MASK	= 0x000000FF;

namespace SIM
{

class WrkIconSet : public IconSet
{
public:
    WrkIconSet();
    PictDef *getPict(const char *name);
    void clear();
protected:
    PictDef *add(const char *name, const QImage &pict, unsigned flags);
};

class FileIconSet : public IconSet, public SAXParser
{
public:
    FileIconSet(const char *file);
    ~FileIconSet();
    PictDef *getPict(const char *name);
    void clear();
protected:
    virtual	void element_start(const char *el, const char **attr);
    virtual	void element_end(const char *el);
    virtual	void char_data(const char *str, int len);
    string		m_name;
    string		m_file;
    string		m_smile;
    string		*m_data;
#ifdef USE_KDE
    string		m_system;
#endif
    unsigned	m_flags;
    UnZip		*m_zip;
};

unsigned Icons::nSmile = 0;

Icons::Icons()
{
    /* This idea came from kapplication.cpp
       I had a similar idea with setting the old defaultFactory in
       the destructor but this won't work :(
       Christian */
    addIconSet("icons/sim.jisp", true);
    m_defSets.push_back(new WrkIconSet);
    addIconSet("icons/smiles.jisp", false);
}

Icons::~Icons()
{
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it)
        delete *it;
    for (it = m_defSets.begin(); it != m_defSets.end(); ++it)
        delete *it;
}

void *Icons::processEvent(Event *e)
{
    if (e->type() == EventIconChanged){
        list<IconSet*>::iterator it;
        for (it = m_customSets.begin(); it != m_customSets.end(); ++it)
            (*it)->clear();
        for (it = m_defSets.begin(); it != m_defSets.end(); ++it)
            (*it)->clear();
    }
    return NULL;
}

void Icons::iconChanged(int)
{
    Event e(EventIconChanged, NULL);
    e.process();
}

PictDef *Icons::getPict(const char *name)
{
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it){
        PictDef *res = (*it)->getPict(name);
        if (res)
            return res;
    }
    for (it = m_defSets.begin(); it != m_defSets.end(); ++it){
        PictDef *res = (*it)->getPict(name);
        if (res)
            return res;
    }
    return NULL;
}

list<string> Icons::getSmile(const char *name)
{
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it){
        list<string> res = (*it)->getSmile(name);
        if (!res.empty())
            return res;
    }
    list<string> res;
    return res;
}

string Icons::getSmileName(const char *name)
{
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it){
        string res = (*it)->getSmileName(name);
        if (!res.empty())
            return res;
    }
    return "";
}

void Icons::getSmiles(list<string> &smiles)
{
    list<string> used;
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it)
        (*it)->getSmiles(smiles, used);
}

QString Icons::parseSmiles(const QString &str)
{
    QString s = str;
    QString res;
    while (!s.isEmpty()){
        unsigned start = (unsigned)(-1);
        unsigned size  = 0;
        string smile;
        list<IconSet*>::iterator it;
        for (it = m_customSets.begin(); it != m_customSets.end(); ++it){
            unsigned pos    = (unsigned)(-1);
            unsigned length = 0;
            string n_smile;
            (*it)->parseSmiles(s, pos, length, n_smile);
            if (length == 0)
                continue;
            if (pos < start){
                start = pos;
                size  = length;
                smile = n_smile;
            }
        }
        if (size == 0){
            res += quoteString(s);
            break;
        }
        res += s.left(start);
        res += "<img src=\"icon:";
        res += smile.c_str();
        res += "\" alt=\"";
        res += quoteString(s.mid(start, size));
        res += "\">";
        s = s.mid(start + size);
    }
    return res;
}

IconSet *Icons::addIconSet(const char *name, bool bDefault)
{
    FileIconSet *is = new FileIconSet(name);
    if (bDefault){
        m_defSets.push_front(is);
    }else{
        m_customSets.push_back(is);
    }
    return is;
}

void Icons::removeIconSet(IconSet *is)
{
    list<IconSet*>::iterator it;
    for (it = m_customSets.begin(); it != m_customSets.end(); ++it){
        if (*it == is){
            delete is;
            m_customSets.erase(it);
            return;
        }
    }
    for (it = m_defSets.begin(); it != m_defSets.end(); ++it){
        if (*it == is){
            delete is;
            m_defSets.erase(it);
            return;
        }
    }
}

static QImage makeOffline(unsigned flags, const QImage &p)
{
    QImage image = p.copy();
    unsigned swapColor = flags & ICON_COLOR_MASK;
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
                 (unsigned int*)image.colorTable().size();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    int i;
    for (i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.getHsv(&h, &s, &v);
        if (swapColor){
            h = (swapColor * 2 - h) & 0xFF;
            c.setHsv(h, s, v);
        }else{
            c.setHsv(h, 0, v * 3 / 4);
        }
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    return image;
}

static QImage makeInactive(const QImage &p)
{
    QImage image = p.copy();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
                 (unsigned int*)image.colorTable().size();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    int i;
    for (i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.getHsv(&h, &s, &v);
        c.setHsv(h, s / 8, v);
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    return image;
}

static QImage makeInvisible(unsigned flags, const QImage &p)
{
    QImage image = p.copy();
    unsigned swapColor = flags & ICON_COLOR_MASK;
    char shift = (flags >> 8) & 0xFF;
    image = image.convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly);
    unsigned int *data = (unsigned int*)image.bits();
    for (int y = 0; y < image.width(); y++){
        int x = image.width() / 2 - (y - image.height() / 2) * 2 / 3 + shift;
        if (x < 0)
            x = 0;
        if (x > image.width())
            x = image.width();
        unsigned int *line = data + y * (image.width()) + x;
        for (; x < image.width(); x++, line++){
            QColor c(qRed(*line), qGreen(*line), qBlue(*line));
            int a = qAlpha(*line);
            int h, s, v;
            c.getHsv(&h, &s, &v);
            if (swapColor){
                h = (swapColor * 2 - h) & 0xFF;
                c.setHsv(h, s / 2, v * 3 / 4);
            }else{
                c.setHsv(h, s / 2, v * 3 / 4);
            }
            *line = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
    return image;
}

static QPixmap merge(const QString &cmdicon, const QString &cmd)
{
    QPixmap statusicon(":/icons/" + cmd + ".png");
    QPixmap protoicon(":/icons/" + cmdicon + ".png");
    QPainter painter;
    painter.begin( &protoicon);
    painter.drawPixmap(0, 0, statusicon);
    painter.end();
    return protoicon;
}

static Icons *icons = NULL;

Icons *getIcons()
{
    return icons;
}

void createIcons()
{
    icons = new Icons;
}

void deleteIcons()
{
    delete icons;
}

PictDef *getPict(const char *name)
{
    return icons->getPict(name);
}

static QPixmap getPixmap(PictDef *d, const char*)
{
    QPixmap res;
    res.fromImage(*(d->image));
    return res;
}

QIcon Icon(const char *name)
{
    QString iconPath = name;
    iconPath = iconPath.toLower();
    if (iconPath.indexOf("!") != -1)
	iconPath.replace(QString("!"), QString(""));
    if (iconPath.indexOf("_") != -1)
    {
        QString cmd = iconPath.right( iconPath.size() - iconPath.indexOf("_") - 1);
	QString cmdicon = iconPath.left( iconPath.indexOf("_"));
	
//	std::cout << "found icon request for command " << static_cast<string>(cmd.toLocal8Bit()) << " and icon " << static_cast<string>(cmdicon.toLocal8Bit()) << std::endl;
	
	if (cmd == "online")
	{
	    return QIcon(":/icons/" + cmdicon + ".png");
	}

	if (cmd == "offline")
	{
	    QImage *officon = new QImage(":/icons/" + cmdicon + ".png");
	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeOffline(330, *officon), Qt::ColorOnly));
	}

	if (cmd == "invisible")
	{
	    QImage *officon = new QImage(":/icons/" + cmdicon + ".png");
	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeInvisible(330, *officon), Qt::ColorOnly));
	}
	
	if (cmd == "inactive")
	{
    	    QImage *officon = new QImage(":/icons/" + cmdicon + ".png");
    	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeInactive(*officon), Qt::ColorOnly));
	}
	
	if (cmd == "away")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "na")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "dnd")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "ffc")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "occupied")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}
    }
    
    if (iconPath == "inactive")
    {
        QImage *officon = new QImage(":/icons/icq.png");
        QPixmap returnIcon;
	return QIcon( returnIcon.fromImage(makeInactive(*officon), Qt::ColorOnly));
    }

    if (iconPath == "offline")
    {
        QImage *officon = new QImage(":/icons/icq.png");
        QPixmap returnIcon;
	return QIcon( returnIcon.fromImage(makeOffline(330, *officon), Qt::ColorOnly));
    }
    
    QIcon returnIcon = QIcon(":/icons/" + iconPath + ".png");
    if ( ! returnIcon.isNull())
	return returnIcon;
    return QIcon();
}

const QImage *Image(const char *name)
{
    PictDef *p = getPict(name);
    if (p == NULL)
        return NULL;
    return p->image;
}

QIcon getIcon(const char *name)
{
    QString iconPath = name;
    iconPath = iconPath.toLower();
    if (iconPath.indexOf("!") != -1)
	iconPath.replace(QString("!"), QString(""));
    if (iconPath.indexOf("_") != -1)
    {
        QString cmd = iconPath.right( iconPath.size() - iconPath.indexOf("_") - 1);
	QString cmdicon = iconPath.left( iconPath.indexOf("_"));
	
//	std::cout << "found standard command " << static_cast<string>(cmd.toLocal8Bit()) << " and icon " << static_cast<string>(cmdicon.toLocal8Bit()) << std::endl;
	
	if (cmd == "online")
	{
	    return QIcon(":/icons/" + cmdicon + ".png");
	}

	if (cmd == "offline")
	{
	    QImage *officon = new QImage(":/icons/" + cmdicon + ".png");
	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeOffline(330, *officon), Qt::ColorOnly));
	}

	if (cmd == "invisible")
	{
	    QImage *officon = new QImage(":/icons/" + cmdicon + ".png");
	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeInvisible(330, *officon), Qt::ColorOnly));
	}
	
	if (cmd == "inactive")
	{
    	    QImage *officon = new QImage(":/icons/" + cmdicon +  ".png");
    	    QPixmap returnIcon;
	    return QIcon( returnIcon.fromImage(makeInactive(*officon), Qt::ColorOnly));
	}
	
	if (cmd == "away")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "na")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "dnd")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "ffc")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}

	if (cmd == "occupied")
	{
		QPixmap returnIcon = merge(cmdicon, cmd);
		return QIcon( returnIcon);
	}
    }
    
    if (iconPath == "inactive")
    {
        QImage *officon = new QImage(":/icons/icq.png");
        QPixmap returnIcon;
	return QIcon( returnIcon.fromImage(makeInactive(*officon), Qt::ColorOnly));
    }

    if (iconPath == "offline")
    {
        QImage *officon = new QImage(":/icons/icq.png");
        QPixmap returnIcon;
	return QIcon( returnIcon.fromImage(makeOffline(330, *officon), Qt::ColorOnly));
    }
    
    QIcon returnIcon = QIcon(":/icons/" + iconPath + ".png");
    if ( ! returnIcon.isNull())
	return returnIcon;
    return QIcon();
}

QIcon getIcon(const char *name, const QColor&)
{
    return getIcon(name);
}

IconSet::IconSet()
{
}

IconSet::~IconSet()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        if ((*it).second.image)
            delete (*it).second.image;
    }
}

void IconSet::parseSmiles(const QString &text, unsigned &start, unsigned &size, string &name)
{
    for (list<smileDef>::iterator it = m_smiles.begin(); it != m_smiles.end(); ++it){
        QString pat = QString::fromUtf8(it->smile.c_str());
        int n = text.indexOf(pat);
        if (n < 0)
            continue;
        if (((unsigned)n < start) || (((unsigned)n == start) && (pat.length() > size))){
            start = n;
            size  = pat.length();
            name  = it->name.c_str();
        }
    }
}

void IconSet::getSmiles(list<string> &smiles, list<string> &used)
{
    string name;
    bool bOK = false;
    for (list<smileDef>::iterator it = m_smiles.begin(); it != m_smiles.end(); ++it){
        if (name != it->name){
            if (bOK && !name.empty())
                smiles.push_back(name);
            name = it->name;
            bOK = true;
        }
        list<string>::iterator itu;
        for (itu = used.begin(); itu != used.end(); ++itu){
            if ((*itu) == it->smile)
                break;
        }
        if (itu == used.end()){
            used.push_back(it->smile);
        }else{
            bOK = false;
        }
    }
    if (bOK && !name.empty())
        smiles.push_back(name);
}

list<string> IconSet::getSmile(const char *name)
{
    list<string> res;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return res;
    for (list<smileDef>::iterator its = m_smiles.begin(); its != m_smiles.end(); ++its){
        if (its->name != name)
            continue;
        res.push_back(its->smile);
    }
    return res;
}

string IconSet::getSmileName(const char *name)
{
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return "";
    string res = it->second.file;
    int n = res.find('.');
    if (n > 0)
        res = res.substr(0, n);
    return res;
}

WrkIconSet::WrkIconSet()
{
}

PictDef *WrkIconSet::getPict(const char *name)
{ /*
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it != m_icons.end())
        return &(*it).second;

    string n = name;
    if (n == "online"){
        unsigned i;
        PictDef *p = NULL;
        const char *icon_name = NULL;
        for (i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            icon_name = client->protocol()->description()->icon;
            p = SIM::getPict(icon_name);
            if (p)
                break;
        }
        if (p == NULL){
            icon_name = "ICQ";
            p = SIM::getPict(icon_name);
        }
        if (p == NULL)
            return NULL;
        return add(name, *(p->image), p->flags);
    }
    if (n == "offline"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeOffline(p->flags, *(p->image)), p->flags);
    }
    if (n == "inactive"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeInactive(*(p->image)), p->flags);
    }
    if (n == "invisible"){
        PictDef *p = SIM::getPict("online");
        if (p == NULL)
            return NULL;
        return add(name, makeInvisible(p->flags, *(p->image)), p->flags);
    }
    int pos = n.find('_');
    if (pos > 0){
        PictDef *p = SIM::getPict(n.substr(0, pos).c_str());
        QImage res;
        if (p){
            string s = n.substr(pos + 1);
            if (s == "online"){
                res = *(p->image);
            }else if (s == "offline"){
                res = makeOffline(p->flags, *(p->image));
            }else if (s == "invisible"){
                res = makeInvisible(p->flags, *(p->image));
            }else if (s == "inactive"){
                res = makeInactive(*(p->image));
            }else{
                PictDef *pp = SIM::getPict(s.c_str());
                if (pp)
                    res = merge(*(p->image), *(pp->image));
            }
            return add(name, res, p->flags);
        }
    }
    if ((strlen(name) <= 4) || memcmp(name, "big.", 4))
        log(L_DEBUG, "Icon %s not found", name);
	*/
    return NULL;
}

void WrkIconSet::clear()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        if ((*it).second.image)
            delete (*it).second.image;
    }
    m_icons.clear();
}

PictDef *WrkIconSet::add(const char *name, const QImage &pict, unsigned flags)
{
    PictDef p;
    p.image = new QImage(pict);
    p.flags = flags;
    m_icons.insert(PIXMAP_MAP::value_type(name, p));
    return &m_icons.find(name)->second;
}

FileIconSet::FileIconSet(const char *file)
{
    m_zip = new UnZip(QFile::decodeName(app_file(file).c_str()));
    QByteArray arr;
    m_data = NULL;
    if (m_zip->open() && m_zip->readFile("icondef.xml", &arr))
        parse(arr.data(), arr.size(), false);
}

FileIconSet::~FileIconSet()
{
    delete m_zip;
}

PictDef *FileIconSet::getPict(const char *name)
{
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
        return NULL;
    if ((*it).second.image == NULL){
#ifdef USE_KDE
        if (!it->second.system.empty()){
            QPixmap pict;
            if (memcmp(name, "big.", 4)){
                pict = SmallIconSet(it->second.system.c_str()).pixmap(QIcon::Small, QIcon::Normal);
            }else{
                pict = DesktopIconSet(it->second.system.c_str()).pixmap(QIcon::Large, QIcon::Normal);
            }
            if (!pict.isNull()){
                (*it).second.image = new QImage(pict.toImage());
                return &((*it).second);
            }
        }
#endif
        if (it->second.file.empty())
            return NULL;
        QByteArray arr;
        if (!m_zip->readFile(QString::fromUtf8(it->second.file.c_str()), &arr))
            return NULL;
        (*it).second.image = new QImage(static_cast<QString>(arr));
    }
    return &((*it).second);
}

void FileIconSet::clear()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        if ((*it).second.image){
            delete (*it).second.image;
            (*it).second.image = NULL;
        }
    }
}

void FileIconSet::element_start(const char *el, const char **args)
{
    if (!strcmp(el, "icon")){
        m_name  = "";
        m_smile = "";
        m_flags = 0;
        m_file  = "";
#ifdef USE_KDE
        m_system = "";
#endif
        if (args){
            for (; *args; ){
                const char *key = *(args++);
                const char *value = *(args++);
                if (!strcmp(key, "name"))
                    m_name = value;
                if (!strcmp(key, "flags"))
                    m_flags = atol(value);
#ifdef USE_KDE
                if (!strcmp(key, "kicon"))
                    m_system = value;
#endif
            }
        }
        if (m_name.empty()){
            m_name = "s_";
            m_name += number(++Icons::nSmile);
        }
        return;
    }
    if (!strcmp(el, "object") && m_file.empty()){
        string mime;
        if (args){
            for (; *args; ){
                const char *key = *(args++);
                const char *value = *(args++);
                if (!strcmp(key, "mime"))
                    mime = value;
            }
        }
        if (mime.empty())
            return;
        int n = mime.find('/');
        if (n < 0)
            return;
        if (mime.substr(0, n) != "image")
            return;
        mime = mime.substr(n + 1);
        QList<QByteArray> l = QImageReader::supportedImageFormats();
        for (unsigned i = 0; i < l.count(); i++){
            if (l[i].toLower() != mime.c_str())
                continue;
            m_data = &m_file;
            return;
        }
        return;
    }
    if (!strcmp(el, "text")){
        m_smile = "";
        m_data = &m_smile;
    }
}

void FileIconSet::element_end(const char *el)
{
    if (!strcmp(el, "icon")){
        PictDef p;
        p.image  = NULL;
        p.file   = m_file;
        p.flags  = m_flags;
#ifdef USE_KDE
        p.system = m_system;
#endif
        PIXMAP_MAP::iterator it = m_icons.find(m_name.c_str());
        if (it == m_icons.end())
            m_icons.insert(PIXMAP_MAP::value_type(m_name.c_str(), p));
#ifdef USE_KDE
        if (m_name.substr(0, 4) != "big."){
            string big_name = "big.";
            big_name += m_name;
            p.file   = "";
            p.flags  = m_flags;
            p.system = m_system;
            it = m_icons.find(big_name.c_str());
            if (it == m_icons.end())
                m_icons.insert(PIXMAP_MAP::value_type(big_name.c_str(), p));
        }
#endif
    }
    if (!strcmp(el, "text")){
        if (!m_smile.empty() && !m_name.empty()){
            smileDef s;
            s.name  = m_name;
            s.smile = m_smile;
            m_smiles.push_back(s);
        }
        m_smile = "";
    }
    m_data = NULL;
}

void FileIconSet::char_data(const char *data, int size)
{
    if (m_data)
        m_data->append(data, size);
}

};
