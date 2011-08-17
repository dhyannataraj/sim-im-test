/*
 * icqmessageeditorfactory.h
 *
 *  Created on: Aug 17, 2011
 */

#ifndef ICQMESSAGEEDITORFACTORY_H_
#define ICQMESSAGEEDITORFACTORY_H_

#include "messageeditorfactory.h"

class ICQMessageEditorFactory: public SIM::MessageEditorFactory
{
public:
    ICQMessageEditorFactory();
    virtual ~ICQMessageEditorFactory();
    virtual QWidget* createMessageEditor(const QString& id, QWidget* parent);
};

#endif /* ICQMESSAGEEDITORFACTORY_H_ */
