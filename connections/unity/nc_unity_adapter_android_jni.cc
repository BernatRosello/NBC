#include <jni.h>
#include <atomic>
#include <string>
#include <cstring>

#include "nc_unity_adapter.h"

static JavaVM* g_vm = nullptr;
static jclass g_cls = nullptr;

static jmethodID g_m_initialize      = nullptr;
static jmethodID g_m_shutdown        = nullptr;
static jmethodID g_m_startDiscovery  = nullptr;
static jmethodID g_m_stopDiscovery   = nullptr;
static jmethodID g_m_startAdvertising= nullptr;
static jmethodID g_m_stopAdvertising = nullptr;
static jmethodID g_m_requestConn     = nullptr;
static jmethodID g_m_sendBytes       = nullptr;
static jmethodID g_m_acceptConn      = nullptr;
static jmethodID g_m_rejectConn      = nullptr;
static jmethodID g_m_disconnect      = nullptr;

// ---------------- Atomics for callbacks ----------------
static std::atomic<OnPeerFound_cb>                cb_peerFound{nullptr};
static std::atomic<OnPeerLost_cb>                 cb_peerLost{nullptr};
static std::atomic<OnConnectionInitiated_cb>      cb_connInit{nullptr};
static std::atomic<OnConnectionEstablished_cb>    cb_connOk{nullptr};
static std::atomic<OnConnectionDisconnected_cb>   cb_connDisc{nullptr};
static std::atomic<OnPayloadReceived_cb>          cb_payload{nullptr};


// ---------------- Helpers ----------------
static JNIEnv* GetEnv() {
    JNIEnv* env = nullptr;
    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
        g_vm->AttachCurrentThread(&env, nullptr);
    return env;
}

static jstring toJString(JNIEnv* env, const char* s) {
    return s ? env->NewStringUTF(s) : nullptr;
}

// ---------------- Cached class + methods ----------------
static void CacheJNI(JNIEnv* env) {
    jclass local = env->FindClass("com/bernatrosello/nearbybridge/NearbyBridge");
    g_cls = (jclass)env->NewGlobalRef(local);
    env->DeleteLocalRef(local);

    g_m_initialize       = env->GetStaticMethodID(g_cls, "initialize", "()V");
    g_m_shutdown         = env->GetStaticMethodID(g_cls, "shutdown", "()V");
    g_m_startDiscovery   = env->GetStaticMethodID(g_cls, "startDiscovery", "(Ljava/lang/String;ZI)V");
    g_m_stopDiscovery    = env->GetStaticMethodID(g_cls, "stopDiscovery", "()V");
    g_m_startAdvertising = env->GetStaticMethodID(g_cls, "startAdvertising", "(Ljava/lang/String;Ljava/lang/String;IZI)V");
    g_m_stopAdvertising  = env->GetStaticMethodID(g_cls, "stopAdvertising", "()V");
    g_m_requestConn      = env->GetStaticMethodID(g_cls, "requestConnection", "(Ljava/lang/String;Ljava/lang/String;)V");
    g_m_sendBytes        = env->GetStaticMethodID(g_cls, "sendBytes", "(Ljava/lang/String;[B)V");
    g_m_acceptConn       = env->GetStaticMethodID(g_cls, "acceptConnection", "(Ljava/lang/String;)V");
    g_m_rejectConn       = env->GetStaticMethodID(g_cls, "rejectConnection", "(Ljava/lang/String;)V");
    g_m_disconnect       = env->GetStaticMethodID(g_cls, "disconnect", "(Ljava/lang/String;)V");
}

// ----------------------------------------
// JNI OnLoad
// ----------------------------------------
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    g_vm = vm;
    JNIEnv* env = GetEnv();
    CacheJNI(env);
    return JNI_VERSION_1_6;
}

