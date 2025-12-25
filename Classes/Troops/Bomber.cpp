#include "Bomber.h"

Bomber* Bomber::create()
{
    Bomber* pRet = new (std::nothrow) Bomber();
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

bool Bomber::init()
{
    return Unit::init(UnitType::BOMBER);
}

