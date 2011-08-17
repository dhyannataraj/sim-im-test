/*
 * icqmessageeditorfactory.cpp
 *
 *  Created on: Aug 17, 2011
 */

#include "icqmessageeditorfactory.h"

ICQMessageEditorFactory::ICQMessageEditorFactory()
{
}

ICQMessageEditorFactory::~ICQMessageEditorFactory()
{
}

QWidget* createMessageEditor(const QString& id, QWidget* parent)
{
    if(id == "generic")
    {
    }
    return 0;
}

