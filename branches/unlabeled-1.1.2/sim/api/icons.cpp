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

#include <qiconset.h>
#include <qmime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qmime.h>

#ifdef USE_KDE
#include <kapp.h>
#include <kipc.h>
#include <kiconloader.h>
#endif

#include <map>
using namespace std;

const unsigned ICON_COLOR_MASK	= 0x000000FF;

namespace SIM
{

class WrkIconSet : public IconSet
{
public:
	WrkIconSet();
    const QPixmap *getPict(const char *name, unsigned &flags);
	void clear();
protected:
	const QPixmap *add(const char *name, const QPixmap &pict, unsigned flags);
};

class FileIconSet : public IconSet, public SAXParser
{
public:
	FileIconSet(const char *file);
	~FileIconSet();
    const QPixmap *getPict(const char *name, unsigned &flags);
	void clear();
protected:
    virtual	void element_start(const char *el, const char **attr);
    virtual	void element_end(const char *el);
    virtual	void char_data(const char *str, int len);
	list<string> m_smiles;
	string		m_name;
	string		m_file;
	string		m_smile;
	string		*m_data;
	unsigned	m_flags;
	UnZip		*m_zip;
};

class MyMimeSourceFactory : public QMimeSourceFactory
{
public:
    MyMimeSourceFactory();
    ~MyMimeSourceFactory();
    virtual const QMimeSource* data(const QString &abs_name) const;
};

unsigned Icons::nSmile = 0;

Icons::Icons()
{
    /* This idea came from kapplication.cpp
       I had a similar idea with setting the old defaultFactory in
       the destructor but this won't work :(
       Christian */
#if COMPAT_QT_VERSION >= 0x030000
    QMimeSourceFactory* oldDefaultFactory = QMimeSourceFactory::takeDefaultFactory();
#endif
    QMimeSourceFactory::setDefaultFactory(new MyMimeSourceFactory());
#if COMPAT_QT_VERSION >= 0x030000
    if (oldDefaultFactory)
        QMimeSourceFactory::addFactory( oldDefaultFactory );
#endif
	m_sets.push_back(new FileIconSet("icons/smiles.jisp"));
	m_sets.push_back(new FileIconSet("icons/sim.jisp"));
	m_sets.push_back(new WrkIconSet);
}

Icons::~Icons()
{
#if COMPAT_QT_VERSION < 0x030000
    QMimeSourceFactory::setDefaultFactory(new QMimeSourceFactory());
#endif
	for (list<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it)
		delete *it;
}

void *Icons::processEvent(Event *e)
{
    if (e->type() == EventIconChanged){
		for (list<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it)
			(*it)->clear();
    }
    return NULL;
}

void Icons::iconChanged(int)
{
    Event e(EventIconChanged, NULL);
    e.process();
}

const QPixmap *Icons::getPict(const char *name, unsigned &flags)
{
	for (list<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it){
		const QPixmap *res = (*it)->getPict(name, flags);
		if (res)
			return res;
	}
	return NULL;
}

QString Icons::parseSmiles(const QString &str)
{
	QString s = str;
	QString res;
	while (!s.isEmpty()){
		unsigned start = (unsigned)(-1);
		unsigned size  = 0;
		string smile;
		for (list<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it){
			unsigned pos    = (unsigned)(-1);
			unsigned length = 0;
			string n_smile;
			(*it)->parseSmiles(str, pos, length, n_smile);
			if (length == 0)
				continue;
			if (pos < start){
				start = pos;
				size  = length;
				smile = n_smile;
			}
		}
		if (size == 0){
			res += s;
			break;
		}
		res += s.left(start);
		res += "<img src=\"";
		res += smile.c_str();
		res += "\" alt=\"";
		res += quoteString(s.mid(start, size));
		res += "\">";
		s = s.mid(start + size);
	}
	return res;
}

Icons *Icons::icons = NULL;

void createIcons()
{
	Icons::icons = new Icons;
}

void deleteIcons()
{
	delete Icons::icons;
}

const QPixmap *getPict(const char *name, unsigned &flags)
{
	return Icons::icons->getPict(name, flags);
}

QIconSet Icon(const char *name)
{
	unsigned flags;
	const QPixmap *pict = getPict(name, flags);
	if (pict == NULL)
		return NULL;
	QIconSet res;
	res.setPixmap(*pict, QIconSet::Small);
	string bigName = "big.";
	bigName += name;
	pict = getPict(bigName.c_str(), flags);
	if (pict)
		res.setPixmap(*pict, QIconSet::Large);
	return res;
}

QPixmap Pict(const char *name)
{
	unsigned flags;
    const QPixmap *icon = getPict(name, flags);
    if (icon == NULL)
        return QPixmap();
    return *icon;
}

MyMimeSourceFactory::MyMimeSourceFactory()
        : QMimeSourceFactory()
{
}

MyMimeSourceFactory::~MyMimeSourceFactory()
{
}

const QMimeSource *MyMimeSourceFactory::data(const QString &abs_name) const
{
    QString name = abs_name;
    if (name.left(5) == "icon:"){
        name = name.mid(5);
		unsigned flags;
        const QPixmap *icon = getPict(name.latin1(), flags);
        if (icon){
            QImage img = icon->convertToImage();
            img = img.convertDepth(32);
            ((QMimeSourceFactory*)this)->setImage(abs_name, img);
        }
    }
    return QMimeSourceFactory::data(abs_name);
}

IconSet::IconSet()
{
}

IconSet::~IconSet()
{
	for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
		if ((*it).second.icon)
			delete (*it).second.icon;
	}
}

