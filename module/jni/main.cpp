#include <jni.h>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <android/log.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#include "zygisk.hpp"

class DeviceSpoofZygisk : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        
        std::ifstream configFile("/data/adb/modules/device_spoof/config.yaml");
        if (!configFile.is_open()) {
            __android_log_print(ANDROID_LOG_ERROR, "DeviceSpoofZygisk", "Failed to open config.yaml");
            return;
        }
        YAML::Node yaml = YAML::Load(configFile);
        for (const auto& pkg : yaml["packages"]) {
            std::string packageName = pkg["package_name"].as<std::string>();
            auto& fields = config[packageName];
            for (const auto& field : pkg["fields"]) {
                std::string fieldName = field.first.as<std::string>();
                std::string value = field.second.as<std::string>();
                fields[fieldName] = value;
            }
        }

    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        jstring jPackageName = args->nice_name;
        const char* packageName = env->GetStringUTFChars(jPackageName, nullptr);
        if (packageName) {
            auto it = config.find(packageName);
            if (it != config.end()) {
                shouldSpoof = true;
                currentFields = it->second;
            } else {
                shouldSpoof = false;
            }
            env->ReleaseStringUTFChars(jPackageName, packageName);
        }
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (!shouldSpoof) return;
        jclass buildClass = env->FindClass("android/os/Build");
        if (!buildClass) {
            __android_log_print(ANDROID_LOG_ERROR, "DeviceSpoofZygisk", "Failed to find Build class");
            return;
        }
        for (const auto& pair : currentFields) {
            const std::string& fieldName = pair.first;
            const std::string& value = pair.second;
            jfieldID fieldId = env->GetStaticFieldID(buildClass, fieldName.c_str(), "Ljava/lang/String;");
            if (fieldId) {
                jstring jValue = env->NewStringUTF(value.c_str());
                env->SetStaticObjectField(buildClass, fieldId, jValue);
                env->DeleteLocalRef(jValue);
                __android_log_print(ANDROID_LOG_INFO, "DeviceSpoofZygisk", "Set Build.%s to %s", fieldName.c_str(), value.c_str());
            } else {
                __android_log_print(ANDROID_LOG_WARN, "DeviceSpoofZygisk", "Field %s not found in Build class", fieldName.c_str());
            }
        }
        env->DeleteLocalRef(buildClass);
    }

private:
    zygisk::Api *api;
    JNIEnv *env;
    std::map<std::string, std::map<std::string, std::string>> config;
    bool shouldSpoof = false;
    std::map<std::string, std::string> currentFields;
};

REGISTER_ZYGISK_MODULE(DeviceSpoofZygisk)