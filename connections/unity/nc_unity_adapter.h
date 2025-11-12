#ifndef UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H
#define UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Platform export macro
// -----------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
  #define NBC_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
  #define NBC_EXPORT __attribute__((visibility("default")))
#else
  #define NBC_EXPORT
#endif

// -----------------------------------------------------------------------------
// Callback typedefs (shared by native adapter and JNI bridge)
// -----------------------------------------------------------------------------
typedef void (*OnPeerFound_cb)(int endpoint_id, const char* name);
typedef void (*OnPeerLost_cb)(int endpoint_id);
typedef void (*OnConnectionRequested_cb)(int endpoint_id, const char* name);
typedef void (*OnConnectionEstablished_cb)(int endpoint_id);
typedef void (*OnConnectionDisconnected_cb)(int endpoint_id);
typedef void (*OnDataReceived_cb)(int endpoint_id, const void* data, int len);
typedef void (*OnPayloadProgress_cb)(int endpoint_id, long payload_id, long bytes, long total_bytes);

// -----------------------------------------------------------------------------
// Public API (C interface exposed to Unity C# side)
// -----------------------------------------------------------------------------
NBC_EXPORT void NBC_Initialize(const char* service_id);
NBC_EXPORT void NBC_Shutdown(void);

// Discovery / Advertising
NBC_EXPORT void NBC_StartAdvertising(void);
NBC_EXPORT void NBC_StopAdvertising(void);
NBC_EXPORT void NBC_StartDiscovery(void);
NBC_EXPORT void NBC_StopDiscovery(void);

// Messaging / Connection control
NBC_EXPORT void NBC_SendBytes(int endpoint_id, const void* data, int len);
NBC_EXPORT void NBC_AcceptConnection(int endpoint_id);
NBC_EXPORT void NBC_RejectConnection(int endpoint_id);
NBC_EXPORT void NBC_Disconnect(int endpoint_id);

// Callback registration
NBC_EXPORT void NBC_SetOnPeerFound(OnPeerFound_cb cb);
NBC_EXPORT void NBC_SetOnPeerLost(OnPeerLost_cb cb);
NBC_EXPORT void NBC_SetOnConnectionRequested(OnConnectionRequested_cb cb);
NBC_EXPORT void NBC_SetOnConnectionEstablished(OnConnectionEstablished_cb cb);
NBC_EXPORT void NBC_SetOnConnectionDisconnected(OnConnectionDisconnected_cb cb);
NBC_EXPORT void NBC_SetOnDataReceived(OnDataReceived_cb cb);
NBC_EXPORT void NBC_SetOnPayloadProgress(OnPayloadProgress_cb cb);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H