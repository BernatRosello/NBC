#include "nc_unity_adapter.h"

#include "connections/c/nc.h"

#include <iostream>
#include <string>
#include <mutex>

// -- Global instance handle --
static NC_INSTANCE g_instance = nullptr;
static std::mutex g_lock;

// -- Unity callbacks --
static OnPeerFound_cb g_onPeerFound = nullptr;
static OnPeerLost_cb g_onPeerLost = nullptr;
static OnConnectionInitiated_cb g_onConnectionRequested = nullptr;
static OnConnectionEstablished_cb g_onConnectionEstablished = nullptr;
static OnConnectionDisconnected_cb g_onConnectionDisconnected = nullptr;
static OnPayloadReceived_cb g_onDataReceived = nullptr;
static OnPayloadProgress_cb g_onPayloadProgress = nullptr;

void NBC_SetOnPeerFound(OnPeerFound_cb cb) { g_onPeerFound = cb; }
void NBC_SetOnPeerLost(OnPeerLost_cb cb) { g_onPeerLost = cb; }
void NBC_SetOnConnectionRequested(OnConnectionInitiated_cb cb) { g_onConnectionRequested = cb; }
void NBC_SetOnConnectionEstablished(OnConnectionEstablished_cb cb) { g_onConnectionEstablished = cb; }
void NBC_SetOnConnectionDisconnected(OnConnectionDisconnected_cb cb) { g_onConnectionDisconnected = cb; }
void NBC_SetOnDataReceived(OnPayloadReceived_cb cb) { g_onDataReceived = cb; }
void NBC_SetOnPayloadProgress(OnPayloadProgress_cb cb) { g_onPayloadProgress = cb; }

NBC_EXPORT void NBC_Initialize(const char* serviceId) {
    std::lock_guard<std::mutex> guard(g_lock);
    if (g_instance != nullptr) return;

    g_instance = NcCreateService();

    // ---- Discovery listener ----
    static NC_DISCOVERY_LISTENER discovery_listener;
    discovery_listener.endpoint_found_callback = [](NC_INSTANCE inst, int endpoint_id, const NC_DATA* info, const NC_DATA*, void*) {
        std::string name(info->data, info->size);
        if (g_onPeerFound) g_onPeerFound(endpoint_id, name.c_str());
    };
    discovery_listener.endpoint_lost_callback = [](NC_INSTANCE inst, int endpoint_id, void*) {
        if (g_onPeerLost) g_onPeerLost(endpoint_id);
    };

    // ---- Start discovery (we can start later explicitly) ----
    // Keep discovery_listener static so lambdas remain valid.
}

NBC_EXPORT void NBC_StartDiscovery() {
    if (!g_instance) return;
    NC_DATA sid = {(uint64_t)strlen("unity-nc"), (char*)"unity-nc"};

    NC_DISCOVERY_OPTIONS opts{};
    opts.common_options.strategy.type = NC_STRATEGY_TYPE_P2P_STAR;

    static NC_DISCOVERY_LISTENER listener;
    listener.endpoint_found_callback = [](NC_INSTANCE inst, int id, const NC_DATA* info, const NC_DATA*, void*) {
        if (g_onPeerFound) g_onPeerFound(id, std::string(info->data, info->size).c_str());
    };
    listener.endpoint_lost_callback = [](NC_INSTANCE inst, int id, void*) {
        if (g_onPeerLost) g_onPeerLost(id);
    };

    NcStartDiscovery(g_instance, &sid, &opts, &listener,
        [](NC_STATUS s, void*) { std::cout << "[NCU] Discovery started: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_StartAdvertising() {
    if (!g_instance) return;
    NC_DATA sid = {(uint64_t)strlen("unity-nc"), (char*)"unity-nc"};

    NC_ADVERTISING_OPTIONS adv_opts{};
    adv_opts.common_options.strategy.type = NC_STRATEGY_TYPE_P2P_STAR;

    static NC_CONNECTION_REQUEST_INFO conn_info{};
    conn_info.endpoint_info = {(uint64_t)strlen("UnityPeer"), (char*)"UnityPeer"};

    conn_info.initiated_callback = [](NC_INSTANCE inst, int id, const NC_CONNECTION_RESPONSE_INFO* info, void*) {
        if (g_onConnectionRequested) g_onConnectionRequested(id, "peer");
    };
    conn_info.accepted_callback = [](NC_INSTANCE inst, int id, void*) {
        if (g_onConnectionEstablished) g_onConnectionEstablished(id);
    };
    conn_info.disconnected_callback = [](NC_INSTANCE inst, int id, void*) {
        if (g_onConnectionDisconnected) g_onConnectionDisconnected(id);
    };

    NcStartAdvertising(g_instance, &sid, &adv_opts, &conn_info,
        [](NC_STATUS s, void*) { std::cout << "[NCU] Advertising started: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_SendBytes(int endpoint_id, const void* data, int len) {
    if (!g_instance || !data || len <= 0) return;

    NC_PAYLOAD payload{};
    payload.id = 1;
    payload.type = NC_PAYLOAD_TYPE_BYTES;
    payload.direction = NC_PAYLOAD_DIRECTION_OUTGOING;
    payload.content.bytes.content.data = (char*)data;
    payload.content.bytes.content.size = len;

    int ids[1] = {endpoint_id};
    NcSendPayload(g_instance, 1, ids, &payload,
        [](NC_STATUS s, void*) { std::cout << "[NCU] SendPayload: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_AcceptConnection(int endpoint_id) {
    if (!g_instance) return;
    NC_PAYLOAD_LISTENER pl{};
    pl.received_callback = [](NC_INSTANCE inst, int id, const NC_PAYLOAD* payload, void*) {
        if (payload->type == NC_PAYLOAD_TYPE_BYTES && g_onDataReceived) {
            g_onDataReceived(id, payload->content.bytes.content.data, payload->content.bytes.content.size);
        }
    };
    pl.progress_updated_callback = [](NC_INSTANCE inst, int id, const NC_PAYLOAD_PROGRESS_INFO* prog, void*) {
        if (g_onPayloadProgress)
            g_onPayloadProgress(id, prog->id, prog->bytes_transferred, prog->total_bytes);
    };

    NcAcceptConnection(g_instance, endpoint_id, pl,
        [](NC_STATUS s, void*) { std::cout << "[NCU] AcceptConnection: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_RejectConnection(int endpoint_id) {
    if (!g_instance) return;
    NcRejectConnection(g_instance, endpoint_id,
        [](NC_STATUS s, void*) { std::cout << "[NCU] RejectConnection: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_Disconnect(int endpoint_id) {
    if (!g_instance) return;
    NcDisconnectFromEndpoint(g_instance, endpoint_id,
        [](NC_STATUS s, void*) { std::cout << "[NCU] Disconnect: " << s << std::endl; },
        nullptr);
}

NBC_EXPORT void NBC_Shutdown() {
    std::lock_guard<std::mutex> guard(g_lock);
    if (!g_instance) return;
    NcCloseService(g_instance);
    g_instance = nullptr;
}
