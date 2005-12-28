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
#include <qapplication.h>

#ifdef USE_KDE
#include <kapp.h>
#include <kipc.h>
#include <kiconloader.h>
#endif

#ifdef WIN32
#include <windows.h>
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
    addIconSet("icons/sim.jisp", true);
    m_defSets.push_back(new WrkIconSet);
    addIconSet("icons/smiles.jisp", false);
}

Icons::~Icons()
{
#if COMPAT_QT_VERSION < 0x030000
    QMimeSourceFactory::setDefaultFactory(new QMimeSourceFactory());
#endif
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
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
        m_icons.clear();
#endif
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

//#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)

PictDef *Icons::getPict(const QPixmap &p)
{
    unsigned serial = p.serialNumber();
    ICONS_MAP::iterator it = m_icons.find(serial);
    if (it == m_icons.end())
        return NULL;
    return getPict(it->second.c_str());
}

//#endif

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

#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
static QPixmap getPixmap(PictDef *d, const char *name)
#else
static QPixmap getPixmap(PictDef *d, const char*)
#endif
{
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    if (d->pixmap == NULL){
        QColor c = QApplication::palette().active().button();
        unsigned char cr = c.red();
        unsigned char cg = c.green();
        unsigned char cb = c.blue();
        QImage image(d->image->width(), d->image->height(), 32);
        QBitmap mask(d->image->width(), d->image->height());
        QPainter pmask(&mask);
        pmask.fillRect(0, 0, d->image->width(), d->image->height(), QColor(255, 255, 255));
        pmask.setPen(QColor(0, 0, 0));
        unsigned int *from = (unsigned int*)d->image->bits();
        unsigned int *to   = (unsigned int*)image.bits();
        for (int i = 0; i < d->image->height(); i++){
            for (int j = 0; j < d->image->width(); j++){
                unsigned int c = *(from++);
                unsigned char a = (c >> 24) & 0xFF;
                unsigned char r = (c >> 16) & 0xFF;
                unsigned char g = (c >> 8) & 0xFF;
                unsigned char b = c & 0xFF;
                r = (((r * a) + (cr * (0xFF - a))) >> 8) & 0xFF;
                g = (((g * a) + (cg * (0xFF - a))) >> 8) & 0xFF;
                b = (((b * a) + (cb * (0xFF - a))) >> 8) & 0xFF;
                *(to++) = 0xFF000000 + (r << 16) + (g << 8) + b;
                if (a)
                    pmask.drawPoint(j, i);
            }
        }
        pmask.end();
        QPixmap *res = new QPixmap;
        res->convertFromImage(image);
        res->setMask(mask);
        d->pixmap = res;
    }
    ICONS_MAP &icons = getIcons()->m_icons;
    ICONS_MAP::iterator it = icons.find(d->pixmap->serialNumber());
    if (it == icons.end())
        icons.insert(ICONS_MAP::value_type(d->pixmap->serialNumber(), name));
    return *(d->pixmap);
#else
    QPixmap res;
    res.convertFromImage(*(d->image));
    return res;
#endif
}

QIconSet Icon(const char *name)
{
    PictDef *pict = getPict(name);
    if (pict == NULL)
        return QIconSet();
    QIconSet res(getPixmap(pict, name));
    string bigName = "big.";
    bigName += name;
    pict = getPict(bigName.c_str());
    if (pict)
        res.setPixmap(getPixmap(pict, bigName.c_str()), QIconSet::Large);
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    string disName = "disabled.";
    disName += name;
    pict = getPict(disName.c_str());
    if (pict)
        res.setPixmap(getPixmap(pict, bigName.c_str()), QIconSet::Small, QIconSet::Disabled);
#endif
    return res;
}

const QImage *Image(const char *name)
{
    PictDef *p = getPict(name);
    if (p == NULL)
        return NULL;
    return p->image;
}

QPixmap Pict(const char *name)
{
    PictDef *p = getPict(name);
    if (p == NULL)
        return QPixmap();
    return getPixmap(p, name);
}

#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)

QPixmap Pict(const char *name, const QColor &c)
{
	const QImage *img = Image(name);
	if (img == NULL)
		return QPixmap();
	QImage res = img->copy();
	unsigned char cr = c.red();
	unsigned char cg = c.green();
	unsigned char cb = c.blue();
    unsigned int *data = (unsigned int*)res.bits();
    for (int i = 0; i < res.height() * res.width(); i++){
		unsigned char a = qAlpha(data[i]);
		data[i] = qRgba((qRed(data[i]) * a + cr * (0xFF - a)) >> 8,
			(qGreen(data[i]) * a + cg * (0xFF - a)) >> 8,
			(qBlue(data[i]) * a + cb * (0xFF - a)) >> 8, 0xFF);
	}
	QPixmap r;
	r.convertFromImage(res);
	return r;
}

#else

QPixmap Pict(const char *name, const QColor&)
{
	return Pict(name);
}

#endif

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
        PictDef *p = getPict(name.latin1());
        if (p)
            ((QMimeSourceFactory*)this)->setImage(abs_name, *(p->image));
    }
    return QMimeSourceFactory::data(abs_name);
}

IconSet::IconSet()
{
}

IconSet::~IconSet()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        if ((*it).second.image)
            delete (*it).second.image;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
        if ((*it).second.pixmap)
            delete (*it).second.pixmap;
#endif
    }
}

