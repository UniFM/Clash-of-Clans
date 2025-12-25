#include "BuildingPreview.h"
#include "Map/HomeVillageMap.h"
#include "TownHall.h"
#include "ResourceBuilding.h"
#include "Barracks.h"
#include "DefenseTower.h"
#include "StorageBuilding.h"
#include "BuildingData.h"

BuildingPreview* BuildingPreview::create(HomeVillageMap* mapLayer)
{
    BuildingPreview* preview = new (std::nothrow) BuildingPreview();
    if (preview && preview->init(mapLayer))
    {
        preview->autorelease();
        return preview;
    }
    CC_SAFE_DELETE(preview);
    return nullptr;
}

BuildingPreview::BuildingPreview()
    : _mapLayer(nullptr)
    , _previewBuilding(nullptr)
    , _movingBuilding(nullptr)
    , _isPreviewing(false)
    , _isMoving(false)
    , _isSimpleDragging(false)
    , _isPreviewDragging(false)
    , _dragStartPos(Vec2::ZERO)
    , _dragThreshold(10.0f)
    , _originalGridX(0)
    , _originalGridY(0)
    , _gridHighlightNode(nullptr)
    , _confirmBtn(nullptr)
    , _cancelBtn(nullptr)
    , _uiListener(nullptr)
{
}

BuildingPreview::~BuildingPreview()
{
    // 安全清理监听器
    if (_uiListener) {
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->removeEventListener(_uiListener);
        _uiListener = nullptr;
    }
}

bool BuildingPreview::init(HomeVillageMap* mapLayer)
{
    if (!Node::init() || !mapLayer)
        return false;

    _mapLayer = mapLayer;

    // 初始化网格高亮节点
    _gridHighlightNode = Node::create();
    _mapLayer->addChild(_gridHighlightNode, 100);

    return true;
}

void BuildingPreview::startPreview(BuildingType buildingType)
{
    if (_isPreviewing || _isMoving)
    {
        cancel();
    }

    _previewType = buildingType;
    _isPreviewing = true;
    _isMoving = false;
    _isSimpleDragging = false;

    // 根据类型创建建筑
    Building* building = nullptr;
    switch (buildingType)
    {
    case BuildingType::TOWN_HALL:
        building = TownHall::create(1);
        break;
    case BuildingType::GOLD_MINE:
        building = ResourceBuilding::create(BuildingType::GOLD_MINE, 1);
        break;
    case BuildingType::ELIXIR_COLLECTOR:
        building = ResourceBuilding::create(BuildingType::ELIXIR_COLLECTOR, 1);
        break;
    case BuildingType::BARRACKS:
        building = Barracks::create(1);
        break;
    case BuildingType::ARMY_CAMP:
        building = Barracks::create(1);
        break;
    case BuildingType::ARCHER_TOWER:
        building = DefenseTower::create(BuildingType::ARCHER_TOWER, 1);
        break;
    case BuildingType::CANNON:
        building = DefenseTower::create(BuildingType::CANNON, 1);
        break;
    case BuildingType::GOLD_STORAGE:
        building = StorageBuilding::create(BuildingType::GOLD_STORAGE, 1);
        break;
    case BuildingType::ELIXIR_STORAGE:
        building = StorageBuilding::create(BuildingType::ELIXIR_STORAGE, 1);
        break;
    default:
        CCLOG("BuildingPreview::startPreview - Unsupported building type: %d", static_cast<int>(buildingType));
        building = nullptr;
        break;
    }

    if (building)
    {
        _previewBuilding = building;
        
        // 计算初始位置（地图中心）
        int centerGridX = _mapLayer->getGridCols() / 2;
        int centerGridY = _mapLayer->getGridRows() / 2;
        
        // 根据建筑大小调整位置
        Size gridSize = building->getGridSize();
        centerGridX -= (int)gridSize.width / 2;
        centerGridY -= (int)gridSize.height / 2;
        
        // 如果中心位置无法放置，寻找附近位置
        if (!_mapLayer->canPlaceBuilding(centerGridX, centerGridY, gridSize))
        {
            // 在周围寻找位置
            bool found = false;
            for (int radius = 1; radius <= 10 && !found; radius++)
            {
                for (int dx = -radius; dx <= radius && !found; dx++)
                {
                    for (int dy = -radius; dy <= radius && !found; dy++)
                    {
                        if (abs(dx) == radius || abs(dy) == radius) // 在边界上
                        {
                            int testX = centerGridX + dx;
                            int testY = centerGridY + dy;
                            if (_mapLayer->canPlaceBuilding(testX, testY, gridSize))
                            {
                                centerGridX = testX;
                                centerGridY = testY;
                                found = true;
                            }
                        }
                    }
                }
            }
        }
        
        // 设置建筑位置
        _previewBuilding->setGridPosition(centerGridX, centerGridY);
        Vec2 worldPos = _mapLayer->gridToWorld(centerGridX, centerGridY);
        _previewBuilding->setPosition(worldPos);
        
        // 设置建筑为半透明状态
        setBuildingOpacity(_previewBuilding, 180); // 70% 透明度
        _mapLayer->addBuilding(_previewBuilding);
        
        // 显示网格高亮
        updateGridHighlight(centerGridX, centerGridY, gridSize);
        
        CCLOG("Building preview started at center position (%d, %d)", centerGridX, centerGridY);
    }
    else
    {
        CCLOG("ERROR: Failed to create building for preview (type: %d)", static_cast<int>(buildingType));
        _isPreviewing = false;
        return;
    }

    // 创建确认/取消按钮
    createActionButtons();
}

