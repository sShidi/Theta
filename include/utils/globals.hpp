#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <napi.h>

/**
 * Global symbols for processor return values
 *
 * These are initialized in polyDBM_wrapper::Init() and used to identify
 * special return values from JavaScript processor callbacks:
 *
 * - noopSym: Return this to keep record unchanged (don't modify)
 * - removeSym: Return this to delete the record
 *
 * Note: These are implemented as special strings rather than true Symbols
 * because Symbol comparison doesn't work reliably across execution contexts.
 */

// Declaration (extern means defined elsewhere)
extern Napi::String noopSym;
extern Napi::String removeSym;

#endif //GLOBALS_HPP