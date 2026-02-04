#ifdef NX_WEB_MODULE

#include "nn_web.h"

NNWeb::NNWeb() {
}

void NNWeb::_bind_methods() {
    ClassDB::bind_method(D_METHOD("show_web_page", "url"), &NNWeb::show_web_page);
    ClassDB::bind_method(D_METHOD("get_last_url"), &NNWeb::get_last_url);

	ClassDB::bind_method(D_METHOD("set_media_audio_volume", "volume"), &NNWeb::set_media_audio_volume);
	ClassDB::bind_method(D_METHOD("get_media_audio_volume"), &NNWeb::get_media_audio_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "media_audio_volume"), "set_media_audio_volume", "get_media_audio_volume");

	ClassDB::bind_method(D_METHOD("set_web_audio_volume", "volume"), &NNWeb::set_web_audio_volume);
	ClassDB::bind_method(D_METHOD("get_web_audio_volume"), &NNWeb::get_web_audio_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "web_audio_volume"), "set_web_audio_volume", "get_web_audio_volume");

	ClassDB::bind_method(D_METHOD("set_background_kind", "kind"), &NNWeb::set_background_kind);
	ClassDB::bind_method(D_METHOD("get_background_kind"), &NNWeb::get_background_kind);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "background_kind", PROPERTY_HINT_ENUM, "Normal,ApplicationCapture,ApplicationCaptureBlur"), "set_background_kind", "get_background_kind");

	ClassDB::bind_method(D_METHOD("set_boot_as_media_player", "enabled"), &NNWeb::set_boot_as_media_player);
	ClassDB::bind_method(D_METHOD("get_boot_as_media_player"), &NNWeb::get_boot_as_media_player);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "boot_as_media_player"), "set_boot_as_media_player", "get_boot_as_media_player");

	ClassDB::bind_method(D_METHOD("set_boot_display_kind", "kind"), &NNWeb::set_boot_display_kind);
	ClassDB::bind_method(D_METHOD("get_boot_display_kind"), &NNWeb::get_boot_display_kind);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "boot_display_kind", PROPERTY_HINT_ENUM, "Default,White,Black,CallerCapture,CallerCaptureBlur"), "set_boot_display_kind", "get_boot_display_kind");

	ClassDB::bind_method(D_METHOD("set_callback_url", "url"), &NNWeb::set_callback_url);
	ClassDB::bind_method(D_METHOD("get_callback_url"), &NNWeb::get_callback_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "callback_url"), "set_callback_url", "get_callback_url");

	ClassDB::bind_method(D_METHOD("set_callbackable_url", "url"), &NNWeb::set_callbackable_url);
	ClassDB::bind_method(D_METHOD("get_callbackable_url"), &NNWeb::get_callbackable_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "callbackable_url"), "set_callbackable_url", "get_callbackable_url");

    ClassDB::bind_method(D_METHOD("set_display_url_kind", "kind"), &NNWeb::set_display_url_kind);
    ClassDB::bind_method(D_METHOD("get_display_url_kind"), &NNWeb::get_display_url_kind);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "display_url_kind", PROPERTY_HINT_ENUM, "FullPath,Domain"), "set_display_url_kind", "get_display_url_kind");

	ClassDB::bind_method(D_METHOD("set_footer_fixed_kind", "kind"), &NNWeb::set_footer_fixed_kind);
	ClassDB::bind_method(D_METHOD("get_footer_fixed_kind"), &NNWeb::get_footer_fixed_kind);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "footer_fixed_kind", PROPERTY_HINT_ENUM, "Auto,Shown"), "set_footer_fixed_kind", "get_footer_fixed_kind");

	ClassDB::bind_method(D_METHOD("set_left_stick_mode", "kind"), &NNWeb::set_left_stick_mode);
	ClassDB::bind_method(D_METHOD("get_left_stick_mode"), &NNWeb::get_left_stick_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "left_stick_mode", PROPERTY_HINT_ENUM, "StickPointer,SpatialNavi"), "set_left_stick_mode", "get_left_stick_mode");

	ClassDB::bind_method(D_METHOD("set_media_autoplay_enabled", "enabled"), &NNWeb::set_media_autoplay_enabled);
	ClassDB::bind_method(D_METHOD("get_media_autoplay_enabled"), &NNWeb::get_media_autoplay_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "media_autoplay_enabled"), "set_media_autoplay_enabled", "get_media_autoplay_enabled");

	ClassDB::bind_method(D_METHOD("set_media_player_auto_close_enabled", "enabled"), &NNWeb::set_media_player_auto_close_enabled);
	ClassDB::bind_method(D_METHOD("get_media_player_auto_close_enabled"), &NNWeb::get_media_player_auto_close_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "media_player_auto_close_enabled"), "set_media_player_auto_close_enabled", "get_media_player_auto_close_enabled");

	ClassDB::bind_method(D_METHOD("set_media_player_closed_caption_control_enabled", "enabled"), &NNWeb::set_media_player_closed_caption_control_enabled);
	ClassDB::bind_method(D_METHOD("get_media_player_closed_caption_control_enabled"), &NNWeb::get_media_player_closed_caption_control_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "media_player_closed_caption_control_enabled"), "set_media_player_closed_caption_control_enabled", "get_media_player_closed_caption_control_enabled");

	ClassDB::bind_method(D_METHOD("set_media_player_speed_control_enabled", "enabled"), &NNWeb::set_media_player_speed_control_enabled);
	ClassDB::bind_method(D_METHOD("get_media_player_speed_control_enabled"), &NNWeb::get_media_player_speed_control_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "media_player_speed_control_enabled"), "set_media_player_speed_control_enabled", "get_media_player_speed_control_enabled");

	ClassDB::bind_method(D_METHOD("set_page_cache_enabled", "enabled"), &NNWeb::set_page_cache_enabled);
	ClassDB::bind_method(D_METHOD("get_page_cache_enabled"), &NNWeb::get_page_cache_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "page_cache_enabled"), "set_page_cache_enabled", "get_page_cache_enabled");

	ClassDB::bind_method(D_METHOD("set_page_fade_enabled", "enabled"), &NNWeb::set_page_fade_enabled);
	ClassDB::bind_method(D_METHOD("get_page_fade_enabled"), &NNWeb::get_page_fade_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "page_fade_enabled"), "set_page_fade_enabled", "get_page_fade_enabled");

	ClassDB::bind_method(D_METHOD("set_page_scroll_indicator_enabled", "enabled"), &NNWeb::set_page_scroll_indicator_enabled);
	ClassDB::bind_method(D_METHOD("get_page_scroll_indicator_enabled"), &NNWeb::get_page_scroll_indicator_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "page_scroll_indicator_enabled"), "set_page_scroll_indicator_enabled", "get_page_scroll_indicator_enabled");

	ClassDB::bind_method(D_METHOD("set_pointer_enabled", "enabled"), &NNWeb::set_pointer_enabled);
	ClassDB::bind_method(D_METHOD("get_pointer_enabled"), &NNWeb::get_pointer_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pointer_enabled"), "set_pointer_enabled", "get_pointer_enabled");

	ClassDB::bind_method(D_METHOD("set_touch_enabled", "enabled"), &NNWeb::set_touch_enabled);
	ClassDB::bind_method(D_METHOD("get_touch_enabled"), &NNWeb::get_touch_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "touch_enabled"), "set_touch_enabled", "get_touch_enabled");

	ClassDB::bind_method(D_METHOD("set_web_audio_enabled", "enabled"), &NNWeb::set_web_audio_enabled);
	ClassDB::bind_method(D_METHOD("get_web_audio_enabled"), &NNWeb::get_web_audio_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "web_audio_enabled"), "set_web_audio_enabled", "get_web_audio_enabled");

	ClassDB::bind_method(D_METHOD("set_js_extension_enabled", "enabled"), &NNWeb::set_js_extension_enabled);
	ClassDB::bind_method(D_METHOD("get_js_extension_enabled"), &NNWeb::get_js_extension_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "js_extension_enabled"), "set_js_extension_enabled", "get_js_extension_enabled");

	ClassDB::bind_method(D_METHOD("set_user_agent_additional_string", "string"), &NNWeb::set_user_agent_additional_string);
	ClassDB::bind_method(D_METHOD("get_user_agent_additional_string"), &NNWeb::get_user_agent_additional_string);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "user_agent_additional_string"), "set_user_agent_additional_string", "get_user_agent_additional_string");

    // Signal emitted when the web applet exits
    ADD_SIGNAL(MethodInfo("exited", PropertyInfo(Variant::INT, "reason")));

    // Exit reason enum (exposed as NNWeb.ExitReason.*)
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "EndButtonPressed", (int)EndButtonPressed);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "BackButtonPressed", (int)BackButtonPressed);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "ExitMessage", (int)ExitMessage);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "CallbackUrlReached", (int)CallbackUrlReached);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "LastWindowDeleted", (int)LastWindowDeleted);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "LocalNetworkDisconnectionDenied", (int)LocalNetworkDisconnectionDenied);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "MediaPlayerClosed", (int)MediaPlayerClosed);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "CausedByWebPage", (int)CausedByWebPage);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "UserSelectionCanceled", (int)UserSelectionCanceled);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "NetworkConnectionFailed", (int)NetworkConnectionFailed);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "SystemUpdateRequired", (int)SystemUpdateRequired);
    ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "CallerDataCorrupted", (int)CallerDataCorrupted);
	ClassDB::bind_integer_constant(get_class_static(), StringName("ExitReason"), "Unexpected", (int)Unexpected);

	// Display URL kind enum (exposed as NNWeb.BackgroundKind.*)
	ClassDB::bind_integer_constant(get_class_static(), StringName("BackgroundKind"), "Normal", (int)Normal);
	ClassDB::bind_integer_constant(get_class_static(), StringName("BackgroundKind"), "ApplicationCapture", (int)ApplicationCapture);
	ClassDB::bind_integer_constant(get_class_static(), StringName("BackgroundKind"), "ApplicationCaptureBlur", (int)ApplicationCaptureBlur);

    // Display URL kind enum (exposed as NNWeb.DisplayUrlKind.*)
    ClassDB::bind_integer_constant(get_class_static(), StringName("DisplayUrlKind"), "FullPath", (int)FullPath);
    ClassDB::bind_integer_constant(get_class_static(), StringName("DisplayUrlKind"), "Domain", (int)Domain);

    // Boot display kind enum (exposed as NNWeb.BootDisplayKind.*)
    ClassDB::bind_integer_constant(get_class_static(), StringName("BootDisplayKind"), "Default", (int)Default);
    ClassDB::bind_integer_constant(get_class_static(), StringName("BootDisplayKind"), "White", (int)White);
    ClassDB::bind_integer_constant(get_class_static(), StringName("BootDisplayKind"), "Black", (int)Black);
    ClassDB::bind_integer_constant(get_class_static(), StringName("BootDisplayKind"), "CallerCapture", (int)CallerCapture);
	ClassDB::bind_integer_constant(get_class_static(), StringName("BootDisplayKind"), "CallerCaptureBlur", (int)CallerCaptureBlur);

	// Display URL kind enum (exposed as NNWeb.FooterFixedKind.*)
	ClassDB::bind_integer_constant(get_class_static(), StringName("FooterFixedKind"), "Auto", (int)Auto);
	ClassDB::bind_integer_constant(get_class_static(), StringName("FooterFixedKind"), "Shown", (int)Shown);

	// Display URL kind enum (exposed as NNWeb.LeftStickMode.*)
	ClassDB::bind_integer_constant(get_class_static(), StringName("LeftStickMode"), "StickPointer", (int)StickPointer);
	ClassDB::bind_integer_constant(get_class_static(), StringName("LeftStickMode"), "SpatialNavi", (int)SpatialNavi);
}