void BuildingPreview::startMoving(Building* building)
{
    if (!building || _isPreviewing || _isMoving)
        return;

    if (_isPreviewing)
    {
        cancel();
    }

    _movingBuilding = building;
    _isMoving = true;
    _isPreviewing = false;

    // 记录原始位置
    Vec2 gridPos = building->getGridPosition();
    _originalGridX = (int)gridPos.x;
    _originalGridY = (int)gridPos.y;

    // 清除 occupy 标记
    Size gridSize = building->getGridSize();
    _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, false);

    // 设置建筑半透明
    setBuildingOpacity(building, 200);

    // 创建按钮
    createActionButtons();

    // 显示网格高亮
    updateGridHighlight(_originalGridX, _originalGridY, gridSize);
}

void BuildingPreview::createActionButtons()
{
    // 如果已经在简单拖拽模式，不需要按钮
    if (_isSimpleDragging)
    {
        return;
    }

    // 清理旧的按钮和监听器
    if (_confirmBtn || _cancelBtn)
    {
        if (_confirmBtn) _confirmBtn->removeFromParent();
        if (_cancelBtn) _cancelBtn->removeFromParent();
        
        if (_uiListener) {
            _eventDispatcher->removeEventListener(_uiListener);
            _uiListener = nullptr;
        }
        
        _confirmBtn = nullptr;
        _cancelBtn = nullptr;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto parentNode = _mapLayer->getParent(); // SceneMap

    // 创建确认按钮 Sprite
    _confirmBtn = Sprite::create("Icon/confirm_construction_button.png");
    if (!_confirmBtn) {
        // Fallback if image missing
        _confirmBtn = Sprite::create();
        auto label = Label::createWithSystemFont("YES", "Arial", 32);
        label->setColor(Color3B::GREEN);
        _confirmBtn->addChild(label);
        _confirmBtn->setContentSize(Size(80, 80));
    }
    _confirmBtn->setPosition(visibleSize.width - 100, 150);
    parentNode->addChild(_confirmBtn, 200);

    // 创建取消按钮 Sprite
    _cancelBtn = Sprite::create("Icon/cancel_construction_button.png");
    if (!_cancelBtn) {
        _cancelBtn = Sprite::create();
        auto label = Label::createWithSystemFont("NO", "Arial", 32);
        label->setColor(Color3B::RED);
        _cancelBtn->addChild(label);
        _cancelBtn->setContentSize(Size(80, 80));
    }
    _cancelBtn->setPosition(visibleSize.width - 100, 100);
    parentNode->addChild(_cancelBtn, 200);

    // 创建高优先级监听器 (FixedPriority -2, 低于 SceneMap 的 -1)
    _uiListener = EventListenerTouchOneByOne::create();
    _uiListener->setSwallowTouches(true); // 吞噬触摸，防止传给SceneMap
    
    _uiListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        Vec2 touchPos = touch->getLocation();
        
        if (_confirmBtn) {
             Vec2 localPos = _confirmBtn->getParent()->convertToNodeSpace(touchPos);
             if (_confirmBtn->getBoundingBox().containsPoint(localPos)) {
                 _confirmBtn->setColor(Color3B::GRAY); // 按下效果
                 return true;
             }
        }
        if (_cancelBtn) {
             Vec2 localPos = _cancelBtn->getParent()->convertToNodeSpace(touchPos);
             if (_cancelBtn->getBoundingBox().containsPoint(localPos)) {
                 _cancelBtn->setColor(Color3B::GRAY);
                 return true;
             }
        }
        return false;
    };
    
    _uiListener->onTouchEnded = [this](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();
        bool isConfirm = false;
        bool isCancel = false;

        if (_confirmBtn) {
             _confirmBtn->setColor(Color3B::WHITE); // 恢复颜色
             Vec2 localPos = _confirmBtn->getParent()->convertToNodeSpace(touchPos);
             if (_confirmBtn->getBoundingBox().containsPoint(localPos)) {
                 isConfirm = true;
             }
        }
        
        if (_cancelBtn) {
             _cancelBtn->setColor(Color3B::WHITE);
             Vec2 localPos = _cancelBtn->getParent()->convertToNodeSpace(touchPos);
             if (_cancelBtn->getBoundingBox().containsPoint(localPos)) {
                 isCancel = true;
             }
        }
        
        // 延迟一帧调用，避免在事件处理中删除监听器自身导致的问题（虽然removeEventListener通常是安全的）
        if (isConfirm) {
            this->retain(); // 防止this被销毁
            Director::getInstance()->getScheduler()->schedule([this](float){
                this->confirm();
                this->release();
            }, this, 0, 0, 0, false, "confirm_delay");
        }
        else if (isCancel) {
            this->retain();
            Director::getInstance()->getScheduler()->schedule([this](float){
                this->cancel();
                this->release();
            }, this, 0, 0, 0, false, "cancel_delay");
        }
    };
    
    _eventDispatcher->addEventListenerWithFixedPriority(_uiListener, -2);

    // 更新按钮位置跟随建筑
    if (_isPreviewing && _previewBuilding)
    {
        Vec2 worldPos = _previewBuilding->getPosition();
        updateButtonPosition(worldPos);
    }
    else if (_isMoving && _movingBuilding)
    {
        Vec2 worldPos = _movingBuilding->getPosition();
        updateButtonPosition(worldPos);
    }
}

