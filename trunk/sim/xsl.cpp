/***************************************************************************
                          xsl.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* before xsl.h because auf STL - problem !! */
#include <libxslt/xsltInternals.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "xsl.h"

#include <qfile.h>
#include <qregexp.h>

using std::string;
using namespace SIM;

class XSLPrivate
{
public:
    XSLPrivate(const char *my_xsl);
    ~XSLPrivate();
    xsltStylesheetPtr styleSheet;
    xmlDocPtr doc;
};

XSLPrivate::XSLPrivate(const char *my_xsl)
{
    styleSheet = NULL;
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    doc = xmlParseMemory(my_xsl, strlen(my_xsl));
    if (doc == NULL){
        log(L_WARN, "Can't parse XSLT");
        return;
    }
    styleSheet = xsltParseStylesheetDoc(doc);
}

XSLPrivate::~XSLPrivate()
{
    if (styleSheet)
        xsltFreeStylesheet(styleSheet);
}

static char STYLES[] = "styles/";
static char EXT[]    = ".xsl";

XSL::XSL(const QString &name)
{
    QString fname = STYLES;
    fname += name;
    fname += EXT;
    QFile f(user_file(fname));
    bool bOK = true;
    if (!f.open(IO_ReadOnly)){
        f.setName(app_file(fname));
        if (!f.open(IO_ReadOnly)){
            log(L_WARN, "Can't open %s", fname.local8Bit().data());
            bOK = false;
        }
    }
    QCString xsl;
    if(bOK){
		xsl = f.readAll();
    }
    d = new XSLPrivate(xsl);
}

XSL::~XSL()
{
    delete d;
}

void XSL::setXSL(const QString &my_xsl)
{
    delete d;
    d = new XSLPrivate(my_xsl);
}

QString XSL::process(const QString &my_xml)
{
    QString my_xsl;
    /* Petr Cimprich, Sablot developer:
       &nbsp; is predefined in HTML but not in XML
       ... use Unicode numerical entity instead: &#160;*/
    my_xsl = quote_nbsp(my_xml);

    xmlDocPtr doc = xmlParseMemory(my_xsl.utf8(), my_xsl.utf8().length());
    if (doc == NULL){
        log(L_WARN, "Parse XML error: %s", my_xsl.local8Bit().data());
        return QString::null;
    }
    const char *params[1];
    params[0] = NULL;
    xmlDocPtr res = xsltApplyStylesheet(d->styleSheet, doc, params);
    if (res == NULL){
        log(L_WARN, "Apply stylesheet error");
        xmlFreeDoc(doc);
        return QString::null;
    }
    xmlFreeDoc(doc);

    xmlOutputBufferPtr buf = xmlAllocOutputBuffer(NULL);
    xsltSaveResultTo(buf, res, d->styleSheet);
    xmlFreeDoc(res);

    QString result = QString::fromUtf8((char*)(buf->buffer->content));
    xmlOutputBufferClose(buf);;

    return result;
}

void XSL::cleanup()
{
    xsltCleanupGlobals();
}
