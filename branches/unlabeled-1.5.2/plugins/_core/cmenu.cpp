/***************************************************************************
                          cmenu.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "cmenu.h"
#include "commands.h"

#include <qaccel.h>
#include <qtimer.h>
#include <qapplication.h>

CMenu::CMenu(CommandsDef *def)
        : KPopupMenu(NULL)
{
    m_def = def;
    m_param = NULL;
	m_bInit = false;
    setCheckable(true);
    connect(this, SIGNAL(aboutToShow()), this, SLOT(showMenu()));
    connect(this, SIGNAL(aboutToHide()), this, SLOT(hideMenu()));
    connect(this, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
}

CMenu::~CMenu()
{
}

void CMenu::setParam(void *param)
{
    m_param = param;
}

void CMenu::processItem(CommandDef *s, bool &bSeparator, bool &bFirst, unsigned base_id)
{
    if (s->id == 0){
        bSeparator = true;
        return;
    }
    s->param = m_param;
    if (s->flags & COMMAND_CHECK_STATE){
        s->flags &= ~COMMAND_DISABLED;
        s->text_wrk = NULL;
        Event e(EventCheckState, s);
        s->flags |= COMMAND_CHECK_STATE;
        if (!e.process()){
            if (s->text_wrk)
                free(s->text_wrk);
            return;
        }
        if (s->flags & COMMAND_RECURSIVE){
            CommandDef *cmds = (CommandDef*)(s->param);
            for (CommandDef *cmd = cmds; cmd->text; cmd++){
                processItem(cmd, bSeparator, bFirst, s->id);
            }
            delete[] cmds;
            return;
        }
    }
	if (m_wrk->count()){
		QSize s = m_wrk->sizeHint();
	    QWidget *desktop = qApp->desktop();
		int nHeight = (s.height() - margin() * 2) / m_wrk->count();
		if (s.height() + nHeight * 2 + margin() * 2 >= desktop->height()){
			KPopupMenu *more = new KPopupMenu(m_wrk);
			m_wrk->insertItem(i18n("More..."), more);
			m_wrk = more;
			connect(m_wrk, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
		}
	}
    if (bFirst){
        bFirst = false;
        bSeparator = false;
    }else if (bSeparator){
        m_wrk->insertSeparator();
        bSeparator = false;
    }
    const QIconSet *icons = NULL;
    if ((s->flags & COMMAND_CHECKED) && s->icon_on)
        icons = Icon(s->icon_on);
    if ((icons == NULL) && s->icon)
        icons = Icon(s->icon);
    QString title = i18n(s->text);
    if (s->text_wrk){
        title = QString::fromUtf8(s->text_wrk);
        free(s->text_wrk);
    }
    if (s->accel){
        title += "\t";
        title += i18n(s->accel);
    }
    if (s->flags & COMMAND_TITLE){
        if (icons){
            m_wrk->insertTitle(icons->pixmap(QIconSet::Automatic, QIconSet::Normal), title);
        }else{
            m_wrk->insertTitle(title);
        }
        bFirst = true;
        bSeparator = false;
        return;
    }
    QPopupMenu *popup = NULL;
    if (s->popup_id){
        ProcessMenuParam mp;
        mp.id = s->popup_id;
        mp.param = s->param;
        mp.key	 = 0;
        Event e(EventProcessMenu, &mp);
        popup = (QPopupMenu*)(e.process());
    }
    unsigned id = 0;
    if (popup){
        if (icons){
            m_wrk->insertItem(*icons, title, popup);
        }else{
            m_wrk->insertItem(title, popup);
        }
    }else{
        CMD c;
        c.id = s->id;
        c.base_id = base_id;
        m_cmds.push_back(c);
        id = m_cmds.size();
        if (icons){
            m_wrk->insertItem(*icons, title, id);
        }else{
            m_wrk->insertItem(title, id);
        }
    }
    if (id){
        if (s->flags & COMMAND_DISABLED)
            m_wrk->setItemEnabled(id, false);
        if (s->accel)
            m_wrk->setAccel(QAccel::stringToKey(i18n(s->accel)), id);
        m_wrk->setItemChecked(id, (s->flags & COMMAND_CHECKED) != 0);
    }
    bSeparator = false;
}

void CMenu::showMenu()
{
	initMenu();
}

void CMenu::hideMenu()
{
	m_bInit = false;
}

void CMenu::clearMenu()
{
    clear();
}

QSize CMenu::sizeHint() const
{
	((CMenu*)this)->initMenu();
	return KPopupMenu::sizeHint();
}

void CMenu::initMenu()
{
	if (m_bInit)
		return;
	m_bInit = true;
	m_wrk   = this;
    clear();
    m_cmds.clear();
    bool bSeparator = false;
    bool bFirst = true;
    CommandsList list(*m_def);
    CommandDef *s;
    while ((s = ++list) != NULL){
        processItem(s, bSeparator, bFirst, 0);
    }
}

void CMenu::menuActivated(int n)
{
    if ((n < 1) || (n > (int)(m_cmds.size())))
        return;

    CMD c = m_cmds[n - 1];
    unsigned id = c.id;
    if (c.base_id)
        id = c.base_id;

    CommandsList list(*m_def, true);
    CommandDef *s;
    while ((s = ++list) != NULL){
        if (s->id == id){
            s->text_wrk = NULL;
            if (s->flags & COMMAND_CHECK_STATE){
                s->param = m_param;
                Event e(EventCheckState, s);
                s->flags |= COMMAND_CHECK_STATE;
                if (!e.process()){
                    if (s->text_wrk){
                        free(s->text_wrk);
                        s->text_wrk = NULL;
                    }
                    return;
                }
                s->flags ^= COMMAND_CHECKED;
                if (s->flags & COMMAND_RECURSIVE){
                    CommandDef *cmds = (CommandDef*)(s->param);
                    for (CommandDef *c = cmds; c->text; c++){
                        if (c->text_wrk)
                            free(c->text_wrk);
                    }
                    delete[] cmds;
                }
            }
            if (c.base_id)
                s->id = c.id;
            s->param = m_param;
            Event e(EventCommandExec, s);
            e.process();
            if (s->text_wrk)
                free(s->text_wrk);
            s->id = id;
            break;
        }
    }
}

#ifndef _WINDOWS
#include "cmenu.moc"
#endif

