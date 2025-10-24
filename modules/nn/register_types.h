/*************************************************************************/
/*  register_types.h                                                     */
/*************************************************************************/

#ifndef NN_REGISTER_TYPES_H
#define NN_REGISTER_TYPES_H

#include "modules/register_module_types.h"

// Ces fonctions seront appelées au moment de l'initialisation/termination du module.
void initialize_nn_module(ModuleInitializationLevel p_level);
void uninitialize_nn_module(ModuleInitializationLevel p_level);

#endif // NN_REGISTER_TYPES_H
