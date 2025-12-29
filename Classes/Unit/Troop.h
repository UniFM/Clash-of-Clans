#pragma once
#include "cocos2d.h"
#include "Constant/Constant.h"

class Troop : public cocos2d::Sprite {
public:
    static Troop* create(Troops type);
    virtual bool init(Troops type);

    // Start patrolling with a callback to get next position
    void startPatrol(std::function<cocos2d::Vec2(const cocos2d::Size&)> getNextPosition);
    
    // Stop patrolling
    void stopPatrol();

protected:
    Troop();
    virtual ~Troop();

    void createWalkAnimation();
    void playWalkAnimation();
    void stopWalkAnimation();
    
    void moveToNextPosition();

    Troops _type;
    std::function<cocos2d::Vec2(const cocos2d::Size&)> _getNextPosition;
    bool _isPatrolling;
};
