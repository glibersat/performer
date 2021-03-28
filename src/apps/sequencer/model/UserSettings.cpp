#include <iostream> // TODO remove
#include "UserSettings.h"

void UserSettings::set(int key, int value) {
    std::cout << "Setting " << key << ": " << value << std::endl;
    _settings[key]->setValue(value);
}

//void UserSettings::set(int key, int value) {
//    int i = 0;
//    for (auto it = _settings.begin(); it != _settings.end(); it++)
//        _setting.second.reset();
//    }
//}

void UserSettings::shift(int key, int shift) {
    _settings[key]->shiftValue(shift);
}

std::shared_ptr<BaseSetting> UserSettings::get(const char *key) {
//    return _settings[key].getValue();
    for (auto &setting : _settings) {
        if (setting->getKey() == key) {
            return setting;
        }
    }
    return nullptr;
}

std::shared_ptr<BaseSetting> UserSettings::get(int key) {
    return _settings[key];
}

std::vector<std::shared_ptr<BaseSetting>> UserSettings::all() {
    return _settings;
}

void UserSettings::clear() {
    std::cout<<"clearing brightness"<<std::endl;
    for (auto &_setting : _settings) {
        _setting.reset();
    }
}

void UserSettings::write(VersionedSerializedWriter &writer) const {
    for (auto &setting : _settings) {
        setting->write(writer);
    }
}

void UserSettings::read(VersionedSerializedReader &reader) {
    for (auto &setting : _settings) {
        setting->read(reader);
    }
}