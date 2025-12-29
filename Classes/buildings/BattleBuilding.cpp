#include "BattleBuilding.h"
#include "Control/BattleManager.h"

USING_NS_CC;

BattleBuilding* BattleBuilding::create(BattleBuildingType type, Team team) {
    BattleBuilding* pRet = new(std::nothrow) BattleBuilding();
    if (pRet && pRet->init(type, team)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool BattleBuilding::init(BattleBuildingType type, Team team) {
    if (!Entity::init()) return false;

    _type = type;
    _stats = BattleManager::getInstance()->getBattleBuildingStats(type);
    setStats(_stats.maxHealth, team);

    _target = nullptr;
    _attackTimer = 0.0f;
    std::string imagePath = getImagePathForBattleBuilding(type, team);
    auto sprite = Sprite::create(imagePath);

    if (sprite) {
        // 设置图片锚点为底部中心，这样血条可以显示在顶部
        sprite->setAnchorPoint(Vec2(0.5f, 0.0f));

        // 根据建筑类型调整大小
        float scale = getScaleForBattleBuilding(type);
        sprite->setScale(scale);


        this->addChild(sprite, 0);

        //// 添加血条（如果需要的话）
        //setupHealthBar(sprite->getContentSize().height * scale);
    }
    else {
        // 如果图片加载失败，回退到原来的颜色块
        CCLOG("Failed to load image for building: %s", imagePath.c_str());
        auto draw = DrawNode::create();
        Color4F color = (team == Team::PLAYER) ? Color4F::BLUE : Color4F::RED;
        draw->drawSolidRect(Vec2(-20, -20), Vec2(20, 20), color);
        this->addChild(draw);
    }

    // 添加建筑名称标签
    auto label = Label::createWithSystemFont(_stats.name, "Arial", 12);
    label->setPosition(Vec2(0, -25));
    label->setColor(Color3B::WHITE);
    this->addChild(label, 1);

    this->scheduleUpdate();

    return true;
}
//    // Visual representation (Placeholder)
//    auto draw = DrawNode::create();
//    Color4F color = (team == Team::PLAYER) ? Color4F::BLUE : Color4F::RED;
//    draw->drawSolidRect(Vec2(-20, -20), Vec2(20, 20), color);
//    this->addChild(draw);
//    
//    // Add a label for type
//    auto label = Label::createWithSystemFont(_stats.name, "Arial", 10);
//    label->setPosition(Vec2(0, 0));
//    this->addChild(label);
//    
//    this->scheduleUpdate();
//    
//    return true;
//}

BattleBuildingType BattleBuilding::getType() const {
    return _type;
}

//void BattleBuilding::update(float dt) {
//    // Defense logic
//    if (_stats.damage > 0) {
//        if (_target && !_target->isDead()) {
//            float dist = this->getPosition().distance(_target->getPosition());
//            if (dist <= _stats.attackRange) {
//                _attackTimer += dt;
//                if (_attackTimer >= _stats.attackSpeed) {
//                    _attackTimer = 0;
//                    _target->takeDamage(_stats.damage);
//                    // Visualize shot
//                    auto shot = DrawNode::create();
//                    shot->drawLine(Vec2::ZERO, _target->getPosition() - this->getPosition(), Color4F::YELLOW);
//                    this->addChild(shot);
//                    shot->runAction(Sequence::create(DelayTime::create(0.1f), RemoveSelf::create(), nullptr));
//                }
//            }
//            else {
//                _target = nullptr; // Lost range
//            }
//        }
//    }
//}

void BattleBuilding::update(float dt) {
    // 防御逻辑（只针对有攻击力的建筑）
    if (_stats.damage > 0) {
        if (_target && !_target->isDead()) {
            float dist = this->getPosition().distance(_target->getPosition());
            if (dist <= _stats.attackRange) {
                // 在攻击范围内
                _attackTimer += dt;
                if (_attackTimer >= _stats.attackSpeed) {
                    _attackTimer = 0;
                    _target->takeDamage(_stats.damage);

                    // 【修改】可视化射击效果
                    auto shot = DrawNode::create();
                    shot->drawLine(Vec2::ZERO, _target->getPosition() - this->getPosition(), Color4F::YELLOW);
                    shot->setLineWidth(2.0f);
                    this->addChild(shot);
                    shot->runAction(Sequence::create(
                        DelayTime::create(0.1f),
                        RemoveSelf::create(),
                        nullptr
                    ));

                    CCLOG("%s attacked %s for %d damage",
                        _stats.name.c_str(),
                        _target->getName().c_str(),
                        _stats.damage);
                }
            }
            else {
                _target = nullptr; // 超出范围，丢失目标
            }
        }
    }
}


// ========== 辅助函数 ==========
std::string BattleBuilding::getImagePathForBattleBuilding(BattleBuildingType type, Team team) {
    // 根据建筑类型选择图片
    switch (type) {
    case BattleBuildingType::TOWN_HALL:
        return "BattleBuildings/Town_Hall3.png";
    case BattleBuildingType::GOLD_MINE:
        return "BattleBuildings/Gold_Mine1.png";
    case BattleBuildingType::ELIXIR_COLLECTOR:
        return "BattleBuildings/Elixir_Collector2.png";
    case BattleBuildingType::CANNON:
        return "BattleBuildings/Cannon1.png";
    case BattleBuildingType::ARCHER_TOWER:
        return "BattleBuildings/archer_tower.png";
    case BattleBuildingType::GOLD_STORAGE:
        return "BattleBuildings/Gold_Storage1.png";
    case BattleBuildingType::ELIXIR_STORAGE:
        return "BattleBuildings/Elixir_Storage.png";
    case BattleBuildingType::WALL:
        return "BattleBuildings/Wall.png";
    case BattleBuildingType::ARMY_CAMP:
        return "BattleBuildings/army_camp.png";
    case BattleBuildingType::BARRACKS:
        return "BattleBuildings/Barracks.png";
    default:
        return "BattleBuildings/building_default.png";
    }
}

float BattleBuilding::getScaleForBattleBuilding(BattleBuildingType type) {
    // 根据建筑类型调整显示大小
    switch (type) {
    case BattleBuildingType::TOWN_HALL:
        return 0.4f; // 主城大一些
    case BattleBuildingType::CANNON:
        return 0.4f;
    case BattleBuildingType::ARCHER_TOWER:
        return 0.1f;
    case BattleBuildingType::WALL:
        return 0.1f; // 城墙小一些
    default:
        return 0.3f; // 默认大小
    }
}

void BattleBuilding::setTarget(Entity* target) {
    _target = target;
}

Entity* BattleBuilding::getTarget() const {
    return _target;
}
