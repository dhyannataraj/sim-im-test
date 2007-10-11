
#include "log.h"
#include "iq.h"

using namespace SIM;
//using namespace std;


void JabberClient::Iq::element_start(const QString& el, const QXmlAttributes& attrs)
{
  log(L_DEBUG, "Element Start: %s", el.latin1());
}

void JabberClient::Iq::element_end(const QString& el)
{
  log(L_DEBUG, "Element End: %s", el.latin1());
}

void JabberClient::Iq::char_data(const QString& str)
{}    

JabberClient::Iq::Iq()
{

}

JabberClient::Iq::~Iq()
{

}
