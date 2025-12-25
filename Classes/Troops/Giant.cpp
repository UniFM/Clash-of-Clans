#include "Giant.h"

Giant* Giant::create()
{
    Giant* pRet = new (std::nothrow) Giant();
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

bool Giant::init()
{
    return Unit::init(UnitType::GIANT);
}

