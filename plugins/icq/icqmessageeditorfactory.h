/*
 * icqmessageeditorfactory.h
 *
 *  Created on: Aug 17, 2011
 */

#ifndef ICQMESSAGEEDITORFACTORY_H_
#define ICQMESSAGEEDITORFACTORY_H_

#include "simgui/messageeditorfactory.h"
#include "simgui/messageeditor.h"
#include "icq_defines.h"
#include "contacts/imcontact.h"

class ICQ_EXPORT ICQMessageEditorFactory: public SIM::MessageEditorFactory
{
public:
    ICQMessageEditorFactory();
    virtual ~ICQMessageEditorFactory();
    virtual SIM::MessageEditor* createMessageEditor(const SIM::IMContactPtr& from, const SIM::IMContactPtr& to, const QString& id, QWidget* parent);
};

#endif /* ICQMESSAGEEDITORFACTORY_H_ */