void IconSet::parseSmiles(const QString &text, unsigned &start, unsigned &size, string &name)
{
	for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
		list<string> &smiles = (*it).second.smiles;
		if (smiles.empty())
			continue;
		for (list<string>::iterator itl = smiles.begin(); itl != smiles.end(); ++itl){
			QString pat = QString::fromUtf8((*itl).c_str());
			int n = text.find(pat);
			if (n < 0)
				continue;
			if (((unsigned)n < start) || (((unsigned)n == start) && (pat.length() > size))){
				start = n;
				size  = pat.length();
				name  = (*it).first.c_str();
			}
		}
	}	
}

WrkIconSet::WrkIconSet()
{
}

static QPixmap makeOffline(unsigned flags, const QPixmap &p)
{
    unsigned swapColor = flags & ICON_COLOR_MASK;
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
        if (swapColor){
            h = (swapColor * 2 - h) & 0xFF;
            c.setHsv(h, s, v);
        }else{
            c.setHsv(h, 0, v * 3 / 4);
        }
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QPixmap makeInactive(const QPixmap &p)
{
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
        c.setHsv(h, s / 8, v);
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QPixmap makeInvisible(unsigned flags, const QPixmap &p)
{
    unsigned swapColor = flags & ICON_COLOR_MASK;
    char shift = (flags >> 8) & 0xFF;
    QImage image = p.convertToImage();
    if (image.depth() != 32)
        image = image.convertDepth(32);
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
            c.hsv(&h, &s, &v);
            if (swapColor){
                h = (swapColor * 2 - h) & 0xFF;
                c.setHsv(h, s / 2, v * 3 / 4);
            }else{
                c.setHsv(h, s / 2, v * 3 / 4);
            }
            *line = qRgba(c.red(), c.green(), c.blue(), a);
        }
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QPixmap merge(const QPixmap &p1, const QPixmap &p2)
{
    QImage img1 = p1.convertToImage();
    if (img1.depth() != 32)
        img1 = img1.convertDepth(32);
    QImage img2 = p2.convertToImage();
    if (img2.depth() != 32)
        img2 = img2.convertDepth(32);
    unsigned int *data1 = (unsigned int*)img1.bits();
    unsigned int *data2 = (unsigned int*)img2.bits();
    for (int y1 = 0; y1 < img1.height(); y1++){
        int y2 = y1 - (img1.height() - img2.height());
        if ((y2 < 0) || (y2 >= img2.height()))
            continue;
        unsigned int *line1 = data1 + y1 * img1.width();
        unsigned int *line2 = data2 + y2 * img2.width();
        int w = img1.width();
        if (w < img2.width()){
            line2 += (img2.width() - w) / 2;
        }else if (w > img2.width()){
            w = img2.width();
            line1 += (img1.width() - w) / 2;
        }
        for (int i = 0; i < w; i++, line1++, line2++){
            int r1 = qRed(*line1);
            int g1 = qGreen(*line1);
            int b1 = qBlue(*line1);
            int a1 = qAlpha(*line1);
            int r2 = qRed(*line2);
            int g2 = qGreen(*line2);
            int b2 = qBlue(*line2);
            int a2 = qAlpha(*line2);
            r1 = (r2 * a2 + r1 * (255 - a2)) / 255;
            g1 = (g2 * a2 + g1 * (255 - a2)) / 255;
            b1 = (b2 * a2 + b1 * (255 - a2)) / 255;
            if (a2 > a1)
                a1 = a2;
            *line1 = qRgba(r1, g1, b1, a1);
        }
    }
    QPixmap res;
    res.convertFromImage(img1);
    return res;
}

const QPixmap *WrkIconSet::getPict(const char *name, unsigned &flags)
{
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it != m_icons.end()){
		flags = (*it).second.flags;
		return (*it).second.icon;
	}
	
	string n = name;
	if (n == "online"){
		unsigned i;
		const QPixmap *icon = NULL;
		const char *icon_name = NULL;
		for (i = 0; i < getContacts()->nClients(); i++){
			Client *client = getContacts()->getClient(i);
			icon_name = client->protocol()->description()->icon;
			icon = SIM::getPict(icon_name, flags);
			if (icon)
				break;
		}
		if (icon == NULL){
			icon_name = "ICQ";
			icon = SIM::getPict(icon_name, flags);
		}
		if (icon == NULL)
			return NULL;
		return add(name, *icon, flags);
	}
	if (n == "offline"){
		const QPixmap *icon = SIM::getPict("online", flags);
		if (icon == NULL)
			return NULL;
		return add(name, makeOffline(flags, *icon), flags);
	}
	if (n == "inactive"){
		const QPixmap *icon = SIM::getPict("online", flags);
		if (icon == NULL)
			return NULL;
		return add(name, makeInactive(*icon), flags);
	}
	if (n == "invisible"){
		const QPixmap *icon = SIM::getPict("online", flags);
		if (icon == NULL)
			return NULL;
		return add(name, makeInvisible(flags, *icon), flags);
	}
	int pos = n.find('_');
	if (pos > 0){
		const QPixmap *icon = SIM::getPict(n.substr(0, pos).c_str(), flags);
		QPixmap res;
		if (icon){
			string s = n.substr(pos + 1);
			if (s == "online"){
				res = *icon;
			}else if (s == "offline"){
				res = makeOffline(flags, *icon);
			}else if (s == "invisible"){
				res = makeInvisible(flags, *icon);
			}else if (s == "inactive"){
				res = makeInactive(*icon);
			}else{
				unsigned f;
				const QPixmap *i = SIM::getPict(s.c_str(), f);
				if (i)
					res = merge(*icon, *i);
			}
			return add(name, res, flags);
		}
	}
	log(L_DEBUG, "Icon %s not found", name);
	return NULL;
}

void WrkIconSet::clear()
{
	for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
		if ((*it).second.icon)
			delete (*it).second.icon;
	}
	m_icons.clear();
}

