/*
 * standardavatarstorage.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include "standardavatarstorage.h"

namespace SIM
{

StandardAvatarStorage::StandardAvatarStorage()
{
}

StandardAvatarStorage::~StandardAvatarStorage()
{
}

void StandardAvatarStorage::addAvatar(const IMContactId& contactId, const QImage& image, const QString& type)
{
    saveImage(makeFilename(contactId), image);
}

QImage StandardAvatarStorage::getAvatar(const IMContactId& contactId, const QString& type)
{
    return QImage();
}

QString StandardAvatarStorage::id() const
{
    return "standard-avatar-storage";
}

bool StandardAvatarStorage::hasIcon(const QString& iconId)
{
    return false;
}

QIcon StandardAvatarStorage::icon(const QString& iconId)
{
    return QIcon();
}

QImage StandardAvatarStorage::image(const QString& iconId)
{
    return QImage();
}

QPixmap StandardAvatarStorage::pixmap(const QString& iconId)
{
    return QPixmap();
}

QString StandardAvatarStorage::parseSmiles(const QString& input)
{
    return QString();
}

bool StandardAvatarStorage::saveImage(const QString& path, const QImage& image)
{
    return true;
}

QImage StandardAvatarStorage::loadImage(const QString& path)
{
    return QImage();
}

QString StandardAvatarStorage::makeFilename(const IMContactId& id)
{
    QString str = id.toString();
    str.replace('/', '.');
    str = str.toLower();
    return str;
}

} /* namespace SIM */
