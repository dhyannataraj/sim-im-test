/*
 * uicommandlist.h
 *
 *  Created on: Aug 7, 2011
 */

#ifndef UICOMMANDLIST_H_
#define UICOMMANDLIST_H_

#include "uicommand.h"
#include "simapi.h"

namespace SIM
{

class EXPORT UiCommandList
{
public:
    UiCommandList();
    virtual ~UiCommandList();
    void clear();
    void appendCommand(const UiCommandPtr& command);

    QStringList allCommandIds() const;

private:
    QStringList m_commandIds;
};

} /* namespace SIM */
#endif /* UICOMMANDLIST_H_ */
