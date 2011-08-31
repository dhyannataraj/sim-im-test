/*
 * mockimagestorage.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef MOCKIMAGESTORAGE_H_
#define MOCKIMAGESTORAGE_H_

#include "imagestorage/imagestorage.h"

namespace MockObjects
{
    class MockImageStorage : public SIM::ImageStorage
    {
    public:
        MOCK_METHOD1(icon, QIcon(const QString& id));
        MOCK_METHOD1(image, QImage(const QString& id));
        MOCK_METHOD1(pixmap, QPixmap(const QString& id));

        MOCK_METHOD1(addIconSet, bool(SIM::IconSet* set));
        MOCK_METHOD1(removeIconset, bool(const QString& id));
    };
}

#endif /* MOCKIMAGESTORAGE_H_ */
