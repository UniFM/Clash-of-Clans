#pragma once
#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;
//using namespace cocos2d::experimental;

/**
 * 音效管理器
 */
class AudioManager
{
public:
    static AudioManager* getInstance();
    static void destroyInstance();

    // 背景音乐
    void playBackgroundMusic(const std::string& filePath, bool loop = true);
    void stopBackgroundMusic();
    void pauseBackgroundMusic();
    void resumeBackgroundMusic();

    // 音效
    int playSoundEffect(const std::string& filePath, bool loop = false);
    void stopSoundEffect(int soundId);

    // 音量控制
    void setBackgroundMusicVolume(float volume);
    void setSoundEffectVolume(float volume);

    // 预加载
    void preloadBackgroundMusic(const std::string& filePath);
    void preloadSoundEffect(const std::string& filePath);

private:
    AudioManager();
    ~AudioManager();

    int _backgroundMusicId;
    float _backgroundMusicVolume;
    float _soundEffectVolume;

    static AudioManager* _instance;
};

#endif // __AUDIO_MANAGER_H__