void BuildingPreview::cancel()
{
    if (!_isPreviewing && !_isMoving && !_isSimpleDragging)
    {
        return;
    }
    
    CCLOG("BuildingPreview::cancel");

    if (_isPreviewing)
    {
        if (_previewBuilding)
        {
            _mapLayer->removeBuilding(_previewBuilding);
            _previewBuilding = nullptr;
        }
        _isPreviewing = false;
        _isPreviewDragging = false;
    }
    else if (_isMoving)
    {
        if (_movingBuilding)
        {
            // 恢复占位
            Size gridSize = _movingBuilding->getGridSize();
            _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, true);

            // 恢复位置
            _movingBuilding->setGridPosition(_originalGridX, _originalGridY);
            Vec2 worldPos = _mapLayer->gridToWorld(_originalGridX, _originalGridY);
            _movingBuilding->setPosition(worldPos);

            // 恢复透明度
            setBuildingOpacity(_movingBuilding, 255);
        }
        _movingBuilding = nullptr;
        _isMoving = false;
    }
    else if (_isSimpleDragging)
    {
        if (_movingBuilding)
        {
            Size gridSize = _movingBuilding->getGridSize();
            _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, true);

            _movingBuilding->setGridPosition(_originalGridX, _originalGridY);
            Vec2 worldPos = _mapLayer->gridToWorld(_originalGridX, _originalGridY);
            _movingBuilding->setPosition(worldPos);

            setBuildingOpacity(_movingBuilding, 255);
        }
        _movingBuilding = nullptr;
        _isSimpleDragging = false;
    }

    clearGridHighlight();

    // 清理按钮和监听器
    if (_confirmBtn) {
        _confirmBtn->removeFromParent();
        _confirmBtn = nullptr;
    }
    if (_cancelBtn) {
        _cancelBtn->removeFromParent();
        _cancelBtn = nullptr;
    }
    if (_uiListener) {
        _eventDispatcher->removeEventListener(_uiListener);
        _uiListener = nullptr;
    }
}

