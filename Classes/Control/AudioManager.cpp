#include "AudioManager.h"

AudioManager* AudioManager::_instance = nullptr;

AudioManager* AudioManager::getInstance()
{
    if (!_instance)
    {
        _instance = new (std::nothrow) AudioManager();
    }
    return _instance;
}

void AudioManager::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}

AudioManager::AudioManager()
    : _backgroundMusicId(-1)
    , _backgroundMusicVolume(1.0f)
    , _soundEffectVolume(1.0f)
{
}

AudioManager::~AudioManager()
{
    stopBackgroundMusic();
}

void AudioManager::playBackgroundMusic(const std::string& filePath, bool loop)
{
    stopBackgroundMusic();
    _backgroundMusicId = AudioEngine::play2d(filePath, loop);
    AudioEngine::setVolume(_backgroundMusicId, _backgroundMusicVolume);
}

void AudioManager::stopBackgroundMusic()
{
    if (_backgroundMusicId != -1)
    {
        AudioEngine::stop(_backgroundMusicId);
        _backgroundMusicId = -1;
    }
}

void AudioManager::pauseBackgroundMusic()
{
    if (_backgroundMusicId != -1)
    {
        AudioEngine::pause(_backgroundMusicId);
    }
}

void AudioManager::resumeBackgroundMusic()
{
    if (_backgroundMusicId != -1)
    {
        AudioEngine::resume(_backgroundMusicId);
    }
}

int AudioManager::playSoundEffect(const std::string& filePath, bool loop)
{
    int soundId = AudioEngine::play2d(filePath, loop);
    AudioEngine::setVolume(soundId, _soundEffectVolume);
    return soundId;
}

void AudioManager::stopSoundEffect(int soundId)
{
    AudioEngine::stop(soundId);
}

void AudioManager::setBackgroundMusicVolume(float volume)
{
    _backgroundMusicVolume = clampf(volume, 0.0f, 1.0f);
    if (_backgroundMusicId != -1)
    {
        AudioEngine::setVolume(_backgroundMusicId, _backgroundMusicVolume);
    }
}

void AudioManager::setSoundEffectVolume(float volume)
{
    _soundEffectVolume = clampf(volume, 0.0f, 1.0f);
}

void AudioManager::preloadBackgroundMusic(const std::string& filePath)
{
    AudioEngine::preload(filePath);
}

void AudioManager::preloadSoundEffect(const std::string& filePath)
{
    AudioEngine::preload(filePath);
}

