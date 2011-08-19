/*
 * icqmessageeditorfactory.h
 *
 *  Created on: Aug 17, 2011
 */

#ifndef ICQMESSAGEEDITORFACTORY_H_
#define ICQMESSAGEEDITORFACTORY_H_

#include "messageeditorfactory.h"
#include "messageeditor.h"
#include "icq_defines.h"

class ICQ_EXPORT ICQMessageEditorFactory: public SIM::MessageEditorFactory
{
public:
    ICQMessageEditorFactory();
    virtual ~ICQMessageEditorFactory();
    virtual SIM::MessageEditor* createMessageEditor(const QString& id, QWidget* parent);
};

#endif /* ICQMESSAGEEDITORFACTORY_H_ */
