/*
 * avatarstorage.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include "avatarstorage.h"
#include "standardavatarstorage.h"

namespace SIM
{

static AvatarStorage* gs_avatarStorage = 0;

void createAvatarStorage()
{
    Q_ASSERT(!gs_avatarStorage);
    gs_avatarStorage = new StandardAvatarStorage();
}

void destroyAvatarStorage()
{
    Q_ASSERT(gs_avatarStorage);
    delete gs_avatarStorage;
    gs_avatarStorage = 0;
}

AvatarStorage* getAvatarStorage()
{
    return gs_avatarStorage;
}

void setAvatarStorage(AvatarStorage* storage)
{
    if(gs_avatarStorage)
        delete gs_avatarStorage;
    gs_avatarStorage = storage;
}

} /* namespace SIM */