// ----------------------------------------
// Native callbacks called from Java
// ----------------------------------------
extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnPeerFound(
        JNIEnv* env, jclass,
        jstring endpointId, jstring name)
{
    auto cb = cb_peerFound.load();
    if (!cb) return;

    const char* id  = env->GetStringUTFChars(endpointId, nullptr);
    const char* nm  = env->GetStringUTFChars(name, nullptr);

    cb(id, nm);

    env->ReleaseStringUTFChars(endpointId, id);
    env->ReleaseStringUTFChars(name, nm);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnPeerLost(
        JNIEnv* env, jclass,
        jstring endpointId)
{
    auto cb = cb_peerLost.load();
    if (!cb) return;

    const char* id = env->GetStringUTFChars(endpointId, nullptr);
    cb(id);
    env->ReleaseStringUTFChars(endpointId, id);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionInitiated(
        JNIEnv* env, jclass,
        jstring endpointId, jstring name, jstring authDigits, jint authStatus)
{
    auto cb = cb_connInit.load();
    if (!cb) return;

    const char* id   = env->GetStringUTFChars(endpointId, nullptr);
    const char* nm   = env->GetStringUTFChars(name, nullptr);
    const char* auth = env->GetStringUTFChars(authDigits, nullptr);

    cb(id, nm, auth, authStatus);

    env->ReleaseStringUTFChars(endpointId, id);
    env->ReleaseStringUTFChars(name, nm);
    env->ReleaseStringUTFChars(authDigits, auth);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionEstablished(
        JNIEnv* env, jclass, jstring endpointId)
{
    auto cb = cb_connOk.load();
    if (!cb) return;

    const char* id = env->GetStringUTFChars(endpointId, nullptr);
    cb(id);
    env->ReleaseStringUTFChars(endpointId, id);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnConnectionDisconnected(
        JNIEnv* env, jclass, jstring endpointId)
{
    auto cb = cb_connDisc.load();
    if (!cb) return;

    const char* id = env->GetStringUTFChars(endpointId, nullptr);
    cb(id);
    env->ReleaseStringUTFChars(endpointId, id);
}

extern "C" JNIEXPORT void JNICALL
Java_com_bernatrosello_nearbybridge_NearbyBridge_nativeOnDataReceived(
        JNIEnv* env, jclass, jstring endpointId, jbyteArray data)
{
    auto cb = cb_payload.load();
    if (!cb) return;

    const char* id = env->GetStringUTFChars(endpointId, nullptr);
    jsize len = env->GetArrayLength(data);
    jbyte* ptr = env->GetByteArrayElements(data, nullptr);

    cb(id, reinterpret_cast<uint8_t*>(ptr), (size_t)len);

    env->ReleaseStringUTFChars(endpointId, id);
    env->ReleaseByteArrayElements(data, ptr, JNI_ABORT);
}


// ----------------------------------------
// C API forwarding to Java
// ----------------------------------------
extern "C" {

NBC_EXPORT void NBC_Initialize(void) {
    JNIEnv* env = GetEnv();
    env->CallStaticVoidMethod(g_cls, g_m_initialize);
}

NBC_EXPORT void NBC_Shutdown(void) {
    JNIEnv* env = GetEnv();
    env->CallStaticVoidMethod(g_cls, g_m_shutdown);
}

NBC_EXPORT void NBC_StartAdvertising(const char* endpointname, const char* serviceId,
                                     int connectionType, bool lowPower, int strategy)
{
    JNIEnv* env = GetEnv();
    jstring jname = toJString(env, endpointname);
    jstring jsid  = toJString(env, serviceId);

    env->CallStaticVoidMethod(g_cls, g_m_startAdvertising,
            jname, jsid, connectionType, (jboolean)lowPower, strategy);

    env->DeleteLocalRef(jname);
    env->DeleteLocalRef(jsid);
}

NBC_EXPORT void NBC_StopAdvertising(void) {
    GetEnv()->CallStaticVoidMethod(g_cls, g_m_stopAdvertising);
}

NBC_EXPORT void NBC_StartDiscovery(const char* serviceId, bool lowPower, int strategy)
{
    JNIEnv* env = GetEnv();
    jstring jsid = toJString(env, serviceId);
    env->CallStaticVoidMethod(g_cls, g_m_startDiscovery,
            jsid, (jboolean)lowPower, strategy);
    env->DeleteLocalRef(jsid);
}

NBC_EXPORT void NBC_StopDiscovery(void) {
    GetEnv()->CallStaticVoidMethod(g_cls, g_m_stopDiscovery);
}

NBC_EXPORT void NBC_RequestConnection(const char* name, const char* endpointId)
{
    JNIEnv* env = GetEnv();
    jstring jname = toJString(env, name);
    jstring jid   = toJString(env, endpointId);

    env->CallStaticVoidMethod(g_cls, g_m_requestConn, jname, jid);

    env->DeleteLocalRef(jname);
    env->DeleteLocalRef(jid);
}

NBC_EXPORT void NBC_SendBytes(const char* endpointId, const void* data, int len)
{
    JNIEnv* env = GetEnv();
    jstring jid = toJString(env, endpointId);

    jbyteArray arr = env->NewByteArray(len);
    env->SetByteArrayRegion(arr, 0, len, reinterpret_cast<const jbyte*>(data));

    env->CallStaticVoidMethod(g_cls, g_m_sendBytes, jid, arr);

    env->DeleteLocalRef(jid);
    env->DeleteLocalRef(arr);
}

NBC_EXPORT void NBC_AcceptConnection(const char* endpointId)
{
    JNIEnv* env = GetEnv();
    jstring jid = toJString(env, endpointId);
    env->CallStaticVoidMethod(g_cls, g_m_acceptConn, jid);
    env->DeleteLocalRef(jid);
}

NBC_EXPORT void NBC_RejectConnection(const char* endpointId)
{
    JNIEnv* env = GetEnv();
    jstring jid = toJString(env, endpointId);
    env->CallStaticVoidMethod(g_cls, g_m_rejectConn, jid);
    env->DeleteLocalRef(jid);
}

NBC_EXPORT void NBC_Disconnect(const char* endpointId)
{
    JNIEnv* env = GetEnv();
    jstring jid = toJString(env, endpointId);
    env->CallStaticVoidMethod(g_cls, g_m_disconnect, jid);
    env->DeleteLocalRef(jid);
}


// ---------------------------------------------------------
// Setters for atomics — exactly 1:1 with .h file
// ---------------------------------------------------------
NBC_EXPORT void NBC_SetOnPeerFound(OnPeerFound_cb cb) {
    cb_peerFound.store(cb);
}
NBC_EXPORT void NBC_SetOnPeerLost(OnPeerLost_cb cb) {
    cb_peerLost.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionInitiated(OnConnectionInitiated_cb cb) {
    cb_connInit.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionEstablished(OnConnectionEstablished_cb cb) {
    cb_connOk.store(cb);
}
NBC_EXPORT void NBC_SetOnConnectionDisconnected(OnConnectionDisconnected_cb cb) {
    cb_connDisc.store(cb);
}
NBC_EXPORT void NBC_SetOnPayloadReceived(OnPayloadReceived_cb cb) {
    cb_payload.store(cb);
}

} // extern "C"
