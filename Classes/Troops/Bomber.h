#ifndef __BOMBER_H__
#define __BOMBER_H__

#include "Unit.h"

class Bomber : public Unit
{
public:
    static Bomber* create();
    virtual bool init() override;
//private:    
//    CREATE_FUNC(Bomber);
};

#endif // __BOMBER_H__

