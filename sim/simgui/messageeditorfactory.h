/*
 * messageeditorfactory.h
 *
 *  Created on: Aug 17, 2011
 */

#ifndef MESSAGEEDITORFACTORY_H_
#define MESSAGEEDITORFACTORY_H_

#include <QWidget>
#include "messageeditor.h"
#include "contacts/imcontact.h"

namespace SIM
{

class MessageEditorFactory
{
public:
    virtual ~MessageEditorFactory() {}
    virtual MessageEditor* createMessageEditor(const IMContactPtr& from, const IMContactPtr& to, const QString& id, QWidget* parent) = 0;
};

} /* namespace SIM */
#endif /* MESSAGEEDITORFACTORY_H_ */
