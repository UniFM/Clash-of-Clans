/*************************************************************
* @file     : BattleTroop.cpp
* @function ：战斗兵种核心实现 - 兵种AI+动画系统+攻击逻辑
* @author   : 俞筱雨 叶芷含
* @note     : 1. 实现兵种创建、初始化及帧更新，适配不同兵种属性；
*             2. 支持闲置/移动/攻击动画切换，集成攻击特效可视化；
*             3. 差异化AI目标选择，适配兵种特性（巨人/哥布林等）；
*             4. 适配Entity基类，完成血量、伤害、移动寻路等核心逻辑。
**************************************************************/


#include "BattleTroop.h"
#include "Control/BattleManager.h"
#include "buildings/BattleBuilding.h"
#include "Constant/Constant.h"

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
    setEntityName(_stats.name);

    _target = nullptr;
    _attackTimer = 0.0f;
    _currentAnimationState = AnimationState::IDLE; // 【新增】初始化动画状态

    // 获取士兵图片路径
    std::string imagePath = getImagePathForBattleTroop(type, team);
    auto sprite = Sprite::create(imagePath);

    if (sprite) {
        // 设置锚点
        sprite->setAnchorPoint(Vec2(0.5f, 0.0f));
        sprite->setScale(0.5f);
        this->addChild(sprite, 0);

        // 保存精灵指针
        _sprite = sprite;
    }
    else {
        // 如果图片加载失败，使用原来的圆形作为备用
        CCLOG("Failed to load image for troop: %s", imagePath.c_str());
        auto draw = DrawNode::create();
        Color4F color = (team == Team::PLAYER) ? Color4F::GREEN : Color4F::MAGENTA;
        draw->drawSolidCircle(Vec2::ZERO, 10, 0, 10, color);
        this->addChild(draw);
    }

    //// 修改标签位置，确保在图片上方
    //auto label = Label::createWithSystemFont(_stats.name, "Arial", 8);
    //label->setPosition(Vec2(0, 15));
    //label->setColor(Color3B::WHITE);
    //this->addChild(label);

    // 【新增】开始播放闲置动画
    runIdleAnimation();

    this->scheduleUpdate();

    return true;
}

void BattleTroop::update(float dt) {
    if (isDead()) return;

    if (_target && !_target->isDead()) {
        float dist = this->getPosition().distance(_target->getPosition());

        if (dist <= _stats.attackRange) {
            // 攻击状态
            // 【新增】切换到攻击动画
            if (_currentAnimationState != AnimationState::ATTACKING) {
                runAttackAnimation();
                _currentAnimationState = AnimationState::ATTACKING;
            }
            
            _attackTimer += dt;
            if (_attackTimer >= _stats.attackSpeed) {
                _attackTimer = 0;
                _target->takeDamage(_stats.damage);
                
                // 可视化攻击效果
                showAttackEffect();
            }
        }
        else {
            // 移动状态
            // 【新增】切换到移动动画
            if (_currentAnimationState != AnimationState::MOVING) {
                runMoveAnimation();
                _currentAnimationState = AnimationState::MOVING;
            }
            
            Vec2 direction = (_target->getPosition() - this->getPosition()).getNormalized();
            Vec2 newPos = this->getPosition() + direction * _stats.moveSpeed * dt;
            this->setPosition(newPos);
            
            // 根据移动方向翻转精灵
            if (_sprite) {
                if (direction.x < 0) _sprite->setFlippedX(true);
                else if (direction.x > 0) _sprite->setFlippedX(false);
            }
        }
    }
    else {
        _target = nullptr;
        // 【新增】切换到闲置动画
        if (_currentAnimationState != AnimationState::IDLE) {
            runIdleAnimation();
            _currentAnimationState = AnimationState::IDLE;
        }
    }
}

// 【新增】播放闲置动画
void BattleTroop::runIdleAnimation() {
    if (!_sprite) return;
    
    // 停止当前动画
    _sprite->stopAllActions();
    
    // 获取动画帧
    Vector<SpriteFrame*> animFrames = getAnimationFrames("idle");
    
    if (animFrames.size() > 0) {
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.3f);
        auto animate = Animate::create(animation);
        auto repeatAction = RepeatForever::create(animate);
        repeatAction->setTag(100); // 闲置动画标签
        _sprite->runAction(repeatAction);
        CCLOG("%s: Playing idle animation with %d frames", _stats.name.c_str(), (int)animFrames.size());
    }
}

