#include "config.h"
using namespace qwb;

ConfigReaderPtr ConfigReaderFactory::createFromFile(const char *path) {
    FILE *file = fopen(path, "r");
    char buffer[4096];

    ConfigReaderPtr ptr(new ConfigReader);
    std::map<std::string, std::string> &kv = ptr->getKv();

    while(fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        for(size_t i = 0; i < len; i++) {
            if(buffer[i] == '=') {
                std::string k, v;
                k = Utils::trim(buffer, 0, i - 1);
                v = Utils::trim(buffer, i + 1, len - 1);
                kv[k] = v;
                break;
            }
        }
    }
    return ptr;
}

std::string ConfigReader::getAsString(const std::string &k, std::string defaultValue) {
    auto iter = kv.find(k);
    if(iter == kv.end()) return defaultValue;
    return iter->second;
}

int32_t ConfigReader::getAsInt(const std::string &k, int32_t defaultValue) {
    auto iter = kv.find(k);
    if(iter == kv.end()) return defaultValue;
    return std::atoi(iter->second.c_str());
}