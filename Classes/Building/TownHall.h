#pragma once
#ifndef __TOWN_HALL_H__
#define __TOWN_HALL_H__

#include "Building.h"

class TownHall : public Building
{
public:
    static TownHall* create(int level = 1);
    virtual bool init(int level);

    virtual Size getGridSize() const override { return Size(4, 4); }
    virtual int getAttackPriority() const override { return 10; } // 最高优先级

    //CREATE_FUNC(TownHall);
};

#endif // __TOWN_HALL_H__

