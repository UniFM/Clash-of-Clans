/*************************************************************
* @file     : HomeVillageMap.cpp
* @function ：家乡基地地图实现
* @author   : 叶芷含
* @note     ：实现家乡基地特有的地图功能
**************************************************************/

#include "HomeVillageMap.h"

USING_NS_CC;

// 静态实例指针初始化
HomeVillageMap* HomeVillageMap::sInstance = nullptr;

// 构造函数
HomeVillageMap::HomeVillageMap()
    : backgroundLayer(nullptr)
    , grassLayer(nullptr)
    , isPlacingBuilding(false)
    , currentBuildingType(BuildingType::CANNON)
    , buildingPreview(nullptr)
{
}

// 析构函数
HomeVillageMap::~HomeVillageMap() {
    // 清空静态实例指针
    if (sInstance == this) {
        sInstance = nullptr;
    }
}

// 获取单例实例
HomeVillageMap* HomeVillageMap::getInstance() {
    if (!sInstance) {
        sInstance = new (std::nothrow) HomeVillageMap();
        if (sInstance && sInstance->init(ResPath::TMX_HOMEVILLAGEMAP)) {
            sInstance->autorelease();
        }
        else {
            CC_SAFE_DELETE(sInstance);
        }
    }
    return sInstance;
}

// 初始化地图
bool HomeVillageMap::init(const std::string& tmxFile) {
    if (!SceneMap::init(tmxFile)) {
        CCLOG("Failed to init SceneMap with file: %s", tmxFile.c_str());
        return false;
    }

    // 调试地图加载
    if (!tileMap) {
        CCLOG("TileMap is null!");
        return false;
    }

    // 先获取特定层
    backgroundLayer = getLayer("Background");
    grassLayer = getLayer("Grass");

    // 获取屏幕和地图尺寸
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Size mapContentSize = tileMap->getContentSize();

    // 修正的地图位置设置逻辑
    Vec2 initialPos;

    initialPos.x = 0;
    initialPos.y = 0;

    tileMap->setPosition(initialPos);

    // 重置地图的变换属性，确保没有旋转或扭曲
    tileMap->setRotation(0);
    tileMap->setScaleX(1.0f);
    tileMap->setScaleY(1.0f);
    tileMap->setSkewX(0);
    tileMap->setSkewY(0);

    // 设置滚动视图
    setupScrollView();

    return true;
}

// 重写建筑放置检测，添加草地检测
bool HomeVillageMap::canPlaceBuilding(const Vec2& pos, const Size& buildingSize) const {
    // 首先进行基础检测
    if (!SceneMap::canPlaceBuilding(pos, buildingSize)) {
        return false;
    }

    // 检查是否在草地上
    return isOnGrassland(pos, buildingSize);
}

// 检查是否完全在草地上
bool HomeVillageMap::isOnGrassland(const Vec2& pos, const Size& buildingSize) const {
    if (!grassLayer || !tileMap) {
        return false;
    }

    // 将世界坐标转换为瓦片坐标
    Size tileSize = tileMap->getTileSize();
    Size mapSize = tileMap->getMapSize();

    // 计算建筑占用的瓦片范围
    int startTileX = static_cast<int>(pos.x / tileSize.width);
    int startTileY = static_cast<int>(pos.y / tileSize.height);

    int tilesX = static_cast<int>(std::ceil(buildingSize.width / tileSize.width));
    int tilesY = static_cast<int>(std::ceil(buildingSize.height / tileSize.height));

    CCLOG("建筑占用瓦片数: %d x %d", tilesX, tilesY);

    // 检查建筑占用的所有瓦片是否都在草地上
    for (int x = 0; x < tilesX; x++) {
        for (int y = 0; y < tilesY; y++) {
            int tileX = startTileX + x;
            int tileY = startTileY + y;

            // 检查边界（使用瓦片坐标边界）
            if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
                CCLOG("瓦片坐标超出边界: (%d, %d)", tileX, tileY);
                return false; // 超出边界
            }

            // 直接使用瓦片坐标检查，不需要转换
            unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
            if (gid == 0) {  // 0表示没有瓦片，即不是草地
                CCLOG("位置(%d, %d)不是草地，GID: %u", tileX, tileY, gid);
                return false;
            }
            else {
                CCLOG("位置(%d, %d)是草地，GID: %u", tileX, tileY, gid);
            }
        }
    }

    CCLOG("所有瓦片都在草地上");
    return true;
}

// 开始建筑放置模式
void HomeVillageMap::startBuildingPlacement(BuildingType buildingType)
{
    CCLOG("Starting building placement mode for building type: %d", static_cast<int>(buildingType));
    
    isPlacingBuilding = true;
    currentBuildingType = buildingType;
    
    // 创建建筑预览精灵
    std::string spritePath;
    switch (buildingType) {
        case BuildingType::CANNON:
            spritePath = ResPath::CANNONLEVEL1;
            break;
        case BuildingType::TOWN_HALL:
            spritePath = ResPath::TOWNHALLLEVEL1;
            break;
        default:
            // 可以设置默认预览图片或从BuildingConfig获取
            const BuildingLevelStats* stats = BuildingConfig::getStats(buildingType, 1);
            if (stats) {
                spritePath = stats->spriteName;
            }
            break;
    }
    
    buildingPreview = Sprite::create(spritePath);
    if (!buildingPreview) {
        // 如果加载失败，创建占位符
        buildingPreview = Sprite::create();
        auto placeholder = LayerColor::create(Color4B(100, 200, 100, 128), 64, 64);
        buildingPreview->addChild(placeholder);
        buildingPreview->setContentSize(Size(64, 64));
    }
    
    // 设置预览建筑的透明度，表示这是预览状态
    buildingPreview->setOpacity(150);
    buildingPreview->setVisible(false); // 初始时隐藏，等触摸时显示
    this->addChild(buildingPreview, 100); // 高层级确保在最上层
    
    // 创建建筑放置的触摸监听器
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    
    touchListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        if (isPlacingBuilding) {
            Vec2 touchPos = touch->getLocation();
            this->onTouchBeganForBuilding(touchPos);
            return true;
        }
        return false;
    };
    
    touchListener->onTouchMoved = [this](Touch* touch, Event* event) {
        if (isPlacingBuilding) {
            Vec2 touchPos = touch->getLocation();
            this->onTouchMovedForBuilding(touchPos);
        }
    };
    
    touchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (isPlacingBuilding) {
            Vec2 touchPos = touch->getLocation();
            this->onTouchEndedForBuilding(touchPos);
        }
    };
    
    // 添加监听器，使用高优先级确保优先处理建筑放置
    _eventDispatcher->addEventListenerWithFixedPriority(touchListener, -1);
    
    // 保存监听器引用以便后续移除
    buildingPreview->setUserData(touchListener);
    
    CCLOG("Building placement mode started successfully");
}

