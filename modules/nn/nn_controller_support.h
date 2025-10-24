/*************************************************************************/
/*  nn_controller_support.h                                            */
/*************************************************************************/

#ifndef NN_CONTROLLER_SUPPORT_H
#define NN_CONTROLLER_SUPPORT_H

#ifdef NX_ENABLED
	#include <nn/oe.h>
	#include <nn/hid/hid_Npad.h>
	#include <nn/hid/hid_NpadJoy.h>
	#include <nn/hid/hid_ControllerSupport.h>
	#include <nn/hid.h>
#endif

#include "core/io/resource.h"

class NNControllerSupport : public Resource {
	GDCLASS(NNControllerSupport, Resource);

protected:
	static void _bind_methods();

public:
	uint64_t call_applet(uint64_t required_players);
	void show_controller_strap_guide();
	
	NNControllerSupport();
};

#endif // NN_CONTROLLER_SUPPORT_H
