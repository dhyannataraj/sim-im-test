/*
 * messagetypedescriptor.h
 *
 *  Created on: Aug 16, 2011
 */

#ifndef MESSAGETYPEDESCRIPTOR_H_
#define MESSAGETYPEDESCRIPTOR_H_

#include <QString>

namespace SIM
{
    struct MessageTypeDescriptor
    {
        QString id;
        QString iconId;
        QString text;
        QString clientId;
        bool joinable;
    };
}


#endif /* MESSAGETYPEDESCRIPTOR_H_ */
