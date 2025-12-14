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
        if (sInstance && sInstance->init("Map/TEST.tmx")) {
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

    CCLOG("Map loaded successfully:");
    CCLOG("- Map Size: %.0f x %.0f", tileMap->getMapSize().width, tileMap->getMapSize().height);
    CCLOG("- Tile Size: %.0f x %.0f", tileMap->getTileSize().width, tileMap->getTileSize().height);
    CCLOG("- Content Size: %.2f x %.2f", tileMap->getContentSize().width, tileMap->getContentSize().height);

    // 借鉴BeachMap：设置地图在屏幕中的初始位置
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Size mapContentSize = tileMap->getContentSize();

    // 将地图居中放置（可选）
    Vec2 initialPos = Vec2(
        (visibleSize.width - mapContentSize.width) / 2,
        (visibleSize.height - mapContentSize.height) / 2
    );
    tileMap->setPosition(initialPos);

    CCLOG("Map positioned at: (%.2f, %.2f)", initialPos.x, initialPos.y);

    // 获取特定层
    backgroundLayer = getLayer("Background");
    grassLayer = getLayer("Grass");

    if (!backgroundLayer) {
        CCLOG("Background layer not found!");
    }
    else {
        CCLOG("Background layer found: %s", backgroundLayer->getLayerName().c_str());
    }

    if (!grassLayer) {
        CCLOG("Grass layer not found!");
    }
    else {
        CCLOG("Grass layer found: %s", grassLayer->getLayerName().c_str());

        // 借鉴BeachMap的做法：遍历所有瓦片，检测地图数据
        Size mapSize = tileMap->getMapSize();
        int backgroundTileCount = 0;
        int grassTileCount = 0;

        for (int y = 0; y < mapSize.height; y++) {
            for (int x = 0; x < mapSize.width; x++) {
                if (backgroundLayer && backgroundLayer->getTileGIDAt(Vec2(x, y)) != 0) {
                    backgroundTileCount++;
                }
                if (grassLayer && grassLayer->getTileGIDAt(Vec2(x, y)) != 0) {
                    grassTileCount++;
                }
            }
        }

        CCLOG("Background tiles found: %d", backgroundTileCount);
        CCLOG("Grass tiles found: %d", grassTileCount);
    }

    // 调试子节点数量
    CCLOG("TileMap children count: %d", (int)tileMap->getChildren().size());

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

    // 检查建筑占用的所有瓦片是否都在草地上
    for (int x = 0; x < tilesX; x++) {
        for (int y = 0; y < tilesY; y++) {
            int tileX = startTileX + x;
            int tileY = startTileY + y;

            // 检查边界（使用瓦片坐标边界）
            if (tileX < 0 || tileX >= mapSize.width || tileY < 0 || tileY >= mapSize.height) {
                return false; // 超出边界
            }

            // 直接使用瓦片坐标检查，不需要转换
            unsigned int gid = grassLayer->getTileGIDAt(Vec2(tileX, tileY));
            if (gid == 0) {  // 0表示没有瓦片，即不是草地
                return false;
            }
        }
    }

    return true;
}

