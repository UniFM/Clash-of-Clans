#ifndef TROOP_SELECTION_SCENE_H
#define TROOP_SELECTION_SCENE_H

#include "cocos2d.h"
#include "TroopDefinitions.h"

class TroopSelectionScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // UI Callbacks
    void onBackCallback(cocos2d::Ref* pSender);
    void onTroopSelected(TroopType type);   // Add (Plus)
    void onTroopDeselected(TroopType type); // Remove (Minus)
    void onResetCallback(cocos2d::Ref* pSender);

    // Update labels
    void updateLabels();

    CREATE_FUNC(TroopSelectionScene);

private:
    cocos2d::Label* capacityLabel;
    std::map<TroopType, cocos2d::Label*> countLabels;
};

#endif // TROOP_SELECTION_SCENE_H
