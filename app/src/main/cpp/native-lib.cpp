#include <jni.h>
#include <string>
#include <android/log.h>
#include <complex.h>
#include <pthread.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"JNI",__VA_ARGS__);

// 如果不用参数，不用补全参数
void dynamicTest() {
    LOGE("动态注册方法 dynamicTest");
}

// 如果要用到参数，方法需要补上参数 JNIEnv *env, jobject thiz
jint dynamicTest2(JNIEnv *env, jobject thiz, jint i) {
    LOGE("动态注册方法 dynamicTest2 i = %d", i);
    return 11;
}

JavaVM *_vm;

// 定义方法数组
static const JNINativeMethod method[] = {
        // java层方法名，返回值，native方法名
        {"dynamicTest", "()V", (void*)dynamicTest},
        {"dynamicTest2", "(I)I", (int*)dynamicTest2},
};

static const char *mClassName = "com/wzh/cmakedemo/MainActivity";

// java 一调用System.loadLibrary，native 就会调用这个方法
int JNI_OnLoad(JavaVM *vm, void *unused) {
    LOGE("JNI_OnLoad");
    // 给全局的JavaVM赋值
    _vm = vm;

    /**
     * 动态注册方法
     * 不用再写 extern "C" (静态注册)
     * 可以使方法名更短
     * 可以在反编译so时，困难点
     * android底层都是这样注册native方法的
     */
    // 获得JNIEnv
    JNIEnv *env = nullptr;
    // 小于0失败，等于0成功
    int result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (result != JNI_OK) { // JNI_OK = 0
        return -1;
    }
    // 获得class对象
    jclass clz = env->FindClass(mClassName);
    // 注册 最后参数是方法的个数
    env->RegisterNatives(clz, method, sizeof(method) / sizeof(JNINativeMethod));
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_wzh_cmakedemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wzh_cmakedemo_MainActivity_testJNI(JNIEnv *env, jobject thiz, jintArray i_,
                                            jobjectArray j_) {
    // 数组首元素的地址 i
    jint *i = env->GetIntArrayElements(i_, JNI_FALSE);
    jint length = env->GetArrayLength(i_);
    for (int k = 0; k < length; ++k) {
        LOGE("java int 数组为 %d", *(i + k));
    }
    // 0 刷新并释放java数组
    // 1 只刷新
    // 2 只释放
    env->ReleaseIntArrayElements(i_, i, 0);

    jint length2 = env->GetArrayLength(j_);
    for (int k = 0; k < length2; ++k) {
        // GetObjectArrayElement 获得 jobject类型的元素，强转为jstring类型
        jstring str = static_cast<jstring>(env->GetObjectArrayElement(j_, k));
        // 将jstring转成c++类型的string
        const char *string = env->GetStringUTFChars(str, NULL);
        LOGE("java str 数组为 %s", string);
        env->ReleaseStringUTFChars(str, string);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wzh_cmakedemo_MainActivity_test(JNIEnv *env, jobject thiz, jstring str, jintArray i_,
                                         jobjectArray j_) {
    const char *s = env->GetStringUTFChars(str, JNI_FALSE);
    LOGE("test方法 str为 %s", s);

    jint *i = env->GetIntArrayElements(i_, JNI_FALSE);
    jint length = env->GetArrayLength(i_);
    for (int j = 0; j < length; ++j) {
        LOGE("test方法 int 数组为 %d", *(i + j));
    }

    jint len = env->GetArrayLength(j_);
    for (int k = 0; k < len; ++k) {
        jstring jStr = static_cast<jstring>(env->GetObjectArrayElement(j_, k));
        const char *string = env->GetStringUTFChars(jStr, NULL);
        LOGE("test方法 str 数组为 %s", string);
    }
}

jobject bean;

extern "C"
JNIEXPORT void JNICALL
Java_com_wzh_cmakedemo_MainActivity_testBean(JNIEnv *env, jobject thiz, jobject bean) {
    jclass beanClz = env->FindClass("com/wzh/cmakedemo/Bean");
    jmethodID getI = env->GetMethodID(beanClz,"getI", "()I");
    jint i =env->CallIntMethod(bean, getI);
    LOGE("C++ 调用 java的方法getI(), i = %d", i);

    jmethodID setI = env->GetMethodID(beanClz, "setI", "(I)V");
    env->CallVoidMethod(bean, setI, 456);
    LOGE("C++ 调用 java的方法setI()");

    // 调用构造函数
    // 1 获得构造函数
    jmethodID construct = env->GetMethodID(beanClz, "<init>", "()V");
    // 2 调用构造函数
    env->NewObject(beanClz, construct);

    // 获得全局引用bean
    bean = env->NewGlobalRef(beanClz);
    // 删除全局引用bean
    env->DeleteGlobalRef(bean);

    // 获得局部引用
    jobject b = env->NewObject(beanClz,construct);

    // 获得弱引用（弱引用也称为弱全局引用）
    jobject b2 = env->NewWeakGlobalRef(beanClz);
    // 使用弱引用前，需要判断是否为空
    //对一个弱引用 与NULL相比较
    // true： 释放了
    // false: 还可以使用这个弱引用
    jboolean isEqual = env->IsSameObject(b2, NULL);
}

// 线程调用的方法
void* threadTask(void* args) {
    // native 线程附加到 java虚拟机
    // native线程和 java 线程是两个线程
    JNIEnv *env;
    jint res = _vm->AttachCurrentThread(&env, 0);
    if (res != JNI_OK) {
        return 0;
    }
    jobject obj = static_cast<jobject>(args);
    // 获得MainActivity的class对象
    jclass clz = env->GetObjectClass(obj);
    jmethodID jmethodId = env->GetMethodID(clz, "updateUI", "()V");
    env->CallVoidMethod(obj, jmethodId);

    // 删除全局变量
    env->DeleteGlobalRef(obj);
    // 分离线程
    _vm->DetachCurrentThread();
    return 0;
}

// jobj 要是全局变量(跨线程使用)
jobject jobj;
extern "C"
JNIEXPORT void JNICALL
Java_com_wzh_cmakedemo_MainActivity_testThread(JNIEnv *env, jobject thiz) {
    // 创建线程
    pthread_t pid;
    // 启动线程 最后个参数是自定义的，就是线程执行方法的参数 args
    jobj = env->NewGlobalRef(thiz);
    pthread_create(&pid,0,threadTask,jobj);
}