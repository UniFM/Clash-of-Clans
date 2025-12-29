#include "Troop.h"

USING_NS_CC;

Troop::Troop() : _isPatrolling(false) {}
Troop::~Troop() {}

Troop* Troop::create(Troops type) {
    Troop* ret = new (std::nothrow) Troop();
    if (ret && ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Troop::init(Troops type) {
    if (!Sprite::init()) return false;
    
    _type = type;

    // Default appearance - start with a solid color block
    setTextureRect(Rect(0, 0, 32, 32));
    setColor(Color3B::WHITE); 
    
    createWalkAnimation();
    
    return true;
}

void Troop::createWalkAnimation() {
    Vector<SpriteFrame*> frames;
    
    // Helper to create a solid color texture
    auto createTexture = [](Color4B color) -> Texture2D* {
        auto img = new Image();
        int w = 32, h = 32;
        int dataLen = w * h * 4;
        unsigned char* data = new unsigned char[dataLen];
        for(int i=0; i<w*h; i++) {
            data[i*4] = color.r;
            data[i*4+1] = color.g;
            data[i*4+2] = color.b;
            data[i*4+3] = color.a;
        }
        
        img->initWithRawData(data, dataLen, w, h, 8);
        delete[] data; 
        
        auto tex = new Texture2D();
        tex->initWithImage(img);
        img->release();
        
        return tex; 
    };
    
    // Create 3 frames: Red, Green, Blue for Barbarian
    if (_type == Troops::Barbarian) {
        // Frame 1: Red
        auto tex1 = createTexture(Color4B::RED);
        tex1->autorelease();
        frames.pushBack(SpriteFrame::createWithTexture(tex1, Rect(0,0,32,32)));
        
        // Frame 2: Green
        auto tex2 = createTexture(Color4B::GREEN);
        tex2->autorelease();
        frames.pushBack(SpriteFrame::createWithTexture(tex2, Rect(0,0,32,32)));
        
        // Frame 3: Blue
        auto tex3 = createTexture(Color4B::BLUE);
        tex3->autorelease();
        frames.pushBack(SpriteFrame::createWithTexture(tex3, Rect(0,0,32,32)));
    }
    
    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        std::string animName = "walk_" + std::to_string((int)_type);
        AnimationCache::getInstance()->addAnimation(animation, animName);
    }
}

void Troop::playWalkAnimation() {
    if (getActionByTag(101)) return;
    
    std::string animName = "walk_" + std::to_string((int)_type);
    auto animation = AnimationCache::getInstance()->getAnimation(animName);
    if (animation) {
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(101);
        runAction(repeat);
    }
}

void Troop::stopWalkAnimation() {
    stopActionByTag(101);
}

void Troop::startPatrol(std::function<Vec2(const Size&)> getNextPosition) {
    _getNextPosition = getNextPosition;
    _isPatrolling = true;
    moveToNextPosition();
}

void Troop::stopPatrol() {
    _isPatrolling = false;
    stopAllActions();
    stopWalkAnimation();
}

void Troop::moveToNextPosition() {
    if (!_isPatrolling || !_getNextPosition) return;
    
    Vec2 target = _getNextPosition(getContentSize());
    
    // Calculate speed
    float distance = getPosition().distance(target);
    float speed = 60.0f; // Pixels per second
    float duration = distance / speed;
    if (duration < 0.1f) duration = 0.1f;
    
    playWalkAnimation();
    
    auto move = MoveTo::create(duration, target);
    auto stopAnim = CallFunc::create([this](){ this->stopWalkAnimation(); });
    auto delay = DelayTime::create(1.0f + CCRANDOM_0_1() * 2.0f);
    auto next = CallFunc::create([this](){ this->moveToNextPosition(); });
    
    auto seq = Sequence::create(move, stopAnim, delay, next, nullptr);
    runAction(seq);
}
