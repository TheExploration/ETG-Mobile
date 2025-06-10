#include "Menu.h"
#include "Includes/obfuscate.h"
bool iconValid, settingsValid, initValid;
void setText(JNIEnv *env, jobject obj, const char *text)
{
    // https://stackoverflow.com/a/33627640/3763113
    // A little JNI calls here. You really really need a great knowledge if you want to play with JNI stuff
    // Html.fromHtml("");
    jclass html = (*env).FindClass(OBFUSCATE("android/text/Html"));
    jmethodID fromHtml =
        (*env).GetStaticMethodID(html, OBFUSCATE("fromHtml"), OBFUSCATE("(Ljava/lang/String;)Landroid/text/Spanned;"));

    // setText("");
    jclass textView = (*env).FindClass(OBFUSCATE("android/widget/TextView"));
    jmethodID setText = (*env).GetMethodID(textView, OBFUSCATE("setText"), OBFUSCATE("(Ljava/lang/CharSequence;)V"));

    // Java string
    jstring jstr = (*env).NewStringUTF(text);
    (*env).CallVoidMethod(obj, setText, (*env).CallStaticObjectMethod(html, fromHtml, jstr));
}

jstring Icon(JNIEnv *env, jobject thiz)
{
    iconValid = true;

    // Use https://www.base64encode.org/ to encode your image to base64
    return env->NewStringUTF(
                    OBFUSCATE("iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAACXBIWXMAAA7EAAAOxAGVKw4bAAASjUlEQVR42u3deXAeZ2HH8e+zu+++lyTLsnVYvi1fiWI7ieM4F+SkkBLIxVVKaQt/0Bla6DCU0nYyQ2eg7VDaTmcaoANMr5SjQCYw0BKSEIcjIbFJGjuWDxLHsmVbsWzLsvRe++7u0z9eyRUhgUAsvbvy7zOTYyaZRNrn2e8+u++7uya4p9ciIuclR5tARAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARBRAEREARARBUBEFAARUQBERAEQEQVARBQAEVEAREQBEBEFQEQUABFRAEREARARBUBEFAARUQBERAEQEQVARBQAEZl13lz5RczZP539C3baP7dWgy2//lyaPp/m0lzy0jxIxmn8NbaGSmCo1C1hZIliC9biOA6ea8hlDHnf4jp28t9XEORFS2EDxjTmRRAZyoEhCGOi2GJji3EMrmPwPYeCb/Fde/bfj60CMKsDhYFSzXD4FAwMWQaO1Nk/HHPsdMx4JaQWRFhr8X2XYs6jq9Whr9thfW+GDUsNKzthXh4MNtWD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")
                    );
}

jstring IconWebViewData(JNIEnv *env, jobject thiz)
{
    iconValid = true;
    // WebView support GIF animation. Upload your image or GIF on imgur.com or other sites

    // From internet (Requires android.permission.INTERNET)
    // return env->NewStringUTF(OBFUSCATE("https://i.imgur.com/SujJ85j.gif"));

    // Base64 html:
    // return env->NewStringUTF("data:image/png;base64, <encoded base64 here>");

    // To disable it, return NULL. It will use normal image above:
    // return NULL

    // return env->NewStringUTF(OBFUSCATE_KEY("https://i.imgur.com/SujJ85j.gif", 'u'));
    return NULL;
}

jobjectArray SettingsList(JNIEnv *env, jobject activityObject)
{
    jobjectArray ret;

    const char *features[] = {
        OBFUSCATE("Category_Settings"),
        OBFUSCATE("-3_Toggle_Auto size vertically"),
        OBFUSCATE("Category_Menu"),
        OBFUSCATE("-6_Button_<font color='red'>Close settings</font>"),
    };

    int Total_Feature =
        (sizeof features / sizeof features[0]); // Now you dont have to manually update the number everytime;
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                            env->NewStringUTF(""));
    int i;
    for (i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    settingsValid = true;

    return (ret);
}
void setInitValid(bool valid)
{
    initValid = valid;
}
bool getInitValid()
{
    return initValid;
}
