#include "Archer.h"

Archer* Archer::create()
{
    Archer* pRet = new (std::nothrow) Archer();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        return nullptr;
    }
}

bool Archer::init()
{
    return Unit::init(UnitType::ARCHER);
}

