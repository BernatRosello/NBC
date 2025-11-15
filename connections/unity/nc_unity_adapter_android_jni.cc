// jni_nc_unity.cpp
// Build for Android (ABI-specific). Exports C API for Unity and JNI callbacks for Java.

#include "connections/unity/nc_unity_adapter.h"

#include <jni.h>
#include <string>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstring>
#include <cassert>

static JavaVM *g_jvm = nullptr;
static jclass g_nearbyBridgeClass = nullptr;
static std::mutex g_class_lock;

// use the shared typedef names from the header
static std::atomic<OnPeerFound_cb> g_onPeerFound{nullptr};
static std::atomic<OnPeerLost_cb> g_onPeerLost{nullptr};
static std::atomic<OnConnectionRequested_cb> g_onConnectionRequested{nullptr};
static std::atomic<OnConnectionEstablished_cb> g_onConnectionEstablished{nullptr};
static std::atomic<OnConnectionDisconnected_cb> g_onConnectionDisconnected{nullptr};
static std::atomic<OnDataReceived_cb> g_onDataReceived{nullptr};
static std::atomic<OnPayloadProgress_cb> g_onPayloadProgress{nullptr};

// -------------------- helpers --------------------
static JNIEnv *GetJNIEnv(bool attach_if_needed = true)
{
    if (!g_jvm)
        return nullptr;
    JNIEnv *env = nullptr;
    jint rc = g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (rc == JNI_OK)
        return env;
    if (attach_if_needed)
    {
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6;
        args.name = nullptr;
        args.group = nullptr;
        if (g_jvm->AttachCurrentThread(&env, &args) != JNI_OK)
        {
            return nullptr;
        }
        return env;
    }
    return nullptr;
}

static void DetachCurrentThreadIfAttached()
{
    // no-op: we leave threads attached. If you attach manually, detach in cleanup if needed.
}

// Find and cache NearbyBridge class
static bool EnsureNearbyBridgeClass(JNIEnv *env)
{
    std::lock_guard<std::mutex> lock(g_class_lock);
    if (g_nearbyBridgeClass)
        return true;
    jclass local = env->FindClass("com/bernatrosello/nearbybridge/NearbyBridge");
    if (!local)
        return false;
    g_nearbyBridgeClass = (jclass)env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return g_nearbyBridgeClass != nullptr;
}

static jmethodID GetStaticMethod(JNIEnv *env, const char *name, const char *sig)
{
    if (!EnsureNearbyBridgeClass(env))
        return nullptr;
    return env->GetStaticMethodID(g_nearbyBridgeClass, name, sig);
}

// Call Java static methods wrappers used by our exported C functions
static bool CallJava_void_StringArg(const char *methodName, const char *arg)
{
    JNIEnv *env = GetJNIEnv();
    if (!env)
        return false;
    jmethodID mid = GetStaticMethod(env, methodName, "(Ljava/lang/String;)V");
    if (!mid)
        return false;
    jstring jarg = env->NewStringUTF(arg ? arg : "");
    env->CallStaticVoidMethod(g_nearbyBridgeClass, mid, jarg);
    env->DeleteLocalRef(jarg);
    return true;
}

static bool CallJava_void_NoArgs(const char *methodName)
{
    JNIEnv *env = GetJNIEnv();
    if (!env)
        return false;
    jmethodID mid = GetStaticMethod(env, methodName, "()V");
    if (!mid)
        return false;
    env->CallStaticVoidMethod(g_nearbyBridgeClass, mid);
    return true;
}

static bool CallJava_void_StringString(const char *methodName, const char *a, const char *b)
{
    JNIEnv *env = GetJNIEnv();
    if (!env)
        return false;
    jmethodID mid = GetStaticMethod(env, methodName, "(Ljava/lang/String;Ljava/lang/String;)V");
    if (!mid)
        return false;
    jstring ja = env->NewStringUTF(a ? a : "");
    jstring jb = env->NewStringUTF(b ? b : "");
    env->CallStaticVoidMethod(g_nearbyBridgeClass, mid, ja, jb);
    env->DeleteLocalRef(ja);
    env->DeleteLocalRef(jb);
    return true;
}

static bool CallJava_void_Int(const char *methodName, int value)
{
    JNIEnv *env = GetJNIEnv();
    if (!env)
        return false;
    jmethodID mid = GetStaticMethod(env, methodName, "(I)V");
    if (!mid)
        return false;
    env->CallStaticVoidMethod(g_nearbyBridgeClass, mid, value);
    return true;
}

static bool CallJava_void_Int_ByteArray(const char *methodName, int endpointId, const void *data, int len)
{
    JNIEnv *env = GetJNIEnv();
    if (!env)
        return false;
    jmethodID mid = GetStaticMethod(env, methodName, "(I[B)V");
    if (!mid)
        return false;
    jbyteArray arr = env->NewByteArray(len);
    env->SetByteArrayRegion(arr, 0, len, reinterpret_cast<const jbyte *>(data));
    env->CallStaticVoidMethod(g_nearbyBridgeClass, mid, endpointId, arr);
    env->DeleteLocalRef(arr);
    return true;
}

// -------------------- exported C API (P/Invoke) --------------------
// These names must match the DllImport signatures in your C# exactly.

