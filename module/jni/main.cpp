#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <string>
#include "yaml_parser.h"

#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "DeviceSpoofZygisk", __VA_ARGS__)

class DeviceSpoofZygisk : public zygisk::ModuleBase
{
public:
    void onLoad(Api *api, JNIEnv *env) override
    {
        this->api = api;
        this->env = env;
        // 加载模板文件
        if (!yaml.load_templates(module_path + std::string("template.yml")))
        {
            LOGD("Failed to load template.yml");
            return;
        }

        // 加载目标文件
        if (!yaml.load_targets(module_path + std::string("target.yml")))
        {
            LOGD("Failed to load target.yml");
            return;
        }
        LOGD("DeviceSpoof loading successsful...");
    }

    void preAppSpecialize(AppSpecializeArgs *args) override
    {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        preSpecialize(process);
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override
    {
        preSpecialize("system_server");
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override
    {
        if (shouldSpoof)
        {
            jclass buildClass = env->FindClass("android/os/Build");

            auto setField = [&](const char *fieldName, const std::string &value)
            {
                jfieldID fieldId = env->GetStaticFieldID(buildClass, fieldName, "Ljava/lang/String;");
                if (fieldId)
                {
                    jstring jValue = env->NewStringUTF(value.c_str());
                    env->SetStaticObjectField(buildClass, fieldId, jValue);
                    env->DeleteLocalRef(jValue);
                }
            };
            for (const auto &[key, val] : entry.fields)
            {
                setField(key.data(), val);
            }

            env->DeleteLocalRef(buildClass);
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool shouldSpoof = false;
    YamlParser yaml;
    Entry entry;
    std::string module_path = "/data/adb/modules/device_spoof/";

    void preSpecialize(const char *process)
    {
        if (yaml.contains_package(process))
        {
            shouldSpoof = true;
            entry = yaml.find_by_package(process);
        }
        LOGD("process=[%s], shouldSpoof=%d", process, shouldSpoof);
    }
};

REGISTER_ZYGISK_MODULE(DeviceSpoofZygisk)