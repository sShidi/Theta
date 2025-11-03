#ifndef TSFN_TYPES_HPP
#define TSFN_TYPES_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <future>
#include <napi.h>

// Enum for the type of value returned from JavaScript processor callback
enum CALLJS_PROMISE_RETURNED_VALUE_TYPE
{
    STRING,      // Returned a new string value
    OPERATION    // Returned NOOP or REMOVE symbol
};

// Result type from JavaScript processor callback
// If type == STRING: result contains the actual string value to set
// If type == OPERATION: result is either "NOOP" or "REMOVE"
struct CallJSPromiseType
{
    CALLJS_PROMISE_RETURNED_VALUE_TYPE type;
    std::string result;
};

// Data passed to JavaScript callback via TSFN
struct callJSData
{
    bool processFull;                           // true if key exists, false if empty
    std::promise<CallJSPromiseType>* result_promise;  // Promise to return result
    std::string_view key;                       // Record key
    std::string_view value;                     // Record value (empty if processFull=false)
};

// Type aliases for TypedThreadSafeFunction
using ContextType = std::nullptr_t;
using DataType = callJSData;

// Forward declaration of CallJS function
void CallJS(Napi::Env env, Napi::Function jsCallback, ContextType* context, DataType* data);

// TypedThreadSafeFunction type alias
using TSFN = Napi::TypedThreadSafeFunction<ContextType, DataType, CallJS>;

#endif //TSFN_TYPES_HPP