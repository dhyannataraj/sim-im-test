/***************************************************************************
                          smiles.cpp  -  description
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

#include "smiles.h"
#include "icondll.h"

#include <qfile.h>
#include <qregexp.h>

Smiles::Smiles()
{
}

Smiles::~Smiles()
{
	clear();
}

bool Smiles::load(const QString &file)
{
	clear();
	for (unsigned i = 0; i < 16; i++){
		SmileDef sd;
		sd.icon = NULL;
		defs.push_back(sd);
	}
	QString fname = file;
    QFile f(fname);
    if (!f.open(IO_ReadOnly))
        return false;
#ifdef WIN32
	fname = fname.replace(QRegExp("\\"), "/");
#endif
	int pos = fname.findRev("/");
	log(L_DEBUG, "Pos %s %i", fname.latin1(), pos);
	if (pos >= 0){
		fname = fname.left(pos + 1);
	}else{
		fname = "";
	}
	string s;
	QRegExp start("^ *Smiley *= *");
	QRegExp num("^ *, *-[0-9]+ *, *");
	QRegExp nn("[0-9]+");
	while (getLine(f, s)){
		QString line = QString::fromLocal8Bit(s.c_str());
		if (line[0] == ';')
			continue;
		int size;
		int pos = start.match(line, 0, &size);
		if (pos < 0)
			continue;
		line = line.mid(size);
		getToken(line, '\"');
		QString dll = getToken(line, '\"', false);
		if (dll.isEmpty())
			continue;
		dll = dll.replace(QRegExp("\\\\"), "/");
		pos = num.match(line, 0, &size);
		if (pos < 0)
			continue;
		QString num = line.left(size);
		line = line.mid(size);
		pos = nn.match(num, 0, &size);
		unsigned nIcon = num.mid(pos, size).toUInt();
		getToken(line, '\"');
		QString pattern = getToken(line, '\"', false);
		getToken(line, '\"');
		QString tip = getToken(line, '\"', false);
		QString dllName = fname + dll;
		dllName = dllName.replace(QRegExp("/./"), "/");
		string fn = dllName.utf8();
		ICONS_MAP::iterator it = icons.find(fn.c_str());
		IconDLL *icon_dll = NULL;
		if (it == icons.end()){
			icon_dll = new IconDLL;
			if (!icon_dll->load(fn.c_str())){
				delete icon_dll;
				icon_dll = NULL;
			}
			icons.insert(ICONS_MAP::value_type(fn.c_str(), icon_dll));
		}else{
			icon_dll = (*it).second;
		}
		if (icon_dll == NULL)
			continue;
		SmileDef sd;
		sd.icon = icon_dll->get(nIcon);
		if (sd.icon == NULL)
			continue;
		sd.pattern = pattern;
		sd.tip = tip;
		unsigned index = (unsigned)(-1);
		while (!pattern.isEmpty()){
			QString pat = getToken(pattern, ' ', false);
			for (index = 0; index < 16; index++){
				const char *pp = defaultSmiles(index);
				for (; *pp; pp++)
					if (pat == pp)
						break;
				if (*pp)
					break;
			}
			if (index < 16)
				break;
		}

		if (index < 16){
			defs[index] = sd;
		}else{
			defs.push_back(sd);
		}
	}
	return true;
}

void Smiles::clear()
{
	for (ICONS_MAP::iterator it = icons.begin(); it != icons.end(); ++it){
		if ((*it).second)
			delete (*it).second;
	}
	icons.clear();
	defs.clear();
}

const QIconSet *Smiles::get(unsigned id)
{
	if (id < defs.size())
		return defs[id].icon;
	return NULL;
}

QString Smiles::pattern(unsigned id)
{
	if (id < defs.size())
		return defs[id].pattern;
	return "";
}

QString Smiles::tip(unsigned id)
{
	if (id < defs.size())
		return defs[id].tip;
	return "";
}

unsigned Smiles::count()
{
	return defs.size();
}