NBC_EXPORT void NBC_SetOnPeerFound(OnPeerFound_cb cb)
{
    g_onPeerFound.store(cb);
}
NBC_EXPORT void NBC_SetOnPeerLost(OnPeerLost_cb cb)
{
    g_onPeerLost.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionRequested(OnConnectionRequested_cb cb)
{
    g_onConnectionRequested.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionEstablished(OnConnectionEstablished_cb cb)
{
    g_onConnectionEstablished.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionDisconnected(OnConnectionDisconnected_cb cb)
{
    g_onConnectionDisconnected.store(cb);
}
NBC_EXPORT void NBC_SetOnDataReceived(OnDataReceived_cb cb)
{
    g_onDataReceived.store(cb);
}

void NBC_SetOnPayloadProgress(OnPayloadProgress_cb cb) { g_onPayloadProgress.store(cb); }

// Initialize: call Java initialize(endpointName, serviceId)
NBC_EXPORT void NBC_Initialize(const char *endpointName, const char *serviceId)
{
    CallJava_void_StringArg("initialize", endpointName ? endpointName : "" ? serviceId : "");
}

// Shutdown
NBC_EXPORT void NBC_Shutdown()
{
    CallJava_void_NoArgs("shutdown");
}

// Advertising / discovery
NBC_EXPORT void NBC_StartAdvertising()
{
    // You may want to pass endpoint name; using fixed "UnityPeer" here, or adapt to pass nickname.
    CallJava_void_StringString("startAdvertising");
}
NBC_EXPORT void NBC_StopAdvertising()
{
    CallJava_void_NoArgs("stopAdvertising");
}
NBC_EXPORT void NBC_StartDiscovery()
{
    CallJava_void_StringArg("startDiscovery");
}
NBC_EXPORT void NBC_StopDiscovery()
{
    CallJava_void_NoArgs("stopDiscovery");
}

// Connection operations
NBC_EXPORT void NBC_AcceptConnection(int endpointId)
{
    CallJava_void_Int("acceptConnection", endpointId);
}
NBC_EXPORT void NBC_RejectConnection(int endpointId)
{
    CallJava_void_Int("rejectConnection", endpointId);
}
NBC_EXPORT void NBC_Disconnect(int endpointId)
{
    CallJava_void_Int("disconnect", endpointId);
}

// Send bytes
NBC_EXPORT void NBC_SendBytes(int endpointId, const void *data, int len)
{
    CallJava_void_Int_ByteArray("sendBytes", endpointId, data, len);
}

// -------------------- JNI callbacks from Java -> native --------------------
// Java will call these when events happen. Naming must match Java package/class/method or use RegisterNatives.

extern "C"
{

    // Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnPeerFound
    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnPeerFound(JNIEnv *env, jclass,
                                                                       jint endpointId, jstring name)
    {
        OnPeerFound_cb cb = g_onPeerFound.load();
        if (!cb)
            return;
        const char *s = env->GetStringUTFChars(name, nullptr);
        cb((int)endpointId, s);
        env->ReleaseStringUTFChars(name, s);
    }

    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnPeerLost(JNIEnv *env, jclass,
                                                                      jint endpointId)
    {
        OnPeerLost_cb cb = g_onPeerLost.load();
        if (cb)
            cb((int)endpointId);
    }

    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionRequested(JNIEnv *env, jclass,
                                                                                 jint endpointId, jstring name)
    {
        OnConnectionRequested_cb cb = g_onConnectionRequested.load();
        if (!cb)
            return;
        const char *s = env->GetStringUTFChars(name, nullptr);
        cb((int)endpointId, s);
        env->ReleaseStringUTFChars(name, s);
    }

    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionEstablished(JNIEnv *env, jclass,
                                                                                   jint endpointId)
    {
        OnConnectionEstablished_cb cb = g_onConnectionEstablished.load();
        if (cb)
            cb((int)endpointId);
    }

    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionDisconnected(JNIEnv *env, jclass,
                                                                                    jint endpointId)
    {
        OnConnectionDisconnected_cb cb = g_onConnectionDisconnected.load();
        if (cb)
            cb((int)endpointId);
    }

    JNIEXPORT void JNICALL
    Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnDataReceived(JNIEnv *env, jclass,
                                                                          jint endpointId, jbyteArray data)
    {
        OnDataReceived_cb cb = g_onDataReceived.load();
        if (!cb)
            return;
        jsize len = env->GetArrayLength(data);
        jbyte *buf = env->GetByteArrayElements(data, nullptr);
        cb((int)endpointId, buf, (int)len);
        env->ReleaseByteArrayElements(data, buf, JNI_ABORT);
    }

} // extern "C"

// -------------------- JNI_OnLoad to cache JavaVM --------------------
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    g_jvm = vm;
    JNIEnv *env = nullptr;
    if (g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }
    // Try to cache class reference if present (may not yet be loaded)
    jclass local = env->FindClass("com/bernatrosello/nearbybridge/NearbyBridge");
    if (local)
    {
        g_nearbyBridgeClass = (jclass)env->NewGlobalRef(local);
        env->DeleteLocalRef(local);
    }
    return JNI_VERSION_1_6;
}
