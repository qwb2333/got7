#pragma once
#include "lib/common/base.h"
#include "lib/common/utils.h"

namespace qwb {
    class ConfigReader {
    public:
        ConfigReader() {
            kv.clear();
        }
        std::map<std::string, std::string>& getKv() {
            return kv;
        };

        std::string getAsString(const std::string &k, std::string defaultValue = "");
        int32_t getAsInt(const std::string &k, int32_t defaultValue = 0);

    private:
        std::map<std::string, std::string> kv;
    };

    typedef std::unique_ptr<ConfigReader> ConfigReaderPtr;

    class ConfigReaderFactory {
    public:
        static ConfigReaderPtr createFromFile(const char *path);
    };
}