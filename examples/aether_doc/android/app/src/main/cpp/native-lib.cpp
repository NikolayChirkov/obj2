#include <jni.h>
#include <string>
#include "../../../../../model/model.h"


class MainPresenterAndroid : public MainPresenter {
public:
    AETHER_OBJECT(MainPresenterAndroid);
    AETHER_SERIALIZE(MainPresenterAndroid, MainPresenter);
    AETHER_INTERFACES(MainPresenterAndroid, MainPresenter);
    template <typename T> void Serializator(T& s, int flags) { }
    virtual bool OnEvent(const aether::Event::ptr& event) { return true; };
    virtual void OnLoaded() {};
};
AETHER_IMPLEMENTATION(MainPresenterAndroid);

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_aetherdoc_MainActivity_initAppJNI(
        JNIEnv* env,
        jobject /* this */,
        jstring p) {
    const char* path = env->GetStringUTFChars(p, nullptr);
    App::ptr app = App::Create(path);
    env->ReleaseStringUTFChars(p, path);
    std::string hello = "Hello from CPP";
    return env->NewStringUTF(hello.c_str());
}