#pragma once
#ifndef __STORAGE_BUILDING_H__
#define __STORAGE_BUILDING_H__

#include "Building.h"

class StorageBuilding : public Building
{
public:
    static StorageBuilding* create(BuildingType type, int level = 1);
    virtual bool init(BuildingType type, int level);

    // ´æ´¢ÈÝÁ¿
    int getStorageCapacity() const { return _capacity; }

protected:
    int _capacity;
    void updateCapacity();
};

#endif // __STORAGE_BUILDING_H__

