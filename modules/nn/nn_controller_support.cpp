/*************************************************************************/
/*  nn_controller_support.cpp                                          */
/*************************************************************************/

#include "nn_controller_support.h"

#ifdef NX_ENABLED
#include "platform/nx/os_nx.h"
#endif

/*
static const nn::hid::NpadIdType s_NpadIds_monoplayer[] = {
	nn::hid::NpadId::No1,
	nn::hid::NpadId::No2,
	nn::hid::NpadId::No3,
	nn::hid::NpadId::No4,
	nn::hid::NpadId::Handheld
};

static const nn::hid::NpadIdType s_NpadIds_multiplayer[] = {
	nn::hid::NpadId::No1,
	nn::hid::NpadId::No2,
	nn::hid::NpadId::No3,
	nn::hid::NpadId::No4
};
*/

NNControllerSupport::NNControllerSupport() {
}

void NNControllerSupport::_bind_methods() {
	ClassDB::bind_method(D_METHOD("call_applet", "required_players"), &NNControllerSupport::call_applet);
	ClassDB::bind_method(D_METHOD("show_controller_strap_guide"), &NNControllerSupport::show_controller_strap_guide);
}

uint64_t NNControllerSupport::call_applet(uint64_t required_players) {
#ifndef NX_ENABLED
	return required_players + 1;
#else
	extern nn::os::MutexType g_Mutex;

	nn::hid::ControllerSupportResultInfo *resultInfo;
	nn::Result result;
	nn::hid::ControllerSupportArg arg;
	nn::hid::ControllerSupportResultInfo info;

	arg.SetDefault();
	if (required_players == 1) {
		disable_applet_support_calling_on_changed_connect = false;

		//  Single player gameplay.
		arg.enableSingleMode = true;

		nn::hid::SetSupportedNpadStyleSet(
			nn::hid::NpadStyleFullKey::Mask |
			nn::hid::NpadStyleJoyDual::Mask |
			nn::hid::NpadStyleHandheld::Mask
		);

		// nn::hid::SetSupportedNpadIdType(s_NpadIds_monoplayer, (sizeof(s_NpadIds_monoplayer) / sizeof(nn::hid::NpadIdType)));
	} else {
		disable_applet_support_calling_on_changed_connect = true;

		//  Multiplayer gameplay.
		arg.playerCountMin = arg.playerCountMax = static_cast<int8_t>(required_players);

		nn::hid::SetSupportedNpadStyleSet(
			nn::hid::NpadStyleJoyLeft::Mask |
			nn::hid::NpadStyleJoyRight::Mask |
			nn::hid::NpadStyleJoyDual::Mask |
			nn::hid::NpadStyleFullKey::Mask |
			nn::hid::NpadStyleHandheld::Mask
		);

		// nn::hid::SetSupportedNpadIdType(s_NpadIds_multiplayer, (sizeof(s_NpadIds_multiplayer) / sizeof(nn::hid::NpadIdType)));
	}

	arg.enableIdentificationColor = false;
	arg.enableExplainText = false;

	// Calls the controller support applet.
	result = nn::hid::ShowControllerSupport(&info, arg);

	if (result.IsSuccess()) {
		if (required_players == 1) {
			return 1;
		} else {
			return info.playerCount;
		}
	} else {
		return (info.playerCount) * -1;
	}
#endif
}

void NNControllerSupport::show_controller_strap_guide() {
#ifdef NX_ENABLED
	extern nn::os::MutexType g_Mutex;

	nn::hid::ShowControllerStrapGuide();
#endif
}