void NNWeb::show_web_page(const String &url) {
#ifndef NX_ENABLED
    // Just log and emit an "Unexpected" exit
    print_line("Open Web Page: " + url);
    last_url = String();
    emit_signal("exited", static_cast<int>(Unexpected));
    return;
#else
    CharString url_utf8 = url.utf8();
    nn::web::ShowWebPageArg pageArg(url_utf8.get_data());

    // Apply options
	if (media_audio_volume_set) pageArg.OverrideMediaAudioVolume(media_audio_volume);
	if (web_audio_volume_set) pageArg.OverrideWebAudioVolume(web_audio_volume);
	pageArg.SetBackgroundKind(static_cast<nn::web::WebBackgroundKind>(background_kind));
    pageArg.SetBootAsMediaPlayer(boot_as_media_player);
	pageArg.SetBootDisplayKind(static_cast<nn::web::WebBootDisplayKind>(boot_display_kind));
	if (callback_url_set) {
		CharString cb_utf8 = callback_url.utf8();
		pageArg.SetCallbackUrl(cb_utf8.get_data());
	}
	if (callbackable_url_set) {
		CharString cba_utf8 = callbackable_url.utf8();
		pageArg.SetCallbackableUrl(cba_utf8.get_data());
	}
	pageArg.SetDisplayUrlKind(static_cast<nn::web::WebDisplayUrlKind>(display_url_kind));
	pageArg.SetFooterFixedKind(static_cast<nn::web::WebFooterFixedKind>(footer_fixed_kind));
	pageArg.SetLeftStickMode(static_cast<nn::web::WebLeftStickMode>(left_stick_mode));
    pageArg.SetMediaAutoPlayEnabled(media_autoplay_enabled);
    pageArg.SetMediaPlayerAutoCloseEnabled(media_player_auto_close_enabled);
    pageArg.SetMediaPlayerClosedCaptionControlEnabled(media_player_closed_caption_control_enabled);
    pageArg.SetMediaPlayerSpeedControlEnabled(media_player_speed_control_enabled);
	pageArg.SetPageCacheEnabled(page_cache_enabled);
	pageArg.SetPageFadeEnabled(page_fade_enabled);
	pageArg.SetPageScrollIndicatorEnabled(page_scroll_indicator_enabled);
	pageArg.SetPointerEnabled(pointer_enabled);
	pageArg.SetTouchEnabledOnContents(touch_enabled);
	if (user_agent_additional_string_set) {
	 CharString ua_utf8 = user_agent_additional_string.utf8();
	 pageArg.SetUserAgentAdditionalString(ua_utf8.get_data());
	}
	pageArg.SetWebAudioEnabled(web_audio_enabled);
	pageArg.SetJsExtensionEnabled(js_extension_enabled);

    nn::web::WebPageReturnValue webPageReturnValue;
    const nn::Result result = nn::web::ShowWebPage(&webPageReturnValue, pageArg);
    if (result.IsSuccess())
    {
        nn::web::WebExitReason exitReason = webPageReturnValue.GetWebExitReason();

        // Capture last URL only when the callback URL was reached.
        if (exitReason == nn::web::WebExitReason_CallbackUrlReached) {
            size_t sz = webPageReturnValue.GetLastUrlSize();
            if (sz > 0) {
                const char *ptr = webPageReturnValue.GetLastUrl();
                // Use null-termination as per nn::web contract; sz is the maximum size.
                last_url = ptr ? String::utf8(ptr) : String();
            } else {
                last_url = String();
            }
        } else {
            last_url = String();
        }

        int mapped = (int)Unexpected;
        switch (exitReason) {
            case nn::web::WebExitReason_EndButtonPressed: mapped = (int)EndButtonPressed; break;
            case nn::web::WebExitReason_BackButtonPressed: mapped = (int)BackButtonPressed; break;
            case nn::web::WebExitReason_ExitMessage: mapped = (int)ExitMessage; break;
            case nn::web::WebExitReason_CallbackUrlReached: mapped = (int)CallbackUrlReached; break;
            case nn::web::WebExitReason_LastWindowDeleted: mapped = (int)LastWindowDeleted; break;
            case nn::web::WebExitReason_LocalNetworkDisconnectionDenied: mapped = (int)LocalNetworkDisconnectionDenied; break;
            case nn::web::WebExitReason_MediaPlayerClosed: mapped = (int)MediaPlayerClosed; break;
            case nn::web::WebExitReason_CausedByWebPage: mapped = (int)CausedByWebPage; break;
            case nn::web::WebExitReason_UserSelectionCanceled: mapped = (int)UserSelectionCanceled; break;
            case nn::web::WebExitReason_NetworkConnectionFailed: mapped = (int)NetworkConnectionFailed; break;
            case nn::web::WebExitReason_SystemUpdateRequired: mapped = (int)SystemUpdateRequired; break;
            case nn::web::WebExitReason_CallerDataCorrupted: mapped = (int)CallerDataCorrupted; break;
            case nn::web::WebExitReason_Unexpected: mapped = (int)Unexpected; break;
            default: mapped = (int)Unexpected; break;
        }
        emit_signal("exited", mapped);
    }
    else
    {
        print_line("Cancel");
        last_url = String();
        emit_signal("exited", (int)Unexpected);
    }
#endif
}

