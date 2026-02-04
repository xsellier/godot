#ifndef NN_WEB_H
#define NN_WEB_H

#ifdef NX_WEB_MODULE

#ifdef NX_ENABLED
#include <nn/os.h>
#include <nn/web.h>
#endif

#include "core/io/resource.h"
#include "core/variant/variant.h"
class NNWeb : public Resource {
    GDCLASS(NNWeb, Resource);

protected:
    static void _bind_methods();

public:
    // Exposed to GDScript as: NNWeb.BackgroundKind.*
    enum BackgroundKind {
        Normal = 0,
        ApplicationCapture,
        ApplicationCaptureBlur,
    };
    // Exposed to GDScript as: NNWeb.ExitReason.*
    enum ExitReason {
        EndButtonPressed = 0,
        BackButtonPressed,
        ExitMessage,
        CallbackUrlReached,
        LastWindowDeleted,
        LocalNetworkDisconnectionDenied,
        MediaPlayerClosed,
        CausedByWebPage,
        UserSelectionCanceled,
        NetworkConnectionFailed,
        SystemUpdateRequired,
        CallerDataCorrupted,
        Unexpected,
    };

    // Exposed to GDScript as: NNWeb.DisplayUrlKind.*
    enum DisplayUrlKind {
        FullPath = 0,
        Domain,
    };

    // Exposed to GDScript as: NNWeb.BootDisplayKind.*
    enum BootDisplayKind {
        Default = 0,
        White,
        Black,
        CallerCapture,
        CallerCaptureBlur,
    };

    // Exposed to GDScript as: NNWeb.FooterFixedKind.*
    enum FooterFixedKind {
        Auto = 0,
        Shown,
    };

    // Exposed to GDScript as: NNWeb.LeftStickMode.*
    enum LeftStickMode {
        StickPointer = 0,
        SpatialNavi,
    };

    void show_web_page(const String &url);

    // Options
    void set_media_audio_volume(float p_volume);
    float get_media_audio_volume() const;

    void set_web_audio_volume(float p_volume);
    float get_web_audio_volume() const;

    void set_background_kind(int p_kind);
    int get_background_kind() const;

    void set_boot_as_media_player(bool p_enabled);
    bool get_boot_as_media_player() const;

    void set_boot_display_kind(int p_kind);
    int get_boot_display_kind() const;

    void set_callback_url(const String &p_url);
    String get_callback_url() const;

    void set_callbackable_url(const String &p_url);
    String get_callbackable_url() const;

    void set_display_url_kind(int p_kind);
    int get_display_url_kind() const;

    void set_footer_fixed_kind(int p_kind);
    int get_footer_fixed_kind() const;

    void set_left_stick_mode(int p_kind);
    int get_left_stick_mode() const;

    void set_media_autoplay_enabled(bool p_enabled);
    bool get_media_autoplay_enabled() const;

    void set_media_player_auto_close_enabled(bool p_enabled);
    bool get_media_player_auto_close_enabled() const;

    void set_media_player_closed_caption_control_enabled(bool p_enabled);
    bool get_media_player_closed_caption_control_enabled() const;

    void set_media_player_speed_control_enabled(bool p_enabled);
    bool get_media_player_speed_control_enabled() const;

    void set_page_cache_enabled(bool p_enabled);
    bool get_page_cache_enabled() const;

    void set_page_fade_enabled(bool p_enabled);
    bool get_page_fade_enabled() const;

    void set_page_scroll_indicator_enabled(bool p_enabled);
    bool get_page_scroll_indicator_enabled() const;

    void set_pointer_enabled(bool p_enabled);
    bool get_pointer_enabled() const;

    void set_touch_enabled(bool p_enabled);
    bool get_touch_enabled() const;

    void set_web_audio_enabled(bool p_enabled);
    bool get_web_audio_enabled() const;

    void set_js_extension_enabled(bool p_enabled);
    bool get_js_extension_enabled() const;

    void set_user_agent_additional_string(const String &p_string);
    String get_user_agent_additional_string() const;

    String get_last_url() const;

    NNWeb();

private:
    bool media_audio_volume_set = false;
    float media_audio_volume;
    bool web_audio_volume_set = false;
    float web_audio_volume;
    BackgroundKind background_kind = Normal;
    bool boot_as_media_player = false;
    BootDisplayKind boot_display_kind = Default;
    bool callback_url_set = false;
    String callback_url;
    bool callbackable_url_set = false;
    String callbackable_url;
    DisplayUrlKind display_url_kind = FullPath;
    FooterFixedKind footer_fixed_kind = Auto;
    LeftStickMode left_stick_mode = SpatialNavi;
    bool media_autoplay_enabled = false;
    bool media_player_auto_close_enabled = false;
    bool media_player_closed_caption_control_enabled = false;
    bool media_player_speed_control_enabled = false;
    bool page_cache_enabled = false;
    bool page_fade_enabled = false;
    bool page_scroll_indicator_enabled = true;
    bool pointer_enabled = true;
    bool touch_enabled = true;
    bool user_agent_additional_string_set = false;
    String user_agent_additional_string;
    bool web_audio_enabled = false;
    bool js_extension_enabled = false;
    String last_url;
};

VARIANT_ENUM_CAST(NNWeb::ExitReason);
VARIANT_ENUM_CAST(NNWeb::BackgroundKind);
VARIANT_ENUM_CAST(NNWeb::BootDisplayKind);
VARIANT_ENUM_CAST(NNWeb::DisplayUrlKind);
VARIANT_ENUM_CAST(NNWeb::FooterFixedKind);
VARIANT_ENUM_CAST(NNWeb::LeftStickMode);
#endif // NX_WEB_MODULE

#endif // NN_WEB_H