/*************************************************************************/
/*  nn_utils.cpp                                          */
/*************************************************************************/

#include "nn_utils.h"

NNUtils::NNUtils() {
}

void NNUtils::_bind_methods() {
	ClassDB::bind_method(D_METHOD("enable_cpu_boost", "enabled"), &NNUtils::enable_cpu_boost);
	ClassDB::bind_method(D_METHOD("is_nswitch_2"), &NNUtils::is_nswitch_2);
	ClassDB::bind_method(D_METHOD("is_docked"), &NNUtils::is_docked);
}

void NNUtils::enable_cpu_boost(bool enable) {
#ifndef NX_ENABLED
	return;
#else
	if (enable) {
		nn::oe::SetCpuBoostMode( nn::oe::CpuBoostMode_FastLoad );
	} else {
		nn::oe::SetCpuBoostMode( nn::oe::CpuBoostMode_Normal );
	}
#endif
}

bool NNUtils::is_nswitch_2() {
#ifndef NX_ENABLED
	return false;
#else
	return nn::pl::IsRunningOnOunce();
#endif
}

bool NNUtils::is_docked() {
#ifndef NX_ENABLED
	return false;
#else
	return (nn::oe::GetOperationMode() == nn::oe::OperationMode::OperationMode_Console);
#endif
	
}