bool BuildingPreview::confirm()
{
    bool success = false;

    if (_isPreviewing)
    {
        if (!_previewBuilding) return false;

        Vec2 gridPos = _previewBuilding->getGridPosition();
        int gridX = (int)gridPos.x;
        int gridY = (int)gridPos.y;
        Size gridSize = _previewBuilding->getGridSize();

        if (!_mapLayer->canPlaceBuilding(gridX, gridY, gridSize))
        {
            CCLOG("Cannot confirm placement - invalid position");
            return false;
        }

        _mapLayer->markGridsOccupied(gridX, gridY, gridSize, true);
        setBuildingOpacity(_previewBuilding, 255);
        _isPreviewing = false;

        // 通知
        auto director = Director::getInstance();
        if (director && director->getRunningScene()) {
            Building* buildingToNotify = _previewBuilding;
            director->getScheduler()->schedule([=](float) {
                if (buildingToNotify) {
                    EventCustom event("building_placed");
                    event.setUserData(buildingToNotify);
                    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
                }
            }, this, 0.0f, 0, 0.0f, false, "building_placed_delayed");
        }

        _previewBuilding = nullptr;
        success = true;
    }
    else if (_isMoving)
    {
        if (!_movingBuilding) return false;

        Vec2 gridPos = _movingBuilding->getGridPosition();
        int gridX = (int)gridPos.x;
        int gridY = (int)gridPos.y;
        Size gridSize = _movingBuilding->getGridSize();

        if (!_mapLayer->canPlaceBuilding(gridX, gridY, gridSize))
        {
            // 还原
            _movingBuilding->setGridPosition(_originalGridX, _originalGridY);
            Vec2 worldPos = _mapLayer->gridToWorld(_originalGridX, _originalGridY);
            _movingBuilding->setPosition(worldPos);
            _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, true);
            return false;
        }

        _mapLayer->markGridsOccupied(gridX, gridY, gridSize, true);
        setBuildingOpacity(_movingBuilding, 255);
        _isMoving = false;

        auto director = Director::getInstance();
        if (director && director->getRunningScene()) {
            director->getScheduler()->schedule([=](float) {
                EventCustom event("building_moved");
                event.setUserData(_movingBuilding);
                Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
            }, this, 0.0f, 0, 0.0f, false, "building_moved_delayed");
        }

        _movingBuilding = nullptr;
        success = true;
    }
    else
    {
        return false;
    }

    clearGridHighlight();

    // 清理按钮和监听器
    if (_confirmBtn) {
        _confirmBtn->removeFromParent();
        _confirmBtn = nullptr;
    }
    if (_cancelBtn) {
        _cancelBtn->removeFromParent();
        _cancelBtn = nullptr;
    }
    if (_uiListener) {
        _eventDispatcher->removeEventListener(_uiListener);
        _uiListener = nullptr;
    }

    return success;
}

void BuildingPreview::updatePreviewPosition(const Vec2& screenPos)
{
    Building* targetBuilding = nullptr;
    bool shouldUpdate = false;
    
    if (_isPreviewing)
    {
        targetBuilding = _previewBuilding;
        shouldUpdate = _isPreviewDragging;
    }
    else if (_isMoving || _isSimpleDragging)
    {
        targetBuilding = _movingBuilding;
        shouldUpdate = true;
    }

    if (!targetBuilding || !_mapLayer || !shouldUpdate)
    {
        return;
    }

    Vec2 mapPos = _mapLayer->getMapPosition();
    float zoom = _mapLayer->getZoom();
    Vec2 worldPos = (screenPos - mapPos) / zoom;

    Vec2 gridPos = _mapLayer->worldToGrid(worldPos);
    int gridX = (int)gridPos.x;
    int gridY = (int)gridPos.y;

    if (gridX < 0 || gridY < 0)
    {
        if (_isPreviewing)
        {
            Vec2 currentGridPos = targetBuilding->getGridPosition();
            Size gridSize = targetBuilding->getGridSize();
            updateGridHighlight((int)currentGridPos.x, (int)currentGridPos.y, gridSize);
            return;
        }
        else
        {
            clearGridHighlight();
            return;
        }
    }

    targetBuilding->setGridPosition(gridX, gridY);
    Vec2 worldBuildingPos = _mapLayer->gridToWorld(gridX, gridY);
    targetBuilding->setPosition(worldBuildingPos);

    if (!_isSimpleDragging && (_isMoving || _isPreviewDragging))
    {
        updateButtonPosition(worldBuildingPos);
    }

    Size gridSize = targetBuilding->getGridSize();
    updateGridHighlight(gridX, gridY, gridSize);
}

