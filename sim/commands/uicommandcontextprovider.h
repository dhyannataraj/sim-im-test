/*
 * uicommandcontextprovider.h
 *
 *  Created on: Aug 7, 2011
 */

#ifndef UICOMMANDCONTEXTPROVIDER_H_
#define UICOMMANDCONTEXTPROVIDER_H_

#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/imcontact.h"
#include "contacts/imgroup.h"
#include "uicommandcontext.h"

namespace SIM
{

class UiCommandContextProvider
{
public:
    virtual ~UiCommandContextProvider();

    virtual SIM::UiCommandContext::UiCommandContext contextType() const = 0;

    virtual ContactPtr getContextContact();
    virtual GroupPtr getContextGroup();
    virtual IMContactPtr getContextIMContact();
    virtual IMGroupPtr getContextIMGroup();
};

} /* namespace SIM */
#endif /* UICOMMANDCONTEXTPROVIDER_H_ */
