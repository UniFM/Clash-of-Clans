#include "Building.h"

Building::Building()
    : _type(BuildingType::TOWN_HALL)
    , _level(1)
    , _maxLevel(3)
    , _hp(100)
    , _maxHP(100)
    , _gridX(0)
    , _gridY(0)
{
}

Building::~Building()
{
}

Building* Building::create(BuildingType type, int level)
{
    Building* building = new (std::nothrow) Building();
    if (building && building->init(type, level))
    {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool Building::init(BuildingType type, int level)
{
    _type = type;
    _level = level;
    _maxLevel = 3;

    updateProperties();

    std::string imagePath = getImagePath();
    if (!Sprite::initWithFile(imagePath))
    {
        // 如果图片不存在，创建一个占位符
        if (!Sprite::init())
        {
            return false;
        }
        // 创建一个简单的彩色矩形作为占位符
        auto drawNode = DrawNode::create();
        drawNode->drawSolidRect(Vec2(-30, -30), Vec2(30, 30), Color4F(0.5f, 0.5f, 0.5f, 1.0f));
        this->addChild(drawNode);
        this->setContentSize(Size(60, 60));
    }

    return true;
}

void Building::setGridPosition(int gridX, int gridY)
{
    _gridX = gridX;
    _gridY = gridY;

    // 注意：实际的世界坐标位置需要由MapLayer来设置
    // 这里只保存网格坐标
}

bool Building::upgrade()
{
    if (!canUpgrade())
        return false;

    _level++;
    updateProperties();

    // 更新显示
    std::string imagePath = getImagePath();
    auto texture = Director::getInstance()->getTextureCache()->addImage(imagePath);
    if (texture)
    {
        this->setTexture(texture);
    }

    return true;
}

bool Building::canUpgrade() const
{
    return _level < _maxLevel;
}

void Building::takeDamage(int damage)
{
    _hp -= damage;
    if (_hp < 0)
        _hp = 0;

    // 可以添加受伤动画效果
    if (_hp > 0)
    {
        // 闪烁效果
        auto blink = Blink::create(0.2f, 2);
        this->runAction(blink);
    }
}

void Building::updateProperties()
{
    // 根据类型和等级设置属性
    switch (_type)
    {
    case BuildingType::TOWN_HALL:
        _maxHP = 500 + (_level - 1) * 200;
        break;
    case BuildingType::GOLD_MINE:
    case BuildingType::ELIXIR_COLLECTOR:
        _maxHP = 100 + (_level - 1) * 50;
        break;
    case BuildingType::BARRACKS:
        _maxHP = 200 + (_level - 1) * 100;
        break;
    case BuildingType::ARCHER_TOWER:
    case BuildingType::CANNON:
        _maxHP = 150 + (_level - 1) * 75;
        break;
    case BuildingType::GOLD_STORAGE:
    case BuildingType::ELIXIR_STORAGE:
        _maxHP = 300 + (_level - 1) * 150;
        break;
    }

    if (_hp == 0 || _hp > _maxHP)
        _hp = _maxHP;
}

std::string Building::getImagePath() const
{
    // 根据建筑类型和等级返回图片路径
    // 这里使用占位符路径，实际项目中应该使用真实的图片资源
    std::string typeStr;
    switch (_type)
    {
    case BuildingType::TOWN_HALL: typeStr = "townhall"; break;
    case BuildingType::GOLD_MINE: typeStr = "goldmine"; break;
    case BuildingType::ELIXIR_COLLECTOR: typeStr = "elixircollector"; break;
    case BuildingType::BARRACKS: typeStr = "barracks"; break;
    case BuildingType::ARCHER_TOWER: typeStr = "archertower"; break;
    case BuildingType::CANNON: typeStr = "cannon"; break;
    case BuildingType::GOLD_STORAGE: typeStr = "goldstorage"; break;
    case BuildingType::ELIXIR_STORAGE: typeStr = "elixirstorage"; break;
    }

    return StringUtils::format("buildings/%s_lv%d.png", typeStr.c_str(), _level);
}

