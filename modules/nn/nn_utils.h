/*************************************************************************/
/*  nn_controller_support.h                                            */
/*************************************************************************/

#ifndef NN_UTILS_H
#define NN_UTILS_H

#ifdef NX_ENABLED
	#include <nn/oe.h>
	#include <nn/hid/hid_Npad.h>
	#include <nn/hid/hid_NpadJoy.h>
	#include <nn/hid/hid_ControllerSupport.h>
	#include <nn/hid.h>
	#include <nn/pl.h>
#endif

#include "core/io/resource.h"

class NNUtils : public Resource {
	GDCLASS(NNUtils, Resource);

protected:
	static void _bind_methods();

public:
	void enable_cpu_boost(bool enable);
	bool is_nswitch_2();
	bool is_docked();
	
	NNUtils();
};

#endif // NN_UTILS_H
