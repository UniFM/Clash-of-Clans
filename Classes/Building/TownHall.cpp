#include "TownHall.h"

TownHall* TownHall::create(int level)
{
    TownHall* hall = new (std::nothrow) TownHall();
    if (hall && hall->init(level))
    {
        hall->autorelease();
        return hall;
    }
    CC_SAFE_DELETE(hall);
    return nullptr;
}

bool TownHall::init(int level)
{
    return Building::init(BuildingType::TOWN_HALL, level);
}