String NNWeb::get_last_url() const {
    return last_url;
}
void NNWeb::set_media_audio_volume(float p_volume) {
	media_audio_volume_set = true;
	media_audio_volume = p_volume;
}
float NNWeb::get_media_audio_volume() const {
	if (media_audio_volume_set)
		return media_audio_volume;

	return 0;
}

void NNWeb::set_web_audio_volume(float p_volume) {
	web_audio_volume_set = true;
	web_audio_volume = p_volume;
}
float NNWeb::get_web_audio_volume() const {
	if (web_audio_volume_set)
		return web_audio_volume;

	return 0;
}

void NNWeb::set_background_kind(int p_kind) {
	background_kind = static_cast<BackgroundKind>(p_kind);
}
int NNWeb::get_background_kind() const {
	return background_kind;
}

void NNWeb::set_boot_as_media_player(bool p_enabled) {
	boot_as_media_player = p_enabled;
}
bool NNWeb::get_boot_as_media_player() const {
	return boot_as_media_player;
}

void NNWeb::set_boot_display_kind(int p_kind) {
	boot_display_kind = static_cast<BootDisplayKind>(p_kind);
}
int NNWeb::get_boot_display_kind() const {
	return boot_display_kind;
}

void NNWeb::set_display_url_kind(int p_kind) {
    display_url_kind = static_cast<DisplayUrlKind>(p_kind);
}
int NNWeb::get_display_url_kind() const {
    return display_url_kind;
}

