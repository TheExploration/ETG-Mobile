#include <jni.h>                 // for JNI_ERR, JNIEnv, jclass, JNINativeM...
#include <pthread.h>             // for pthread_create
#include <unistd.h>              // for sleep
#include "Il2cpp/Il2cpp.h"       // for EnsureAttached, Init
#include "Il2cpp/il2cpp-class.h" // for Il2CppImage, Il2CppObject
#include "Includes/Logger.h"     // for LOGD, LOGI
#include "Includes/Utils.h"      // for isGameLibLoaded, isLibraryLoaded
#include "Includes/obfuscate.h"  // for make_obfuscator, OBFUSCATE
#include "Menu/Menu.h"           // for Icon, IconWebViewData, SettingsList
#include "Menu/Setup.h"          // for CheckOverlayPermission, Init
#include "Includes/Macros.h"

// Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

Il2CppImage *g_Image = nullptr;

// exmaple dump.cs
// class Game.Sample.Class //Assembly-CSharp
// {
//     System.Int32 Id; // 0x10
//     System.Int32 BuffType; // 0x14
//     UnityEngine.Vector3 Vel; // 0x18
//     UnityEngine.Vector3 Pos // 0x24
//     System.Single Interval; // 0x30
//     System.Single Counter; // 0x34
//     System.Int32 GoIndex; // 0x38
//     System.String Param; // 0x40
//     System.Int32 get_Id();
//     System.Void set_Position(UnityEngine.Vector3 pos);
//     System.Void .ctor(); // 0x017ad8d4
// }
//
// class Game.Sample.Class.SubClass
// {
//     System.Void SampleMethod();
//     System.Void .ctor();
// }

bool enterMainScene = false;
bool forceUnpause = false;
bool enableSheep = false;
bool charSelect = false;
bool returnToFoyer = false;
bool showLoginScreen = false;
int camScale = 1;
bool setCamScale = false;




void GameManager_Update(Il2CppObject *instance) {
    LOGD("update Called: %f, %f, %f");

    if(instance!=nullptr) {
        if(enterMainScene) {
            instance->invoke_method<void>("EnterMainScene", 1);
            enterMainScene = false;  // because we only want to call the method once
        }
        if (forceUnpause) {
            instance->invoke_method<void>("ForceUnpause");
            forceUnpause = false;
        }
        if (enableSheep) {
            auto gmClass = g_Image->getClass("GameManager");
            gmClass->invoke_static_method<void>("SetBaaBaaActive", 1);
            enableSheep = false;
        }
        if (charSelect) {
            instance->invoke_method<void>("LoadCharacterSelect", 1, 0, 0);
            charSelect = false;
        }
        if (returnToFoyer) {
            instance->invoke_method<void>("ReturnToFoyer");
            returnToFoyer = false;
        }
        if (setCamScale) {
            auto gmClass = g_Image->getClass("GameManager");
            LOGD("Camera Scale is", camScale);
            if (camScale == 1) {
                gmClass->invoke_static_method<void>("SetCameraScale", 1);

            } else if (camScale == 2) {
                gmClass->invoke_static_method<void>("SetCameraScale", 2);

            } else {
                gmClass->invoke_static_method<void>("SetCameraScale", 3);

            }
            setCamScale = false;
        }
    }
    return instance->invoke_method<void>("Update");
}


void OnLoginClick(Il2CppObject *instance) {
    LOGD("OnLoginClick");
    enterMainScene = true;
}

void DoShowBestiary(Il2CppObject *instance, Il2CppObject *control, Il2CppObject *mouseEvent) {
    LOGD("DoShowBestiary");
}

void DoGameOver(Il2CppObject *instance, Il2CppObject *gameOverSource) {
    LOGD("DoGameOver");
    returnToFoyer = true;
}




