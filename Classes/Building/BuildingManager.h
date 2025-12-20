// 在您的游戏场景中使用：

// 1. 创建建筑管理器
auto buildingManager = BuildingManager::create();
this->addChild(buildingManager);

// 2. 开始放置建筑（比如在UI按钮点击时）
buildingManager->startPlacingBuilding(BuildingType::ARCHER_TOWER, homeVillageMap);

// 3. 剩下的交互会自动处理：
//    - 鼠标移动时会显示绿色/红色预览
//    - 点击确认放置或取消