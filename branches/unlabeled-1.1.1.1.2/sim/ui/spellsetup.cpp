/***************************************************************************
                          spellsetup.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "spellsetup.h"
#include "mainwin.h"
#include "icons.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#ifdef USE_SPELL
#include <ksconfig.h>
#include <qlayout.h>
#endif

SpellSetup::SpellSetup(QWidget *p)
        : SpellSetupBase(p)
{
    lblPict->setPixmap(Pict("spellcheck"));
#ifdef USE_SPELL
    QVBoxLayout *lay = new QVBoxLayout(widget);
    KSpellConfig *spell = new KSpellConfig(widget);
    lay->addWidget(spell);
    chkSpell->setChecked(pMain->SpellOnSend);
#endif
}

void SpellSetup::apply(ICQUser*)
{
    pMain->SpellOnSend = chkSpell->isChecked();
}

#ifndef _WINDOWS
#include "spellsetup.moc"
#endif