struct UnityEngine_Vector3
{
    float x, y, z;
};
/*
void Class_set_Position(Il2CppObject *instance, UnityEngine_Vector3 pos)
{
    LOGD("set_Position: %f, %f, %f", pos.x, pos.y, pos.z);
    pos.x += 1;
    return instance->invoke_method<void>("set_Position", pos);
}

void (*o_Class_ctor)(Il2CppObject *);
void Class_ctor(Il2CppObject *instance)
{
    o_Class_ctor(instance);
    auto id = instance->getField<int32_t>("Id");
    LOGINT(id);
    instance->setField("BuffType", 2);
}

void SubClass_SampleMethod(Il2CppObject *instance)
{
    LOGD("SampleMethod");
    return instance->invoke_method<void>("SampleMethod");
}
*/
// we will run our hacks in a new thread so our while loop doesn't block process main thread
void *hack_thread(void *)
{
    LOGI(OBFUSCATE("pthread created"));

    // Check if target lib is loaded
    do
    {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded"), (const char *)targetLibName);

    Il2cpp::Init();
    Il2cpp::EnsureAttached();

    LOGD(OBFUSCATE("HOOKING..."));
    g_Image = Il2cpp::GetAssembly("Assembly-CSharp")->getImage();

    // // HOOKS
    REPLACE_NAME("GameManager", "Update", GameManager_Update);
    REPLACE_NAME("GameMain.ProcedureLogin", "OnLoginClick", OnLoginClick);
    REPLACE_NAME("PauseMenuController", "DoShowBestiary", DoShowBestiary);
    REPLACE_NAME("GameManager", "DoGameOver", DoGameOver);

    //REPLACE_NAME_ORIG("Game.Sample.Class", ".ctor", Class_ctor, o_Class_ctor);

    // // HOOK SUBCLASS
    // auto SubClass = g_Image->getClass("Game.Sample.Class.SubClass", 1);
    // REPLACE_NAME_KLASS(SubClass, "SampleMethod", SubClass_SampleMethod);

    LOGD(OBFUSCATE("HOOKED!"));

    return nullptr;
}


jobjectArray GetFeatureList(JNIEnv *env, [[maybe_unused]] jobject context)
{
    jobjectArray ret;

    const char *features[] = {
        OBFUSCATE("Button_Enter The Breach"),
        OBFUSCATE("Button_Fix Black Screen"),
        OBFUSCATE("Button_Enable Cult of the Lamb Event"),
        OBFUSCATE("Button_Load Character Select"),
        OBFUSCATE("SeekBar_Camera Scale_1_3")};

    // Now you dont have to manually update the number everytime;
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                            env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

void Changes(JNIEnv *env, [[maybe_unused]] jclass clazz, [[maybe_unused]] jobject obj, jint featNum, jstring featName,
             jint value, jboolean boolean, jstring str)
{

    LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d | Text: = %s"), featNum,
         env->GetStringUTFChars(featName, nullptr), value, boolean,
         str != nullptr ? env->GetStringUTFChars(str, nullptr) : "");

    // BE CAREFUL NOT TO ACCIDENTALLY REMOVE break;

    switch (featNum)
    {
        case 0:
        {
            enterMainScene = true;
            break;
        }
        case 1:
        {
            forceUnpause = true;
            break;
        }
        case 2:
        {
            enableSheep = true;
            break;
        }
        case 3:
        {
            charSelect = true;
            break;
        }
        case 4:
        {
            camScale = value;
            setCamScale = true;
            break;
        }
    }
}

__attribute__((constructor)) void lib_main()
{
    // Create a new thread so it does not block the main thread, means the game would not freeze
    pthread_t ptid;
    pthread_create(&ptid, nullptr, hack_thread, nullptr);
}

int RegisterMenu(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(Icon)},
        {OBFUSCATE("IconWebViewData"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(IconWebViewData)},
        {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE("()Z"), reinterpret_cast<void *>(isGameLibLoaded)},
        {OBFUSCATE("Init"), OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"),
         reinterpret_cast<void *>(Init)},
        {OBFUSCATE("SettingsList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(SettingsList)},
        {OBFUSCATE("GetFeatureList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(GetFeatureList)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"),
         reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterMain(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"),
         reinterpret_cast<void *>(CheckOverlayPermission)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;

    return JNI_OK;
}

extern "C" JNIEXPORT jint

    JNICALL
    JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved)
{
    JNIEnv *env;
    vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (RegisterMenu(env) != 0)
        return JNI_ERR;
    if (RegisterPreferences(env) != 0)
        return JNI_ERR;
    if (RegisterMain(env) != 0)
        return JNI_ERR;
    return JNI_VERSION_1_6;
}