const QPixmap *WrkIconSet::add(const char *name, const QPixmap &pict, unsigned flags)
{
	PictDef p;
	p.icon  = new QPixmap(pict);
	p.flags = flags;
	m_icons.insert(PIXMAP_MAP::value_type(name, p));
	return m_icons.find(name)->second.icon;
}

FileIconSet::FileIconSet(const char *file)
{
	m_zip = new UnZip(QFile::decodeName(app_file(file).c_str()));
	QByteArray arr;
	m_data = NULL;
	if (m_zip->open() && m_zip->readFile("icondef.xml", &arr))
		parse(arr.data(), arr.size());
}

FileIconSet::~FileIconSet()
{
	delete m_zip;
}

const QPixmap *FileIconSet::getPict(const char *name, unsigned &flags)
{
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = m_icons.find(name);
    if (it == m_icons.end())
		return NULL;
	flags = (*it).second.flags;
	if ((*it).second.icon == NULL){
		if (it->second.file.empty())
			return NULL;
		QByteArray arr;
		if (!m_zip->readFile(QString::fromUtf8(it->second.file.c_str()), &arr))
			return NULL;
		(*it).second.icon = new QPixmap(arr);
	}
	return (*it).second.icon;
}

void FileIconSet::clear()
{
	for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
		if ((*it).second.icon == NULL)
			continue;
		delete (*it).second.icon;
		(*it).second.icon = NULL;
	}
}

void FileIconSet::element_start(const char *el, const char **args)
{
	if (!strcmp(el, "icon")){
		m_smiles.clear();
		m_name  = "";
		m_smile = "";
		m_flags = 0;
		m_file  = "";
		if (args){
			for (; *args; ){
				const char *key = *(args++);
				const char *value = *(args++);
				if (!strcmp(key, "name"))
					m_name = value;
				if (!strcmp(key, "flags"))
					m_flags = atol(value);
			}
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
		QStringList l = QImage::inputFormatList();
		for (unsigned i = 0; i < l.count(); i++){
			if (l[i].lower() != mime.c_str())
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
		if (m_name.empty()){
			m_name = "s_";
			m_name += number(++Icons::nSmile);
		}
		PictDef p;
		p.icon   = NULL;
		p.file   = m_file;
		p.flags  = m_flags;
		p.smiles = m_smiles;
		m_smiles.clear();
		PIXMAP_MAP::iterator it = m_icons.find(m_name.c_str());
		if (it == m_icons.end())
			m_icons.insert(PIXMAP_MAP::value_type(m_name.c_str(), p));
	}
	if (!strcmp(el, "text")){
		if (!m_smile.empty())
			m_smiles.push_back(m_smile);
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

#ifndef WIN32
#include "icons.moc"
#endif


