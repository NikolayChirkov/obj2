#include <jni.h>
#include <string>
#include "../../../../../model/model.h"


class MainPresenterAndroid : public MainPresenter {
public:
    AETHER_OBJ(MainPresenterAndroid, MainPresenter);
    template <typename T> void Serializator(T& s, int flags) { }
    virtual bool OnEvent(const aether::Event::ptr& event) { return true; };
    virtual void OnLoaded() {};
};
AETHER_IMPL(MainPresenterAndroid);

static App::ptr app;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_aetherdoc_AppNode_nativeInit(
        JNIEnv* env,
        jobject /* this */,
        jstring p) {
    const char* path = env->GetStringUTFChars(p, nullptr);
    app = App::Create(path);
    auto r = env->NewStringUTF(path);
    env->ReleaseStringUTFChars(p, path);
    return r;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_aetherdoc_AppNode_nativeRelease(
        JNIEnv* env,
        jobject /* this */) {
    App::Release(std::move(app));
    return env->NewStringUTF("ok");
}
