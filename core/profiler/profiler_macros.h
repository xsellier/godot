// Common profiler backend for godot-nx.
// Intended to reduce the amount of ifdefs that need to be injected into Godot's core.
 
#ifndef PROFILER_MACROS_H
#define PROFILER_MACROS_H
 
/* Profiler API */
#if !PROFILER_ENABLED || TOOLS_ENABLED
// const char *id: String identifier.
#define PROF_SCOPED_BLOCK(id)
#define PROF_PUSH_BLOCK(id)
#define PROF_POP_BLOCK()
#define PROF_ENTER_BLOCK(id)
#define PROF_EXIT_BLOCK(id)
#else
 
/* Profiler Implementations */
#ifdef NX_ENABLED
#include <nn/profiler.h>
#define PROF_SCOPED_BLOCK(id) nn::profiler::ScopedCodeBlock __prof_scoped_code_block(id)
#define PROF_PUSH_BLOCK(id) nn::profiler::PushCodeBlock(id)
#define PROF_POP_BLOCK() nn::profiler::PopCodeBlock()
#define PROF_ENTER_BLOCK(id) nn::profiler::EnterCodeBlock(id)
#define PROF_EXIT_BLOCK(id) nn::profiler::ExitCodeBlock(id)
#endif // NX_ENABLED
 
#endif // PROFILER_ENABLED
#endif // PROFILER_MACROS_H
