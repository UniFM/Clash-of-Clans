#ifndef __GIANT_H__
#define __GIANT_H__

#include "Unit.h"

class Giant : public Unit
{
public:
    static Giant* create();
    virtual bool init() override;
    
    //CREATE_FUNC(Giant);
};

#endif // __GIANT_H__

