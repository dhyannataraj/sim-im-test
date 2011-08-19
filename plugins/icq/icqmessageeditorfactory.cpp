/*
 * icqmessageeditorfactory.cpp
 *
 *  Created on: Aug 17, 2011
 */

#include "icqmessageeditorfactory.h"
#include "simgui/genericmessageeditor.h"

ICQMessageEditorFactory::ICQMessageEditorFactory()
{
}

ICQMessageEditorFactory::~ICQMessageEditorFactory()
{
}

SIM::MessageEditor* ICQMessageEditorFactory::createMessageEditor(const QString& id, QWidget* parent)
{
    if(id == "generic")
    {
        return new SIM::GenericMessageEditor(parent);
    }
    return 0;
}

