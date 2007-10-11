#include"jabberclient.h"
#include "sax.h"

#ifndef _JABBER__IQ_H
#define _JABBER__IQ_H

class JabberClient;

class JabberClient::Iq : public SAXParser
{
  public:
  
  virtual void element_start(const QString& el, const QXmlAttributes& attrs);
  virtual void element_end(const QString& el);
  virtual void char_data(const QString& str);
		  

    Iq();
    ~Iq();
		
};

#endif
