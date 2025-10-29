
#ifndef UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H
#define UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H

#include "connections/c/nc.h"

#ifdef _WIN32
#define NBC_EXPORT extern "C" __declspec(dllexport)
#else
#define NBC_EXPORT extern "C"
#endif

// Unity-side event hooks
typedef void (*OnPeerFoundCallback)(int endpoint_id, const char* name);
typedef void (*OnPeerLostCallback)(int endpoint_id);
typedef void (*OnConnectionRequestedCallback)(int endpoint_id, const char* name);
typedef void (*OnConnectionEstablishedCallback)(int endpoint_id);
typedef void (*OnConnectionDisconnectedCallback)(int endpoint_id);
typedef void (*OnDataReceivedCallback)(int endpoint_id, const void* data, int len);
typedef void (*OnPayloadProgressCallback)(int endpoint_id, long payload_id, long bytes, long total_bytes);

NBC_EXPORT void NBC_Initialize(const char* serviceId);
NBC_EXPORT void NBC_Shutdown();

// Discovery / Advertising
NBC_EXPORT void NBC_StartAdvertising();
NBC_EXPORT void NBC_StartDiscovery();
NBC_EXPORT void NBC_StopAdvertising();
NBC_EXPORT void NBC_StopDiscovery();

// Messaging / Connection control
NBC_EXPORT void NBC_SendBytes(int endpoint_id, const void* data, int len);
NBC_EXPORT void NBC_AcceptConnection(int endpoint_id);
NBC_EXPORT void NBC_RejectConnection(int endpoint_id);
NBC_EXPORT void NBC_Disconnect(int endpoint_id);

// Callback registration
NBC_EXPORT void NBC_SetOnPeerFound(OnPeerFoundCallback cb);
NBC_EXPORT void NBC_SetOnPeerLost(OnPeerLostCallback cb);
NBC_EXPORT void NBC_SetOnConnectionRequested(OnConnectionRequestedCallback cb);
NBC_EXPORT void NBC_SetOnConnectionEstablished(OnConnectionEstablishedCallback cb);
NBC_EXPORT void NBC_SetOnConnectionDisconnected(OnConnectionDisconnectedCallback cb);
NBC_EXPORT void NBC_SetOnDataReceived(OnDataReceivedCallback cb);
NBC_EXPORT void NBC_SetOnPayloadProgress(OnPayloadProgressCallback cb);

#endif  // UNITY_NGO_TRANSPORT_NEARBY_CONNECTIONS_ADAPTER_C_H