void NNWeb::set_footer_fixed_kind(int p_kind) {
	footer_fixed_kind = static_cast<FooterFixedKind>(p_kind);
}
int NNWeb::get_footer_fixed_kind() const {
	return footer_fixed_kind;
}

void NNWeb::set_left_stick_mode(int p_kind) {
    left_stick_mode = static_cast<LeftStickMode>(p_kind);
}
int NNWeb::get_left_stick_mode() const {
    return left_stick_mode;
}

void NNWeb::set_callback_url(const String &p_url) {
    callback_url_set = true;
    callback_url = p_url;
}
String NNWeb::get_callback_url() const {
    if (callback_url_set)
        return callback_url;
    return String();
}

void NNWeb::set_callbackable_url(const String &p_url) {
    callbackable_url_set = true;
    callbackable_url = p_url;
}
String NNWeb::get_callbackable_url() const {
    if (callbackable_url_set)
        return callbackable_url;
    return String();
}

void NNWeb::set_media_autoplay_enabled(bool p_enabled) {
    media_autoplay_enabled = p_enabled;
}
bool NNWeb::get_media_autoplay_enabled() const {
    return media_autoplay_enabled;
}

void NNWeb::set_media_player_auto_close_enabled(bool p_enabled) {
	media_player_auto_close_enabled = p_enabled;
}
bool NNWeb::get_media_player_auto_close_enabled() const {
	return media_autoplay_enabled;
}