void IconSet::parseSmiles(const QString &text, unsigned &start, unsigned &size, string &name)
{
    for (list<smileDef>::iterator it = m_smiles.begin(); it != m_smiles.end(); ++it){
        QString pat = QString::fromUtf8(it->smile.c_str());
        int n = text.find(pat);
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

static QImage makeOffline(unsigned flags, const QImage &p)
{
    QImage image = p.copy();
    unsigned swapColor = flags & ICON_COLOR_MASK;
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    int i;
    for (i = 0; i < pixels; i++){
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
    return image;
}

static QImage makeInactive(const QImage &p)
{
    QImage image = p.copy();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    int i;
    for (i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
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
    return image;
}

#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)

static QImage makeDisabled(const QImage &p)
{
    QImage image(p.width() + 1, p.height() + 1, 32);
	unsigned int *data = (unsigned int*)image.bits();
	unsigned int *d = (unsigned int*)p.bits();
	QColorGroup g = QApplication::palette().disabled();
	QColor c1 = g.base();
    unsigned char cr1 = c1.red();
    unsigned char cg1 = c1.green();
    unsigned char cb1 = c1.blue();
	QColor c2 = g.button();
    unsigned char cr2 = c2.red();
    unsigned char cg2 = c2.green();
    unsigned char cb2 = c2.blue();
	unsigned int *f = d;
	unsigned int *t = data;
	int i;
	for (i = 0; i < p.width() + 1; i++)
		*(t++) = qRgba(cr2, cg2, cb2, 0);
	for (i = 0; i < p.height(); i++){
		*(t++) = qRgba(cr2, cg2, cb2, 0);
		for (int j = 0; j < p.width(); j++){
			unsigned char v = (qRed(*f) + qGreen(*f) + qBlue(*f)) / 3;
			*(t++) = qRgba((cr2 * v + cr1 * (0xFF - v)) >> 8,
				(cg2 * v + cg1 * (0xFF - v)) >> 8,
				(cb2 * v + cb1 * (0xFF - v)) >> 8, qAlpha(*f));
			f++;
		}
	}
	c1 = g.foreground();
	cr1 = c1.red();
	cg1 = c1.green();
	cb1 = c1.blue();
	f = d;
	t = data;
	for (i = 0; i < p.height(); i++){
		for (int j = 0; j < p.width(); j++){
			unsigned char a = qAlpha(*f);
			unsigned char v = (qRed(*f) + qGreen(*f) + qBlue(*f)) / 3;
			*t = qRgba((((cr2 * v + cr1 * (0xFF - v)) >> 8) * a + qRed(*t) * (0xFF - a)) >> 8,
				(((cg2 * v + cg1 * (0xFF - v)) >> 8) * a + qGreen(*t) * (0xFF - a)) >> 8,
				(((cb2 * v + cb1 * (0xFF - v)) >> 8) * a + qBlue(*t) * (0xFF - a)) >> 8, QMAX(qAlpha(*f), qAlpha(*t)));
			f++;
			t++;
		}
		t++;
	}
    return image;
}

#endif

static QImage merge(const QImage &p1, const QImage &p2)
{
    QImage img1 = p1.copy();
    if (img1.depth() != 32)
        img1 = img1.convertDepth(32);
    QImage img2(p2);
    if (img2.depth() != 32)
        img2 = img2.convertDepth(32);
    unsigned int *data1 = (unsigned int*)img1.bits();
    unsigned int *data2 = (unsigned int*)img2.bits();
    for (int y1 = 0; y1 < img1.height(); y1++){
        int y2 = y1 - (img1.height() - img2.height()) / 2;
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
    return img1;
}

PictDef *WrkIconSet::getPict(const char *name)
{
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
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    char _disabled[] = "disabled.";
    if ((strlen(name) > strlen(_disabled)) && !memcmp(name, _disabled, strlen(_disabled))){
        PictDef *p = SIM::getPict(n.substr(strlen(_disabled)).c_str());
        if (p == NULL)
            return NULL;
        return add(name, makeDisabled(*(p->image)), p->flags);
    }
#endif
    if ((strlen(name) <= 4) || memcmp(name, "big.", 4))
        log(L_DEBUG, "Icon %s not found", name);
    return NULL;
}

void WrkIconSet::clear()
{
    for (PIXMAP_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it){
        if ((*it).second.image)
            delete (*it).second.image;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
        if ((*it).second.pixmap)
            delete (*it).second.pixmap;
#endif
    }
    m_icons.clear();
}

PictDef *WrkIconSet::add(const char *name, const QImage &pict, unsigned flags)
{
    PictDef p;
    p.image = new QImage(pict);
    p.flags = flags;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    p.pixmap = NULL;
#endif
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
                pict = SmallIconSet(it->second.system.c_str()).pixmap(QIconSet::Small, QIconSet::Normal);
            }else{
                pict = DesktopIconSet(it->second.system.c_str()).pixmap(QIconSet::Large, QIconSet::Normal);
            }
            if (!pict.isNull()){
                (*it).second.image = new QImage(pict.convertToImage());
                return &((*it).second);
            }
        }
#endif
        if (it->second.file.empty())
            return NULL;
        QByteArray arr;
        if (!m_zip->readFile(QString::fromUtf8(it->second.file.c_str()), &arr))
            return NULL;
        (*it).second.image = new QImage(arr);
        (*it).second.image->convertDepth(32);
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
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
        if ((*it).second.pixmap){
            delete (*it).second.pixmap;
            (*it).second.pixmap = NULL;
        }
#endif
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
        PictDef p;
        p.image  = NULL;
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
        p.pixmap = NULL;
#endif
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

#ifndef _MSC_VER
#include "icons.moc"
#endif


