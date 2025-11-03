#include "../../include/utils/processor_jsfunc_wrapper.hpp"

std::string_view processor_jsfunc_wrapper::ProcessFull(std::string_view key, std::string_view value)
{
    // Create a promise to receive result from JavaScript
    std::promise<CallJSPromiseType>* result_promise = new std::promise<CallJSPromiseType>();
    std::future<CallJSPromiseType> result_future = result_promise->get_future();

    // Package data to send to JavaScript
    DataType* data = new callJSData{
        true,            // processFull = true (key exists)
        result_promise,
        key,
        value
    };

    // Call JavaScript function (blocking until JS callback completes)
    tsfn.BlockingCall(data);

    // Wait for JavaScript to return result
    CallJSPromiseType JSres = result_future.get();

    // Handle result based on type
    if(JSres.type == CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION)
    {
        if(JSres.result == "NOOP") {
            return NOOP;  // Keep record unchanged
        }
        else if(JSres.result == "REMOVE") {
            return REMOVE;  // Delete record
        }
    }
    else if(JSres.type == CALLJS_PROMISE_RETURNED_VALUE_TYPE::STRING)
    {
        // Store new value in member variable (must persist beyond function call)
        new_value_memory = JSres.result;
        return new_value_memory;
    }

    // Default: keep unchanged
    return NOOP;
}

std::string_view processor_jsfunc_wrapper::ProcessEmpty(std::string_view key)
{
    // Create a promise to receive result from JavaScript
    std::promise<CallJSPromiseType>* result_promise = new std::promise<CallJSPromiseType>();
    std::future<CallJSPromiseType> result_future = result_promise->get_future();

    // Package data to send to JavaScript
    DataType* data = new callJSData{
        false,           // processFull = false (key doesn't exist)
        result_promise,
        key,
        ""               // Empty value
    };

    // Call JavaScript function (blocking until JS callback completes)
    tsfn.BlockingCall(data);

    // Wait for JavaScript to return result
    CallJSPromiseType JSres = result_future.get();

    // Handle result based on type
    if(JSres.type == CALLJS_PROMISE_RETURNED_VALUE_TYPE::OPERATION)
    {
        if(JSres.result == "NOOP") {
            return NOOP;  // Don't create record
        }
        else if(JSres.result == "REMOVE") {
            return REMOVE;  // Same as NOOP for empty records
        }
    }
    else if(JSres.type == CALLJS_PROMISE_RETURNED_VALUE_TYPE::STRING)
    {
        // Store new value in member variable (must persist beyond function call)
        new_value_memory = JSres.result;
        return new_value_memory;
    }

    // Default: don't create record
    return NOOP;
}