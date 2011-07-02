/*
 * standardavatarstorage.h
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#ifndef STANDARDAVATARSTORAGE_H_
#define STANDARDAVATARSTORAGE_H_

#include "avatarstorage.h"
#include "misc.h"

namespace SIM
{

class SIM_EXPORT StandardAvatarStorage: public SIM::AvatarStorage
{
public:
    StandardAvatarStorage();
    virtual ~StandardAvatarStorage();

    virtual void addAvatar(const IMContactId& contactId, const QImage& image, const QString& type = "");
    virtual QImage getAvatar(const IMContactId& contactId, const QString& type = "");

    virtual QString id() const;
    virtual bool hasIcon(const QString& iconId);
    virtual QIcon icon(const QString& iconId);
    virtual QImage image(const QString& iconId);
    virtual QPixmap pixmap(const QString& iconId);
    virtual QString parseSmiles(const QString& input);

protected:
    virtual bool saveImage(const QString& path, const QImage& image);
    virtual QImage loadImage(const QString& path);

private:
    QString makeFilename(const IMContactId& id);
};

} /* namespace SIM */
#endif /* STANDARDAVATARSTORAGE_H_ */
