/***************************************************************************
                          spell.cpp  -  description
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

#include "spell.h"
#include "spellcfg.h"
#include "speller.h"
#include "simapi.h"
#include "textshow.h"
#include "spellhighlight.h"
#include "core.h"

#include <qapplication.h>
#include <qwidgetlist.h>

class PSpellHighlighter : public SpellHighlighter
{
public:
    PSpellHighlighter(TextEdit *edit, SpellPlugin *plugin);
    ~PSpellHighlighter();
};

PSpellHighlighter::PSpellHighlighter(TextEdit *edit, SpellPlugin *plugin)
        : SpellHighlighter(edit, plugin)
{
    m_plugin->m_edits.insert(MAP_EDITS::value_type(edit, this));
    QObject::connect(edit, SIGNAL(finished(TextEdit*)), plugin, SLOT(textEditFinished(TextEdit*)));
    QObject::connect(this, SIGNAL(check(const QString&)), plugin, SLOT(check(const QString&)));
    QObject::connect(plugin, SIGNAL(misspelling(const QString&)), this, SLOT(slotMisspelling(const QString&)));
    QObject::connect(plugin, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
}

PSpellHighlighter::~PSpellHighlighter()
{
    MAP_EDITS::iterator it = m_plugin->m_edits.find(static_cast<TextEdit*>(textEdit()));
    if (it != m_plugin->m_edits.end())
        m_plugin->m_edits.erase(it);
}

Plugin *createSpellPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new SpellPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Spell check"),
        I18N_NOOP("Plugin provides check spelling"),
        VERSION,
        createSpellPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef spellData[] =
    {
#ifdef WIN32
        { "Path", DATA_STRING, 1, 0 },
#endif
        { "Lang", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

SpellPlugin::SpellPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(spellData, &data, config);
    m_bActive = false;
    m_base = NULL;
    CmdSpell = registerType();

    Command cmd;
    cmd->id          = CmdSpell;
    cmd->text        = "_";
    cmd->menu_id     = MenuTextEdit;
    cmd->menu_grp    = 0x0100;
    cmd->flags		 = COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();
    reset();
}

SpellPlugin::~SpellPlugin()
{
    Event eCmd(EventCommandRemove, (void*)CmdSpell);
    eCmd.process();
    deactivate();
    for (list<Speller*>::iterator it = m_spellers.begin(); it != m_spellers.end(); ++it)
        delete (*it);
    delete m_base;
    free_data(spellData, &data);
}

void SpellPlugin::reset()
{
    for (list<Speller*>::iterator it = m_spellers.begin(); it != m_spellers.end(); ++it)
        delete (*it);
    m_spellers.clear();
    if (m_base)
        delete m_base;
#ifdef WIN32
    m_base = new SpellerBase(getPath());
#else
    m_base = new SpellerBase;
#endif
    SpellerConfig cfg(*m_base);
    string ll = getLang();
    while (!ll.empty()){
        string l = getToken(ll, ';');
        cfg.setKey("lang", l.c_str());
        cfg.setKey("encoding", "utf-8");
        Speller *speller = new Speller(&cfg);
        if (speller->created()){
            m_spellers.push_back(speller);
            continue;
        }
        delete speller;
    }
    if (m_spellers.empty()){
        deactivate();
    }else{
        activate();
    }
    configChanged();
}

void SpellPlugin::activate()
{
    if (m_bActive)
        return;
    m_bActive = true;
    qApp->installEventFilter(this);
    QWidgetList  *list = QApplication::allWidgets();
    QWidgetListIt it( *list );
    QWidget * w;
    while ( (w=it.current()) != 0 ){
        ++it;
        if (w->inherits("TextEdit"))
            new PSpellHighlighter(static_cast<TextEdit*>(w), this);
    }
    delete list;
}

void SpellPlugin::deactivate()
{
    if (!m_bActive)
        return;
    m_bActive = false;
    qApp->removeEventFilter(this);
    while (!m_edits.empty())
        delete (*m_edits.begin()).second;
    m_edits.clear();
}

string SpellPlugin::getConfig()
{
    return save_data(spellData, &data);
}

QWidget *SpellPlugin::createConfigWindow(QWidget *parent)
{
    return new SpellConfig(parent, this);
}

void *SpellPlugin::processEvent(Event*)
{
    return NULL;
}

bool SpellPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ChildInserted){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        if (ce->child()->inherits("MsgTextEdit")){
            TextEdit *edit = static_cast<TextEdit*>(ce->child());
            MAP_EDITS::iterator it = m_edits.find(edit);
            if (it == m_edits.end())
                new PSpellHighlighter(edit, this);
        }
    }
    return QObject::eventFilter(o, e);
}

void SpellPlugin::textEditFinished(TextEdit *edit)
{
    MAP_EDITS::iterator it = m_edits.find(edit);
    if (it != m_edits.end())
        delete (*it).second;
}

void SpellPlugin::check(const QString &word)
{
    for (list<Speller*>::iterator it = m_spellers.begin(); it != m_spellers.end(); ++it){
        if ((*it)->check(word.utf8()) == 1)
            return;
    }
    emit misspelling(word);
}

void SpellPlugin::add(const QString &word)
{
    for (list<Speller*>::iterator it = m_spellers.begin(); it != m_spellers.end(); ++it){
        if ((*it)->add(word.utf8()))
            return;
    }
}

QStringList SpellPlugin::suggestions(const QString &word)
{
    QStringList res;
    for (list<Speller*>::iterator it = m_spellers.begin(); it != m_spellers.end(); ++it){
        QStringList wl = (*it)->suggestions(word.utf8());
        for (QStringList::Iterator it = wl.begin(); it != wl.end(); ++it){
            QStringList::Iterator itr;
            for (itr = res.begin(); itr != res.end(); ++itr){
                if ((*itr) == (*it))
                    break;
            }
            if (itr == res.end())
                res.append(*it);
        }
    }
    return res;
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
#include "spell.moc"
#endif

