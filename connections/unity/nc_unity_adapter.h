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
typedef void (*OnPeerFound_cb)(const char* endpointId, const char* name);
typedef void (*OnPeerLost_cb)(const char* endpointId);
typedef void (*OnConnectionInitiated_cb)(const char* endpointId, const char* name, const char* authDigits, int authStatus);
typedef void (*OnConnectionEstablished_cb)(const char* endpointId);
typedef void (*OnConnectionDisconnected_cb)(const char* endpointId);
typedef void (*OnPayloadReceived_cb)(const char* endpointId, const uint8_t* data, size_t len);
//typedef void (*OnPayloadProgress_cb)(const char* endpoint_id, long payload_id, long bytes, long total_bytes);

// -----------------------------------------------------------------------------
// Public API (C interface exposed to Unity C# side)
// -----------------------------------------------------------------------------
NBC_EXPORT void NBC_Initialize(void);
NBC_EXPORT void NBC_Shutdown(void);

// Discovery / Advertising
NBC_EXPORT void NBC_StartAdvertising(const char* endpointname, const char* serviceId, int connectionType, bool lowPower, int strategy);
NBC_EXPORT void NBC_StopAdvertising(void);
NBC_EXPORT void NBC_StartDiscovery(const char* serviceId, bool lowPower, int strategy);
NBC_EXPORT void NBC_StopDiscovery(void);

// Messaging / Connection control
NBC_EXPORT void NBC_RequestConnection(const char* name, const char* endpointId);
NBC_EXPORT void NBC_AcceptConnection(const char* endpointId);
NBC_EXPORT void NBC_RejectConnection(const char* endpointId);
NBC_EXPORT void NBC_Disconnect(const char* endpointId);
NBC_EXPORT void NBC_SendBytes(const char* endpointId, const void* data, int len);

// Callback registration
NBC_EXPORT void NBC_SetOnPeerFound(OnPeerFound_cb cb);
NBC_EXPORT void NBC_SetOnPeerLost(OnPeerLost_cb cb);
NBC_EXPORT void NBC_SetOnConnectionInitiated(OnConnectionInitiated_cb cb);
NBC_EXPORT void NBC_SetOnConnectionEstablished(OnConnectionEstablished_cb cb);
NBC_EXPORT void NBC_SetOnConnectionDisconnected(OnConnectionDisconnected_cb cb);
NBC_EXPORT void NBC_SetOnPayloadReceived(OnPayloadReceived_cb cb);
//NBC_EXPORT void NBC_SetOnPayloadProgress(OnPayloadProgress_cb cb);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H
