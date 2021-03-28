// Copyright 2016 Aether authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#include <jni.h>
#include <string>
#include "../../../../../model/model.h"


class MainPresenterAndroid : public MainPresenter {
public:
    AETHER_OBJ(MainPresenterAndroid, MainPresenter);
    template <typename T> void Serializator(T& s) { }
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
