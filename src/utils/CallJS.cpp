#include "../../include/utils/tsfn_types.hpp"
#include "../../include/utils/globals.hpp"
#include <iostream>

/**
 * CallJS - Function called by TSFN when JavaScript callback needs to be invoked
 * This runs on the main thread and bridges C++ worker threads to JavaScript
 *
 * @param env Node-API environment
 * @param jsCallback JavaScript function to call
 * @param context Unused context (nullptr)
 * @param data Data from C++ worker thread containing key/value and promise
 */
void CallJS(Napi::Env env, Napi::Function jsCallback, ContextType* context, DataType* data)
{
    // Prepare arguments for JavaScript callback
    Napi::Boolean keyExists = Napi::Boolean::New(env, data->processFull);
    Napi::String key = Napi::String::New(env, std::string(data->key));
    Napi::String value = Napi::String::New(env, std::string(data->value));
    std::promise<CallJSPromiseType>* cppPromise = data->result_promise;

    // Call JavaScript function with (exists, key, value)
    Napi::Value jsRes = jsCallback.Call({keyExists, key, value});

    // Handle result - could be Promise or direct value
    if(jsRes.IsPromise())
    {
        // JavaScript returned a Promise
        Napi::Object promiseObj = jsRes.As<Napi::Object>();
        Napi::Function promise_then = promiseObj.Get("then").As<Napi::Function>();

        // Attach then/catch handlers
        promise_then.Call(jsRes, {
            // onFulfilled - Promise resolved
            Napi::Function::New(env, [cppPromise](const Napi::CallbackInfo& info)
            {
                CallJSPromiseType res;
                Napi::Value resolvedValue = info[0];

                // Check if result is NOOP or REMOVE symbol (represented as special strings)
                std::string resultStr = resolvedValue.ToString().Utf8Value();
                if(resultStr == "___TKRZW_NOOP___") {
                    res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION, "NOOP"};
                }
                else if(resultStr == "___TKRZW_REMOVE___") {
                    res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION, "REMOVE"};
                }
                else {
                    res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::STRING, resultStr};
                }

                cppPromise->set_value(res);
            }),

            // onRejected - Promise rejected (treat as NOOP)
            Napi::Function::New(env, [cppPromise](const Napi::CallbackInfo& info)
            {
                std::cerr << "JavaScript processor promise rejected, treating as NOOP" << std::endl;
                cppPromise->set_value(CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION, "NOOP"});
            })
        });
    }
    else
    {
        // JavaScript returned a direct value (not a Promise)
        CallJSPromiseType res;
        std::string resultStr = jsRes.ToString().Utf8Value();

        // Check if result is NOOP or REMOVE symbol
        if(resultStr == "___TKRZW_NOOP___") {
            res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION, "NOOP"};
        }
        else if(resultStr == "___TKRZW_REMOVE___") {
            res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION, "REMOVE"};
        }
        else {
            res = CallJSPromiseType{CALLJS_PROMISE_RETURNED_VALUE_TYPE::STRING, resultStr};
        }

        cppPromise->set_value(res);
    }

    // Clean up data (note: promise is owned by processor_jsfunc_wrapper)
    delete data;
}

/**
 * NOTE: Using string comparison instead of Symbol comparison
 *
 * Originally, we wanted to use Napi::Symbol for NOOP and REMOVE, but:
 * 1. Symbol.StrictEquals() doesn't work reliably across different execution contexts
 * 2. Converting symbols to strings works consistently
 *
 * The special strings "___TKRZW_NOOP___" and "___TKRZW_REMOVE___" are set in
 * polyDBM_wrapper::Init() as static properties on the polyDBM class.
 *
 * JavaScript usage:
 *   return polyDBM.NOOP;    // Don't change record
 *   return polyDBM.REMOVE;  // Delete record
 *   return "new value";     // Set new value
 */