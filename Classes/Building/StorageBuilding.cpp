#include "StorageBuilding.h"
#include "ResourceManager.h"

StorageBuilding* StorageBuilding::create(BuildingType type, int level)
{
    StorageBuilding* storage = new (std::nothrow) StorageBuilding();
    if (storage && storage->init(type, level))
    {
        storage->autorelease();
        return storage;
    }
    CC_SAFE_DELETE(storage);
    return nullptr;
}

bool StorageBuilding::init(BuildingType type, int level)
{
    if (!Building::init(type, level))
        return false;

    updateCapacity();

    // 更新资源管理器的最大容量
    auto resourceMgr = ResourceManager::getInstance();
    if (type == BuildingType::GOLD_STORAGE)
    {
        // 可以在这里更新最大金币容量
    }
    else if (type == BuildingType::ELIXIR_STORAGE)
    {
        // 可以在这里更新最大圣水容量
    }

    return true;
}

void StorageBuilding::updateCapacity()
{
    _capacity = 10000 + (_level - 1) * 5000;
}

