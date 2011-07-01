/*
 * messagematchers.h
 *
 *  Created on: Jun 30, 2011
 *      Author: todin
 */

#ifndef MESSAGEMATCHERS_H_
#define MESSAGEMATCHERS_H_

#include <QString>
#include "messaging/message.h"

namespace Matcher
{
    class MessageTextMatcher
    {
    public:
        MessageTextMatcher(const QString& text) : m_text(text)
        {
        }

        bool operator()(const SIM::MessagePtr& msg) const
        {
            return msg->toPlainText() == m_text;
        }

    private:
        QString m_text;
    };

    class MessageSourceContactMatcher
    {
    public:
        MessageSourceContactMatcher(const SIM::IMContactPtr& contact) : m_contact(contact)
        {
        }

        bool operator()(const SIM::MessagePtr& msg) const
        {
            return msg->sourceContact().toStrongRef() == m_contact;
        }

    private:
        SIM::IMContactPtr m_contact;
    };
}

#endif /* MESSAGEMATCHERS_H_ */