void NNWeb::set_media_player_closed_caption_control_enabled(bool p_enabled) {
	media_autoplay_enabled = p_enabled;
}
bool NNWeb::get_media_player_closed_caption_control_enabled() const {
	return media_player_closed_caption_control_enabled;
}

void NNWeb::set_media_player_speed_control_enabled(bool p_enabled) {
	media_player_speed_control_enabled = p_enabled;
}
bool NNWeb::get_media_player_speed_control_enabled() const {
	return media_player_speed_control_enabled;
}

void NNWeb::set_page_cache_enabled(bool p_enabled) {
	page_cache_enabled = p_enabled;
}
bool NNWeb::get_page_cache_enabled() const {
	return page_cache_enabled;
}

void NNWeb::set_page_fade_enabled(bool p_enabled) {
	page_fade_enabled = p_enabled;
}
bool NNWeb::get_page_fade_enabled() const {
	return page_fade_enabled;
}

void NNWeb::set_page_scroll_indicator_enabled(bool p_enabled) {
	page_scroll_indicator_enabled = p_enabled;
}
bool NNWeb::get_page_scroll_indicator_enabled() const {
    return page_scroll_indicator_enabled;
}

void NNWeb::set_user_agent_additional_string(const String &p_string) {
    user_agent_additional_string_set = true;
    user_agent_additional_string = p_string;
}
String NNWeb::get_user_agent_additional_string() const {
    if (user_agent_additional_string_set)
        return user_agent_additional_string;
    return String();
}

void NNWeb::set_pointer_enabled(bool p_enabled) {
    pointer_enabled = p_enabled;
}
bool NNWeb::get_pointer_enabled() const {
    return pointer_enabled;
}

void NNWeb::set_touch_enabled(bool p_enabled) {
	touch_enabled = p_enabled;
}
bool NNWeb::get_touch_enabled() const {
	return touch_enabled;
}

void NNWeb::set_web_audio_enabled(bool p_enabled) {
	web_audio_enabled = p_enabled;
}
bool NNWeb::get_web_audio_enabled() const {
	return web_audio_enabled;
}

void NNWeb::set_js_extension_enabled(bool p_enabled) {
    js_extension_enabled = p_enabled;
}
bool NNWeb::get_js_extension_enabled() const {
    return js_extension_enabled;
}

#endif // NX_WEB_MODULE