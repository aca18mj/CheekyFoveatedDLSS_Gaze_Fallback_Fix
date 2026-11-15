#pragma once
#include "../third_party/openvr/include/openvr.h"
#include <cstdint>
#include <string>

namespace cheeky {
inline std::string openvr_menu_key(std::uint32_t process_id) {
    return "cheeky.foveated_dlss.menu." + std::to_string(process_id);
}

struct OpenVRGazeFocusState {
    vr::EVRCompositorError wait_result{};
    bool pose_valid{};
    bool input_available{}; // Diagnostic only: controller routing is independent of gaze.
    bool dashboard_visible{};
    bool menu_exception{}; // Diagnostic only: menu capture is independent of gaze.
};

// Controller input can be captured independently of the scene and eye tracker.
// A successful scene frame and HMD pose establish focus; the dashboard still
// suspends gaze even if the compositor continues to accept scene frames.
inline bool openvr_gaze_scene_focused(const OpenVRGazeFocusState& state) noexcept {
    return state.wait_result == vr::VRCompositorError_None &&
        state.pose_valid && !state.dashboard_visible;
}

// Identify Cheeky's visible, interactive menu for diagnostics. Gaze focus is
// determined by the scene frame, pose and dashboard state above.
template<class Overlay>
bool openvr_menu_captures_input(Overlay& overlay, std::uint32_t process_id) {
    if (overlay.IsDashboardVisible()) return false;
    vr::VROverlayHandle_t handle{vr::k_ulOverlayHandleInvalid};
    if (overlay.FindOverlay(openvr_menu_key(process_id).c_str(), &handle) != vr::VROverlayError_None ||
        handle == vr::k_ulOverlayHandleInvalid || overlay.GetOverlayRenderingPid(handle) != process_id ||
        !overlay.IsOverlayVisible(handle)) return false;
    std::uint32_t flags{};
    vr::VROverlayInputMethod input{};
    return overlay.GetOverlayFlags(handle, &flags) == vr::VROverlayError_None &&
        (flags & vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible) != 0 &&
        overlay.GetOverlayInputMethod(handle, &input) == vr::VROverlayError_None &&
        input == vr::VROverlayInputMethod_Mouse;
}
}