// 结束建筑放置模式
void HomeVillageMap::endBuildingPlacement()
{
    CCLOG("Ending building placement mode");
    
    isPlacingBuilding = false;
    
    if (buildingPreview) {
        // 移除触摸监听器
        auto touchListener = static_cast<EventListenerTouchOneByOne*>(buildingPreview->getUserData());
        if (touchListener) {
            _eventDispatcher->removeEventListener(touchListener);
        }
        
        // 移除预览精灵
        buildingPreview->removeFromParent();
        buildingPreview = nullptr;
    }
    
    CCLOG("Building placement mode ended");
}

// 处理建筑放置的触摸开始
void HomeVillageMap::onTouchBeganForBuilding(const Vec2& touchPos)
{
    if (!buildingPreview) return;
    
    buildingPreview->setVisible(true);
    updateBuildingPreview(touchPos);
    
    CCLOG("Building placement touch began at (%.1f, %.1f)", touchPos.x, touchPos.y);
}

// 处理建筑放置的触摸移动
void HomeVillageMap::onTouchMovedForBuilding(const Vec2& touchPos)
{
    updateBuildingPreview(touchPos);
}

// 处理建筑放置的触摸结束
void HomeVillageMap::onTouchEndedForBuilding(const Vec2& touchPos)
{
    CCLOG("Building placement touch ended at (%.1f, %.1f)", touchPos.x, touchPos.y);
    
    // 转换触摸位置到地图坐标
    Vec2 mapPos = this->convertToNodeSpace(touchPos);
    
    // 检查是否可以在此位置放置建筑
    const BuildingData* buildingData = BuildingConfig::getBuildingData(currentBuildingType);
    if (buildingData) {
        Size buildingSize(buildingData->gridWidth * 16, buildingData->gridHeight * 16); // 假设每格16像素
        
        if (canPlaceBuilding(mapPos, buildingSize)) {
            // 确认放置建筑
            placeBuildingAtPosition(mapPos);
            endBuildingPlacement();
        } else {
            CCLOG("Cannot place building at this position");
            // 可以在这里添加错误提示
        }
    }
}

// 更新建筑预览位置和状态
void HomeVillageMap::updateBuildingPreview(const Vec2& worldPos)
{
    if (!buildingPreview) return;
    
    // 转换世界坐标到地图坐标
    Vec2 mapPos = this->convertToNodeSpace(worldPos);
    
    // 设置预览精灵位置
    buildingPreview->setPosition(mapPos);
    
    // 检查当前位置是否可以放置建筑
    const BuildingData* buildingData = BuildingConfig::getBuildingData(currentBuildingType);
    if (buildingData) {
        Size buildingSize(buildingData->gridWidth * 16, buildingData->gridHeight * 16);
        
        if (canPlaceBuilding(mapPos, buildingSize)) {
            // 可以放置 - 绿色透明
            buildingPreview->setColor(Color3B(100, 255, 100));
        } else {
            // 不可以放置 - 红色透明
            buildingPreview->setColor(Color3B(255, 100, 100));
        }
    }
}

// 确认放置建筑
void HomeVillageMap::placeBuildingAtPosition(const Vec2& pos)
{
    CCLOG("Placing building type %d at position (%.1f, %.1f)", 
          static_cast<int>(currentBuildingType), pos.x, pos.y);
    
    // 创建实际的建筑精灵
    std::string spritePath;
    switch (currentBuildingType) {
        case BuildingType::CANNON:
            spritePath = ResPath::CANNONLEVEL1;
            break;
        case BuildingType::TOWN_HALL:
            spritePath = ResPath::TOWNHALLLEVEL1;
            break;
        default:
            const BuildingLevelStats* stats = BuildingConfig::getStats(currentBuildingType, 1);
            if (stats) {
                spritePath = stats->spriteName;
            }
            break;
    }
    
    auto buildingSprite = Sprite::create(spritePath);
    if (!buildingSprite) {
        // 创建占位符
        buildingSprite = Sprite::create();
        auto placeholder = LayerColor::create(Color4B(100, 150, 200, 255), 64, 64);
        buildingSprite->addChild(placeholder);
        buildingSprite->setContentSize(Size(64, 64));
    }
    
    buildingSprite->setPosition(pos);
    this->addChild(buildingSprite, 5); // 在地图之上，但比UI低
    
    CCLOG("Building placed successfully!");
    
    // TODO: 在这里可以添加其他逻辑，如：
    // 1. 更新游戏数据
    // 2. 扣除资源
    // 3. 保存建筑到存档系统
    // 4. 播放放置音效
}