void BuildingPreview::updateGridHighlight(int gridX, int gridY, Size gridSize)
{
    clearGridHighlight();

    std::vector<Vec2> occupiedGrids = _mapLayer->getOccupiedGrids(gridX, gridY, gridSize);
    bool overallCanPlace = _mapLayer->canPlaceBuilding(gridX, gridY, gridSize);

    for (const auto& grid : occupiedGrids)
    {
        int x = (int)grid.x;
        int y = (int)grid.y;

        bool gridCanPlace = _mapLayer->isValidGrid(x, y);
        if (gridCanPlace)
        {
            gridCanPlace = overallCanPlace;
        }

        Node* tile = createHighlightTile(gridCanPlace);
        if (tile)
        {
            Vec2 worldPos = _mapLayer->gridToWorld(x, y);
            tile->setPosition(worldPos);
            _gridHighlightNode->addChild(tile);
            _highlightTiles.push_back(tile);
        }
    }
    
    if (_isSimpleDragging)
    {
        for (auto tile : _highlightTiles)
        {
            if (tile)
            {
                auto scaleUp = ScaleTo::create(0.3f, 1.1f);
                auto scaleDown = ScaleTo::create(0.3f, 1.0f);
                auto sequence = Sequence::create(scaleUp, scaleDown, nullptr);
                auto repeat = RepeatForever::create(sequence);
                tile->runAction(repeat);
            }
        }
    }
}

void BuildingPreview::clearGridHighlight()
{
    for (auto tile : _highlightTiles)
    {
        if (tile)
        {
            tile->removeFromParent();
        }
    }
    _highlightTiles.clear();
}

Node* BuildingPreview::createHighlightTile(bool canPlace)
{
    auto drawNode = DrawNode::create();

    // 使用 _mapLayer 的 getter 获取草地宽高和网格数
    float cellWidth = _mapLayer->getGrassRectWidth() / _mapLayer->getGridCols();
    float cellHeight = _mapLayer->getGrassRectHeight() / _mapLayer->getGridRows();

    Color4F tileColor = canPlace ?
        Color4F(0.0f, 1.0f, 0.0f, 0.6f) :
        Color4F(1.0f, 0.0f, 0.0f, 0.6f);

    Vec2 vertices[4] = {
        Vec2(-cellWidth / 2, -cellHeight / 2),
        Vec2(cellWidth / 2, -cellHeight / 2),
        Vec2(cellWidth / 2, cellHeight / 2),
        Vec2(-cellWidth / 2, cellHeight / 2)
    };

    drawNode->drawPoly(vertices, 4, true, tileColor);

    Color4F borderColor = canPlace ?
        Color4F(0.0f, 0.8f, 0.0f, 1.0f) :
        Color4F(0.8f, 0.0f, 0.0f, 1.0f);
    drawNode->drawPoly(vertices, 4, false, borderColor);

    return drawNode;
}

void BuildingPreview::updateButtonPosition(const Vec2& worldBuildingPos)
{
    if ((!_confirmBtn && !_cancelBtn) || !_mapLayer)
        return;

    Vec2 mapPos = _mapLayer->getMapPosition();
    float zoom = _mapLayer->getZoom();
    Vec2 screenPos = worldBuildingPos * zoom + mapPos;

    float buttonOffsetY = 80.0f;

    if (_confirmBtn)
    {
        _confirmBtn->setPosition(screenPos.x, screenPos.y + buttonOffsetY);
    }
    if (_cancelBtn)
    {
        _cancelBtn->setPosition(screenPos.x + 60, screenPos.y + buttonOffsetY);
    }
}

void BuildingPreview::setBuildingOpacity(Building* building, GLubyte opacity)
{
    if (building)
    {
        building->setOpacity(opacity);
        auto children = building->getChildren();
        for (auto child : children)
        {
            child->setOpacity(opacity);
        }
    }
}

void BuildingPreview::startSimpleDrag(Building* building)
{
    if (!building)
        return;

    if (_isPreviewing || _isMoving || _isSimpleDragging)
    {
        cancel();
    }

    _movingBuilding = building;
    _isSimpleDragging = true;
    _isPreviewing = false;
    _isMoving = false;

    Vec2 gridPos = building->getGridPosition();
    _originalGridX = (int)gridPos.x;
    _originalGridY = (int)gridPos.y;

    Size gridSize = building->getGridSize();
    _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, false);

    setBuildingOpacity(building, 150);

    updateGridHighlight(_originalGridX, _originalGridY, gridSize);
}

