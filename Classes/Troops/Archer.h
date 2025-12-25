#pragma once
#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Unit.h"

class Archer : public Unit
{
public:
    static Archer* create();
    virtual bool init() override;

    //CREATE_FUNC(Archer);
};

#endif // __ARCHER_H__

