#pragma once
#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Unit.h"

class Barbarian : public Unit
{
public:
    static Barbarian* create();
    virtual bool init() override;

    //CREATE_FUNC(Barbarian);
};

#endif // __BARBARIAN_H__

