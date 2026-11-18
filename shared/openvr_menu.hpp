#pragma once
#include "../third_party/openvr/include/openvr.h"
#include <cstdint>
#include <optional>
#include <string>

namespace cheeky {
inline std::string openvr_menu_key(std::uint32_t process_id) {
    return "cheeky.foveated_dlss.menu." + std::to_string(process_id);
}

struct OpenVRGazeFocusState {
    vr::EVRCompositorError wait_result{};
    bool pose_valid{};
    std::optional<bool> dashboard_visible;
    bool input_available{};
};

// Controller input can be captured independently of the scene and eye tracker.
// Use input availability only when dashboard visibility cannot be queried;
// otherwise a visible dashboard suspends gaze even if scene frames continue.
inline bool openvr_gaze_scene_focused(const OpenVRGazeFocusState& state) noexcept {
    return state.wait_result == vr::VRCompositorError_None &&
        state.pose_valid &&
        (state.dashboard_visible.has_value() ? !*state.dashboard_visible : state.input_available);
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
