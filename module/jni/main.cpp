#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <string>

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
        LOGD("DeviceSpoof loading success...");
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

            setField("MODEL", "Pixel 9 Pro");
            setField("MANUFACTURER", "Google");
            setField("BRAND", "google");
            setField("DEVICE", "caiman");
            setField("TAGS", "release-keys");
            setField("TYPE", "user");
            setField("PRODUCT", "caiman");
            setField("BOARD", "caiman");
            setField("ID", "AD1A.240530.047.U1");
            setField("FINGERPRINT", "google/caiman/caiman:14/AD1A.240530.047.U1/12150698:user/release-keys");

            env->DeleteLocalRef(buildClass);
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool shouldSpoof = false;

    int str_in_array(const char *str, const char **arr, int size)
    {
        for (int i = 0; i < size; i++)
        {
            if (strcmp(str, arr[i]) == 0)
            {
                return 1;
            }
        }
        return 0;
    }

    void preSpecialize(const char *process)
    {
        const char *should_spoof_process[] = {"flar2.devcheck", "com.zhenxi.hunter", "me.garfieldhan.holmes"};
        int arr_size = sizeof(should_spoof_process) / sizeof(should_spoof_process[0]);

        if (str_in_array(process, should_spoof_process, arr_size))
        {
            shouldSpoof = true;
        }
        else
        {
            shouldSpoof = false;
        }
        LOGD("process=[%s], shouldSpoof=%d", process, shouldSpoof);
    }
};

// Register our module class and the companion handler function
REGISTER_ZYGISK_MODULE(DeviceSpoofZygisk)