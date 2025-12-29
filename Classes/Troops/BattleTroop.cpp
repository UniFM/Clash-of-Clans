#include "BattleTroop.h"
#include "Control/BattleManager.h"
#include "buildings/BattleBuilding.h"

USING_NS_CC;


BattleTroop* BattleTroop::create(BattleTroopType type, Team team) {
    BattleTroop* pRet = new(std::nothrow) BattleTroop();
    if (pRet && pRet->init(type, team)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool BattleTroop::init(BattleTroopType type, Team team) {
    if (!Entity::init()) return false;

    _type = type;
    _stats = BattleManager::getInstance()->getBattleTroopStats(type);
    setStats(_stats.maxHealth, team);

    _target = nullptr;
    _attackTimer = 0.0f;

    // ========== 修改这里：用图片代替圆形 ==========
// 获取士兵图片路径
    std::string imagePath = getImagePathForBattleTroop(type, team);
    auto sprite = Sprite::create(imagePath);

    if (sprite) {
        // 设置锚点
        sprite->setAnchorPoint(Vec2(0.5f, 0.0f));

        sprite->setScale(0.5f);

        this->addChild(sprite, 0);

        // 保存精灵指针，如果需要的话
        _sprite = sprite;  // 注意：Entity类中已经有一个_sprite成员变量

    }
    else {
        // 如果图片加载失败，使用原来的圆形作为备用
        CCLOG("Failed to load image for troop: %s", imagePath.c_str());
        auto draw = DrawNode::create();
        Color4F color = (team == Team::PLAYER) ? Color4F::GREEN : Color4F::MAGENTA;
        draw->drawSolidCircle(Vec2::ZERO, 10, 0, 10, color);
        this->addChild(draw);
    }

    // 修改标签位置，确保在图片上方
    auto label = Label::createWithSystemFont(_stats.name, "Arial", 8);
    label->setPosition(Vec2(0, 15));  // 向上调整，避免与图片重叠
    label->setColor(Color3B::WHITE);
    this->addChild(label);

    this->scheduleUpdate();

    return true;
}

void BattleTroop::update(float dt) {
    if (isDead()) return;

    if (_target && !_target->isDead()) {
        float dist = this->getPosition().distance(_target->getPosition());

        if (dist <= _stats.attackRange) {
            // 攻击状态
            _attackTimer += dt;
            if (_attackTimer >= _stats.attackSpeed) {
                _attackTimer = 0;
                _target->takeDamage(_stats.damage);
            }
        }
        else {
            // 移动状态 - 简化版本，不需要动画切换
            Vec2 direction = (_target->getPosition() - this->getPosition()).getNormalized();
            Vec2 newPos = this->getPosition() + direction * _stats.moveSpeed * dt;
            this->setPosition(newPos);
        }
    }
    else {
        _target = nullptr;
        // 没有目标时不需要动画
    }
}

void BattleTroop::setTarget(Entity* target) {
    _target = target;
}

Entity* BattleTroop::getTarget() const {
    return _target;
}

//void BattleTroop::findNewTarget(const cocos2d::Vector<Entity*>& potentialTargets) {
//    float minDist = FLT_MAX;
//    Entity* bestTarget = nullptr;
//
//    for (auto entity : potentialTargets) {
//        if (entity->getTeam() != this->getTeam() && !entity->isDead()) {
//            // Special logic: Giants prefer Defenses
//            if (_type == BattleTroopType::GIANT) {
//                BattleBuilding* b = dynamic_cast<BattleBuilding*>(entity);
//                // Check if building is defense (Cannon, Archer Tower)
//                bool isDefense = false;
//                if (b) {
//                    BattleBuildingType bt = b->getType();
//                    if (bt == BattleBuildingType::CANNON || bt == BattleBuildingType::ARCHER_TOWER) {
//                        isDefense = true;
//                    }
//                }
//
//                if (isDefense) {
//                    float dist = this->getPosition().distance(entity->getPosition());
//                    if (dist < minDist) {
//                        minDist = dist;
//                        bestTarget = entity;
//                    }
//                }
//            }
//            else {
//                float dist = this->getPosition().distance(entity->getPosition());
//                if (dist < minDist) {
//                    minDist = dist;
//                    bestTarget = entity;
//                }
//            }
//        }
//    }
//    // Fallback for Giants if no defense found: Attack closest anything
//    if (_type == BattleTroopType::GIANT && bestTarget == nullptr) {
//        for (auto entity : potentialTargets) {
//            if (entity->getTeam() != this->getTeam() && !entity->isDead()) {
//                float dist = this->getPosition().distance(entity->getPosition());
//                if (dist < minDist) {
//                    minDist = dist;
//                    bestTarget = entity;
//                }
//            }
//        }
//    }
//
//    _target = bestTarget;
//}
// 
// 

void BattleTroop::findNewTarget(const cocos2d::Vector<Entity*>& potentialTargets) {
    float minDist = FLT_MAX;
    Entity* bestTarget = nullptr;

    // 存储所有潜在目标的向量
    Vector<Entity*> allEnemies;
    Vector<Entity*> preferredEnemies;  // 偏好目标
    Vector<Entity*> resourceBuildings; // 资源建筑

    // 第一步：分类所有敌人
    for (auto entity : potentialTargets) {
        if (entity->getTeam() != this->getTeam() && !entity->isDead()) {
            allEnemies.pushBack(entity);

            BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
            if (building) {
                BattleBuildingType bt = building->getType();

                // 检查是否是资源建筑
                if (bt == BattleBuildingType::GOLD_STORAGE ||
                    bt == BattleBuildingType::ELIXIR_COLLECTOR ||
                    bt == BattleBuildingType::GOLD_MINE) {
                    resourceBuildings.pushBack(entity);
                }

                // 检查是否是防御建筑
                if (bt == BattleBuildingType::CANNON ||
                    bt == BattleBuildingType::ARCHER_TOWER) {
                    // 防御建筑可能已经是偏好目标
                }
            }
        }
    }

    // 第二步：根据兵种类型选择偏好目标
    switch (_type) {
    case BattleTroopType::GIANT:
        // 巨人：优先攻击防御建筑
        for (auto entity : allEnemies) {
            BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
            if (building) {
                BattleBuildingType bt = building->getType();
                if (bt == BattleBuildingType::CANNON ||
                    bt == BattleBuildingType::ARCHER_TOWER) {
                    preferredEnemies.pushBack(entity);
                }
            }
        }
        break;

    case BattleTroopType::GOBLIN:
        // 哥布林：优先攻击资源建筑
        preferredEnemies = resourceBuildings;
        break;

    case BattleTroopType::BARBARIAN:
        // 野蛮人：攻击最近的任何目标（无特殊偏好）
        preferredEnemies = allEnemies;
        break;

    case BattleTroopType::ARCHER:
        // 弓箭手：优先攻击防御建筑，其次是其他建筑
        for (auto entity : allEnemies) {
            BattleBuilding* building = dynamic_cast<BattleBuilding*>(entity);
            if (building) {
                preferredEnemies.pushBack(entity);
            }
        }
        // 如果没有建筑目标，则攻击所有敌人
        if (preferredEnemies.empty()) {
            preferredEnemies = allEnemies;
        }
        break;

    default:
        preferredEnemies = allEnemies;
        break;
    }

    // 第三步：从偏好目标中选择最近的
    if (!preferredEnemies.empty()) {
        for (auto entity : preferredEnemies) {
            float dist = this->getPosition().distance(entity->getPosition());
            if (dist < minDist) {
                minDist = dist;
                bestTarget = entity;
            }
        }
    }

    // 第四步：如果没有找到偏好目标，从所有敌人中选择
    if (bestTarget == nullptr && !allEnemies.empty()) {
        minDist = FLT_MAX;
        for (auto entity : allEnemies) {
            float dist = this->getPosition().distance(entity->getPosition());
            if (dist < minDist) {
                minDist = dist;
                bestTarget = entity;
            }
        }
    }

    _target = bestTarget;
}

// ========== 添加图片路径函数 ==========
std::string BattleTroop::getImagePathForBattleTroop(BattleTroopType type, Team team) {
    // 根据兵种类型返回图片路径
    switch (type) {
    case BattleTroopType::BARBARIAN:
        return "Troops/Barbarian.png";
    case BattleTroopType::ARCHER:
        return "Troops/Archer.png";
    case BattleTroopType::GIANT:
        return "Troops/Giant.png";
    case BattleTroopType::GOBLIN:
        return "Troops/Goblin.png";
    default:
        return "Troops/troop_default.png";
    }
}