// 【新增】播放移动动画
void BattleTroop::runMoveAnimation() {
    if (!_sprite) return;
    
    // 停止当前动画
    _sprite->stopAllActions();
    
    // 获取动画帧
    Vector<SpriteFrame*> animFrames = getAnimationFrames("move");
    
    // 如果没有移动动画，使用闲置动画
    if (animFrames.size() == 0) {
        animFrames = getAnimationFrames("idle");
    }
    
    if (animFrames.size() > 0) {
        auto animation = Animation::createWithSpriteFrames(animFrames, 0.2f); // 移动动画稍快
        auto animate = Animate::create(animation);
        auto repeatAction = RepeatForever::create(animate);
        repeatAction->setTag(101); // 移动动画标签
        _sprite->runAction(repeatAction);
        CCLOG("%s: Playing move animation with %d frames", _stats.name.c_str(), (int)animFrames.size());
    }
}

// 【新增】播放攻击动画
void BattleTroop::runAttackAnimation() {
    if (!_sprite) return;
    
    // 停止当前动画
    _sprite->stopAllActions();
    
    // 获取动画帧
    Vector<SpriteFrame*> animFrames = getAnimationFrames("attack");
    
    // 如果没有攻击动画，使用闲置动画
    if (animFrames.size() == 0) {
        runIdleAnimation();
        return;
    }
    
    // 播放单次攻击动画，结束后回到闲置状态
    auto animation = Animation::createWithSpriteFrames(animFrames, 0.15f);
    auto animate = Animate::create(animation);
    auto callback = CallFunc::create([this]() {
        // 攻击动画结束后，根据当前状态决定下一个动画
        if (_target && !_target->isDead()) {
            float dist = this->getPosition().distance(_target->getPosition());
            if (dist <= _stats.attackRange) {
                // 仍在攻击范围内，继续攻击动画
                runAttackAnimation();
            } else {
                // 需要移动，切换到移动动画
                runMoveAnimation();
                _currentAnimationState = AnimationState::MOVING;
            }
        } else {
            // 没有目标，切换到闲置动画
            runIdleAnimation();
            _currentAnimationState = AnimationState::IDLE;
        }
    });
    auto sequence = Sequence::create(animate, callback, nullptr);
    sequence->setTag(102); // 攻击动画标签
    _sprite->runAction(sequence);
    CCLOG("%s: Playing attack animation with %d frames", _stats.name.c_str(), (int)animFrames.size());
}

// 【新增】获取动画帧
Vector<SpriteFrame*> BattleTroop::getAnimationFrames(const std::string& animationType) {
    Vector<SpriteFrame*> animFrames;
    
    // 获取基础路径
    std::string basePath = getBaseImagePath();
    
    // 加载3帧动画
    for (int i = 1; i <= 3; i++) {
        std::string framePath = basePath + "_" + animationType + "_" + std::to_string(i) + ".png";
        if (FileUtils::getInstance()->isFileExist(framePath)) {
            auto sprite = Sprite::create(framePath);
            if (sprite) {
                animFrames.pushBack(sprite->getSpriteFrame());
            }
        }
    }
    
    return animFrames;
}

// 【新增】获取基础图片路径（去掉扩展名）
std::string BattleTroop::getBaseImagePath() {
    std::string fullPath = getImagePathForBattleTroop(_type, getTeam());
    size_t lastDot = fullPath.find_last_of(".");
    if (lastDot == std::string::npos) return fullPath;
    return fullPath.substr(0, lastDot);
}

// 【新增】显示攻击特效
void BattleTroop::showAttackEffect() {
    if (!_target) return;
    
    // 创建攻击线条特效
    auto effectLine = DrawNode::create();
    Vec2 startPos = Vec2::ZERO;
    Vec2 endPos = _target->getPosition() - this->getPosition();
    
    Color4F effectColor;
    switch (_type) {
    case BattleTroopType::ARCHER:
        effectColor = Color4F::YELLOW; // 弓箭手用黄色箭矢
        break;
    case BattleTroopType::BARBARIAN:
        effectColor = Color4F::RED; // 野蛮人用红色剑光
        break;
    case BattleTroopType::GIANT:
        effectColor = Color4F::ORANGE; // 巨人用橙色重击
        break;
    case BattleTroopType::GOBLIN:
        effectColor = Color4F::GREEN; // 哥布林用绿色匕首
        break;
    default:
        effectColor = Color4F::WHITE;
        break;
    }
    
    effectLine->drawLine(startPos, endPos, effectColor);
    effectLine->setLineWidth(3.0f);
    this->addChild(effectLine);
    
    // 特效消失动画
    auto fadeOut = FadeOut::create(0.2f);
    auto remove = RemoveSelf::create();
    effectLine->runAction(Sequence::create(fadeOut, remove, nullptr));
    
    // 兵种攻击缩放特效
    if (_sprite) {
        auto scale1 = ScaleTo::create(0.1f, _sprite->getScale() * 1.2f);
        auto scale2 = ScaleTo::create(0.1f, _sprite->getScale());
        _sprite->runAction(Sequence::create(scale1, scale2, nullptr));
    }
}

void BattleTroop::setTarget(Entity* target) {
    _target = target;
}

Entity* BattleTroop::getTarget() const {
    return _target;
}

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

// 获取图片路径
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