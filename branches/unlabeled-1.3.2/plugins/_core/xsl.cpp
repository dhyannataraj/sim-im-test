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

#include "xsl.h"

#include <qfile.h>
#include <qregexp.h>

#include <sablot.h>
#include <shandler.h>

class XSLPrivate
{
public:
    XSLPrivate(const char *my_xsl);
    ~XSLPrivate();
    SablotSituation S;
    SDOM_Document xsl;
};

XSLPrivate::XSLPrivate(const char *my_xsl)
{
    SablotCreateSituation(&S);
    SablotParseStylesheetBuffer(S, my_xsl, &xsl);
}

XSLPrivate::~XSLPrivate()
{
    SablotDestroyDocument(S, xsl);
    SablotDestroySituation(S);
}

MH_ERROR xsl_makeCode(void*, SablotHandle, int, unsigned short, unsigned short code)
{
    return code;
}

MH_ERROR xsl_log(void*, SablotHandle, MH_ERROR, MH_LEVEL, char**)
{
    return 0;
}

MH_ERROR xsl_error(void*, SablotHandle, MH_ERROR code, MH_LEVEL level, char **fields)
{
    int log_level = L_DEBUG;
    switch (level){
    case MH_LEVEL_WARN:
        log_level = L_WARN;
        break;
    case MH_LEVEL_ERROR:
    case MH_LEVEL_CRITICAL:
        log_level = L_ERROR;
        break;
    case MH_LEVEL_DEBUG:
    case MH_LEVEL_INFO:
        log_level = L_DEBUG;
        break;
    }
    string flds;
    for (; *fields; fields++){
        if (!flds.empty())
            flds += ", ";
        flds += *fields;
    }
    log(level, "XSL: %u %s", code, flds.c_str());
    return 0;
}

static MessageHandler mh =
    {
        xsl_makeCode,
        xsl_log,
        xsl_error
    };

#ifdef WIN32
static char STYLES[] = "styles\\";
#else
static char STYLES[] = "styles/";
#endif
static char EXT[]    = ".xsl";

XSL::XSL(const QString &name)
{
    string fname = STYLES;
    fname += QFile::encodeName(name);
    fname += EXT;
    QFile f(QFile::decodeName(user_file(fname.c_str()).c_str()));
    bool bOK = true;
    if (!f.open(IO_ReadOnly)){
        f.setName(QFile::decodeName(app_file(fname.c_str()).c_str()));
        if (!f.open(IO_ReadOnly)){
            log(L_WARN, "Can't open %s", fname.c_str());
            bOK = false;
        }
    }
    string xsl;
    if (bOK){
        xsl.append(f.size(), '\x00');
        f.readBlock((char*)(xsl.c_str()), f.size());
        f.close();
    }
    d = new XSLPrivate(xsl.c_str());
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
    SablotSituation S;
    SablotHandle proc;

    SDOM_Document xml;

    QString my_xsl;
    /* Petr Cimprich, Sablot developer:
       &nbsp; is predefined in HTML but not in XML
       ... use Unicode numerical entity instead: &#160;*/
    my_xsl = my_xml;
    my_xsl.replace( QRegExp("&nbsp;"), QString("&#160;") );

    SablotCreateSituation(&S);
    SablotParseBuffer(S, my_xsl.utf8(), &xml);
    SablotCreateProcessorForSituation(S, &proc);

    SablotRegHandler(proc, HLR_MESSAGE, &mh, NULL);

    SablotAddArgTree(S, proc, "sheet", d->xsl);
    SablotAddArgTree(S, proc, "data", xml);

    SablotRunProcessorGen(S, proc, "arg:/sheet", "arg:/data", "arg:/out");

    char *result;
    SablotGetResultArg(proc, "arg:/out", &result);
    QString res = QString::fromUtf8(result);

    SablotFree(result);
    SablotDestroyProcessor(proc);
    SablotDestroyDocument(S, xml);

    SablotDestroySituation(S);

    return res;
}

