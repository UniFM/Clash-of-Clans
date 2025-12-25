#include "HomeVillageMap.h"
#include "cocos2d.h"

using namespace cocos2d;

HomeVillageMap* HomeVillageMap::sInstance = nullptr;

HomeVillageMap::HomeVillageMap()
    : _hasPendingBuilding(false)
    , _pendingBuildingType(BuildingType::TOWN_HALL)
{
}

HomeVillageMap::~HomeVillageMap()
{
    CCLOG("HomeVillageMap destructor called");
    if (sInstance == this) {
        sInstance = nullptr;
    }
}

HomeVillageMap* HomeVillageMap::getInstance(const std::string& mapImagePath) {
    if (sInstance) {
        if (sInstance->getReferenceCount() == 0) {
            sInstance = nullptr;
        }
    }
    
    if (!sInstance) {
        sInstance = new (std::nothrow) HomeVillageMap();
        if (sInstance && sInstance->init(mapImagePath)) {
            sInstance->autorelease();
            sInstance->retain();
        }
        else {
            CC_SAFE_DELETE(sInstance);
        }
    } else {
        auto parent = sInstance->getParent();
        if (parent) {
            parent->removeChild(sInstance, false);
        }
    }
    return sInstance;
}

HomeVillageMap* HomeVillageMap::create(const std::string& mapImagePath)
{
    HomeVillageMap* homeVillageMap = new (std::nothrow) HomeVillageMap();
    if (homeVillageMap && homeVillageMap->init(mapImagePath))
    {
        homeVillageMap->autorelease();
        return homeVillageMap;
    }
    CC_SAFE_DELETE(homeVillageMap);
    return nullptr;
}

void HomeVillageMap::destroyInstance() {
    if (sInstance) {
        auto parent = sInstance->getParent();
        if (parent) {
            parent->removeChild(sInstance, false);
        }
        sInstance->release();
        sInstance = nullptr;
    }
}

bool HomeVillageMap::init(const std::string& mapImagePath)
{
    // Set map parameters specific to HomeVillage
    _mapWidth = 3460.0f;
    _mapHeight = 2480.0f;
    _grassRectWidth = 2400.0f;
    _grassRectHeight = 1800.0f;
    _grassOffsetX = 530.0f;
    _grassOffsetY = 252.0f;
    _gridCols = 44;
    _gridRows = 44;

    if (!BaseMap::init(mapImagePath))
    {
        return false;
    }
    
    // Additional initialization for HomeVillage if needed
    
    return true;
}

bool HomeVillageMap::onTouchBegan(Touch* touch, Event* event)
{
    // HomeVillageMap specific logic:
    // It seems the original code returned false to let GameScene handle touch if not dragging?
    // "Return false to let GameScene touch events wait"
    // But if we want map dragging, we should handle it.
    // The original code:
    // _lastTouchPos = touch->getLocation();
    // _isDragging = false; 
    // return false; 
    
    // If I return false here, onTouchMoved won't be called for this node.
    // So map dragging wouldn't work if it returns false.
    // Wait, the original code had:
    // return false;
    // AND:
    // void onTouchMoved(...) { if (!_isDragging) return; ... }
    
    // This implies that HomeVillageMap dragging was DISABLED in the original code I read?
    // Or maybe GameScene calls move manually?
    // Let's look at the original Read output again.
    
    /*
    bool HomeVillageMap::onTouchBegan(Touch* touch, Event* event)
    {
        // ...
        _lastTouchPos = touch->getLocation();
        _isDragging = false; // Initial state
        return false; // Return false so GameScene can handle it?
    }
    */
    
    // If it returns false, `onTouchMoved` is NEVER called.
    // So the original code's `onTouchMoved` implementation was dead code unless `onTouchBegan` logic was conditional.
    // But the code I saw just said `return false;`.
    // Maybe I missed something or it was commented out code?
    // "Return false means GameScene's touch event priority waits?" No.
    // If a listener returns false in onTouchBegan, it doesn't swallow the touch, and it doesn't receive subsequent events for that touch.
    
    // However, looking at the user request: "Single player battle map mode... Logic same as village".
    // Usually map should be draggable.
    // In `BaseMap`, I implemented dragging.
    // I will let `HomeVillageMap` behave like `BaseMap` (draggable).
    // If the original intention was to disable dragging in some cases, I'll ignore it for now as I'm refactoring for a "Battle Map" which definitely needs dragging.
    
    return BaseMap::onTouchBegan(touch, event);
}

void HomeVillageMap::onTouchMoved(Touch* touch, Event* event)
{
    BaseMap::onTouchMoved(touch, event);
}

void HomeVillageMap::onTouchEnded(Touch* touch, Event* event)
{
    BaseMap::onTouchEnded(touch, event);
}

void HomeVillageMap::startBuildingPlacement(BuildingType buildingType)
{
    CCLOG("HomeVillageMap: Starting building placement for type: %d", (int)buildingType);
}
