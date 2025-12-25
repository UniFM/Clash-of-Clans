#include "Barbarian.h"
#include "Constant/Constant.h"

USING_NS_CC;

Barbarian* Barbarian::create()
{
    Barbarian* barbarian = new (std::nothrow) Barbarian();
    if (barbarian && barbarian->init())
    {
        barbarian->autorelease();
        return barbarian;
    }
    CC_SAFE_DELETE(barbarian);
    return nullptr;
}

bool Barbarian::init()
{
    if (!Unit::init(UnitType::BARBARIAN))
        return false;

    // 设置野蛮人的属性
    _type = UnitType::BARBARIAN;
    _hp = 45;
    _maxHP = 45;
    _attackDamage = 18;
    _moveSpeed = 16;
    _attackRange = 0; // 近战单位

    // 加载野蛮人精灵（如果有资源的话）
    // auto sprite = Sprite::create("troops/barbarian.png");
    // if (sprite) {
    //     this->addChild(sprite);
    // }

    // 临时使用颜色矩形表示
    auto drawNode = DrawNode::create();
    drawNode->drawSolidRect(Vec2(-10, -10), Vec2(10, 10), Color4F::RED);
    this->addChild(drawNode);

    return true;
}

