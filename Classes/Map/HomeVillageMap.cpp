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

    // 添加地图属性调试
    CCLOG("Map loaded successfully:");
    CCLOG("- Map Size: %.0f x %.0f", tileMap->getMapSize().width, tileMap->getMapSize().height);
    CCLOG("- Tile Size: %.0f x %.0f", tileMap->getTileSize().width, tileMap->getTileSize().height);
    CCLOG("- Content Size: %.2f x %.2f", tileMap->getContentSize().width, tileMap->getContentSize().height);
    CCLOG("- Map Orientation: %d", tileMap->getMapOrientation());

    // 先获取特定层
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
    }

    // 获取屏幕和地图尺寸
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Size mapContentSize = tileMap->getContentSize();

    CCLOG("=== 尺寸调试信息 ===");
    CCLOG("屏幕可见尺寸: %.2f x %.2f", visibleSize.width, visibleSize.height);
    CCLOG("地图内容尺寸: %.2f x %.2f", mapContentSize.width, mapContentSize.height);

    // 修正的地图位置设置逻辑
    Vec2 initialPos;

    // 对于大地图，应该将地图放在原点，通过ScrollView控制可视区域
    // 这里先简单设置为原点，让地图完全可见
    initialPos.x = 0;
    initialPos.y = 0;

    CCLOG("设置地图位置为原点: (%.2f, %.2f)", initialPos.x, initialPos.y);
    CCLOG("地图尺寸: %.2f x %.2f, 屏幕尺寸: %.2f x %.2f",
        mapContentSize.width, mapContentSize.height,
        visibleSize.width, visibleSize.height);

    tileMap->setPosition(initialPos);

    // 重置地图的变换属性，确保没有旋转或扭曲
    tileMap->setRotation(0);
    tileMap->setScaleX(1.0f);
    tileMap->setScaleY(1.0f);
    tileMap->setSkewX(0);
    tileMap->setSkewY(0);

    // 检查地图是否有异常的变换
    CCLOG("=== 地图变换检查 ===");
    CCLOG("Rotation: %.2f", tileMap->getRotation());
    CCLOG("Scale: (%.2f, %.2f)", tileMap->getScaleX(), tileMap->getScaleY());
    CCLOG("Skew: (%.2f, %.2f)", tileMap->getSkewX(), tileMap->getSkewY());
    CCLOG("Anchor Point: (%.2f, %.2f)", tileMap->getAnchorPoint().x, tileMap->getAnchorPoint().y);

    CCLOG("Map positioned at: (%.2f, %.2f)", initialPos.x, initialPos.y);

    // 调试子节点数量
    CCLOG("TileMap children count: %d", (int)tileMap->getChildren().size());

    // 遍历所有瓦片，检测地图数据
    if (backgroundLayer && grassLayer) {
        Size mapSize = tileMap->getMapSize();
        int backgroundTileCount = 0;
        int grassTileCount = 0;

        for (int y = 0; y < mapSize.height; y++) {
            for (int x = 0; x < mapSize.width; x++) {
                if (backgroundLayer->getTileGIDAt(Vec2(x, y)) != 0) {
                    backgroundTileCount++;
                }
                if (grassLayer->getTileGIDAt(Vec2(x, y)) != 0) {
                    grassTileCount++;
                }
            }
        }

        CCLOG("Background tiles found: %d", backgroundTileCount);
        CCLOG("Grass tiles found: %d", grassTileCount);
    }

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

    // 添加调试信息
    CCLOG("=== 坐标转换调试 ===");
    CCLOG("世界坐标: (%.2f, %.2f)", pos.x, pos.y);
    CCLOG("瓦片尺寸: %.2f x %.2f", tileSize.width, tileSize.height);

    // 计算建筑占用的瓦片范围
    int startTileX = static_cast<int>(pos.x / tileSize.width);
    int startTileY = static_cast<int>(pos.y / tileSize.height);

    // 添加坐标转换的调试
    CCLOG("转换后瓦片坐标: (%d, %d)", startTileX, startTileY);

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

    CCLOG("建筑完全在草地上");
    return true;
}
