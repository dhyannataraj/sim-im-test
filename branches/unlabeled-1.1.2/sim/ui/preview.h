/***************************************************************************
 *                         preview.h  -  description
 *                         -------------------
 *                         begin                : Sun Mar 24 2002
 *                         copyright            : (C) 2002 by Vladimir Shutoff
 *                         email                : vovan@shutoff.ru
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"
#ifdef USE_KDE
#include <kpreviewwidgetbase.h>
#else
#include <qfiledialog.h>
#endif

#ifdef USE_KDE
class UI_EXPORT FilePreview : public KPreviewWidgetBase
#else
class UI_EXPORT FilePreview : public QFilePreview, public QWidget
#endif
{
public:
    FilePreview(QWidget *parent);
    ~FilePreview();
    virtual void showPreview(const char *file) = 0;
#ifdef USE_KDE
    virtual void showPreview(const KURL &url);
    virtual void clearPreview();
#else
    virtual void previewUrl(const QUrl&);
#endif
};

#ifndef USE_KDE

class UI_EXPORT PictPreview : public FilePreview
{
public:
    PictPreview(QWidget *parent);
protected:
    QLabel  *label;
    void showPreview(const char*);
};

#endif