void BuildingPreview::endSimpleDrag()
{
    if (!_isSimpleDragging || !_movingBuilding)
        return;

    Vec2 currentGridPos = _movingBuilding->getGridPosition();
    int currentGridX = (int)currentGridPos.x;
    int currentGridY = (int)currentGridPos.y;
    Size gridSize = _movingBuilding->getGridSize();

    bool canPlaceAtCurrent = _mapLayer->canPlaceBuilding(currentGridX, currentGridY, gridSize);
    
    if (canPlaceAtCurrent)
    {
        _mapLayer->markGridsOccupied(currentGridX, currentGridY, gridSize, true);
        
        auto director = Director::getInstance();
        if (director && director->getRunningScene()) {
            Building* buildingToNotify = _movingBuilding;
            director->getScheduler()->schedule([=](float) {
                if (buildingToNotify) {
                    EventCustom event("building_moved");
                    event.setUserData(buildingToNotify);
                    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
                }
            }, this, 0.0f, 0, 0.0f, false, "building_simple_moved_delayed");
        }
    }
    else
    {
        _movingBuilding->setGridPosition(_originalGridX, _originalGridY);
        Vec2 worldPos = _mapLayer->gridToWorld(_originalGridX, _originalGridY);
        _movingBuilding->setPosition(worldPos);
        _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, true);
        
        auto moveAction = MoveTo::create(0.2f, worldPos);
        auto easeAction = EaseBackOut::create(moveAction);
        _movingBuilding->runAction(easeAction);
    }

    setBuildingOpacity(_movingBuilding, 255);

    _movingBuilding = nullptr;
    _isSimpleDragging = false;

    clearGridHighlight();
}

bool BuildingPreview::validateAndConfirmPosition()
{
    if (!_movingBuilding)
        return false;

    Vec2 gridPos = _movingBuilding->getGridPosition();
    int gridX = (int)gridPos.x;
    int gridY = (int)gridPos.y;
    Size gridSize = _movingBuilding->getGridSize();

    if (_mapLayer->canPlaceBuilding(gridX, gridY, gridSize))
    {
        _mapLayer->markGridsOccupied(gridX, gridY, gridSize, true);
        return true;
    }
    else
    {
        _movingBuilding->setGridPosition(_originalGridX, _originalGridY);
        Vec2 worldPos = _mapLayer->gridToWorld(_originalGridX, _originalGridY);
        _movingBuilding->setPosition(worldPos);
        _mapLayer->markGridsOccupied(_originalGridX, _originalGridY, gridSize, true);
        return false;
    }
}

void BuildingPreview::startPreviewDragDetection(const Vec2& startPos)
{
    if (!_isPreviewing)
        return;
    
    _dragStartPos = startPos;
}

bool BuildingPreview::checkDragStart(const Vec2& currentPos)
{
    if (!_isPreviewing || _isPreviewDragging)
        return false;
    
    float distance = _dragStartPos.distance(currentPos);
    
    if (distance >= _dragThreshold)
    {
        _isPreviewDragging = true;
        return true;
    }
    
    return false;
}

void BuildingPreview::endPreviewDrag()
{
    if (!_isPreviewDragging)
        return;
    
    _isPreviewDragging = false;
    
    if (_previewBuilding)
    {
        Vec2 worldPos = _previewBuilding->getPosition();
        updateButtonPosition(worldPos);
    }
}

Building* BuildingPreview::checkBuildingAtPosition(const Vec2& worldPos, const Vector<Building*>& buildings)
{
    for (auto building : buildings)
    {
        if (building && !building->isDestroyed())
        {
            Vec2 buildingPos = building->getPosition();
            Size buildingSize = building->getContentSize();
            
            float padding = 30.0f;
            Rect buildingRect(
                buildingPos.x - buildingSize.width / 2 - padding,
                buildingPos.y - buildingSize.height / 2 - padding,
                buildingSize.width + padding * 2,
                buildingSize.height + padding * 2
            );

            if (buildingRect.containsPoint(worldPos))
            {
                return building;
            }
        }
    }
    return nullptr;
}
