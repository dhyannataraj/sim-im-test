/***************************************************************************
                          replace.cpp  -  description
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

#include "replace.h"
#include "replacecfg.h"
#include "textshow.h"
#include "html.h"

#include <qapplication.h>

Plugin *createReplacePlugin(unsigned base, bool, Buffer *cfg)
{
    Plugin *plugin = new ReplacePlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Replace text"),
        I18N_NOOP("Plugin provides text replacing in message edit window"),
        VERSION,
        createReplacePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef replaceData[] =
    {
        { "Keys", DATA_ULONG, 1, 0 },
        { "Key", DATA_UTFLIST, 1, 0 },
        { "Value", DATA_UTFLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ReplacePlugin::ReplacePlugin(unsigned base, Buffer *cfg)
        : Plugin(base)
{
    load_data(replaceData, &data, cfg);
    qApp->installEventFilter(this);
}

ReplacePlugin::~ReplacePlugin()
{
    free_data(replaceData, &data);
}

string ReplacePlugin::getConfig()
{
    return save_data(replaceData, &data);
}

QWidget *ReplacePlugin::createConfigWindow(QWidget *parent)
{
    return new ReplaceCfg(parent, this);
}

class _UnquoteParser : public HTMLParser
{
public:
    _UnquoteParser(const QString &text);
    QString m_text;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

_UnquoteParser::_UnquoteParser(const QString &text)
{
    parse(text);
}

void _UnquoteParser::text(const QString &text)
{
    m_text += text;
}

void _UnquoteParser::tag_start(const QString &tag, const list<QString>&)
{
    if (tag == "img")
        m_text += " ";
    if (tag == "br")
        m_text += "\n";
}

void _UnquoteParser::tag_end(const QString&)
{
}

bool ReplacePlugin::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::KeyPress) && o->inherits("MsgTextEdit")){
        QKeyEvent *ke = (QKeyEvent*)e;
        if ((ke->key() == Key_Enter) || (ke->key() == Key_Return) || (ke->key() == Key_Space)){
            TextEdit *edit = (TextEdit*)o;
            int paraFrom, paraTo, indexFrom, indexTo;
            edit->getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
            if ((paraFrom == paraTo) && (indexFrom == indexTo)){
                int parag, index;
                edit->getCursorPosition(&parag, &index);
                _UnquoteParser p(edit->text(parag));
                QString text = p.m_text.left(index);
                for (unsigned i = 1; i <= getKeys(); i++){
                    QString key = getKey(i);
                    if (key.length() > text.length())
                        continue;
                    if (key != text.mid(text.length() - key.length()))
                        continue;
                    if ((key.length() < text.length()) && !text[(int)(text.length() - key.length() - 1)].isSpace())
                        continue;
                    edit->setSelection(parag, index - key.length(), parag, index);
                    edit->insert(getValue(i), false, false);
                    break;
                }
            }
        }
    }
    return QObject::eventFilter(o, e);
}

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif

#ifndef WIN32
#include "replace.moc"
#endif



