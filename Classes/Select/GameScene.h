#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "cocos2d.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // Callback for the "Select Troops" button
    void onSelectTroopsCallback(cocos2d::Ref* pSender);

    // Update the visual representation of troops
    void updateTroopDisplay();

    // Implement the "static create()" method manually
    CREATE_FUNC(GameScene);

    virtual void onEnterTransitionDidFinish() override;

private:
    cocos2d::Node* barracksNode;
    cocos2d::Node* troopsContainer;
};

#endif // GAME_SCENE_H
