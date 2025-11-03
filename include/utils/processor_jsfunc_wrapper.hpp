#ifndef PROCESSOR_JSFUNC_WRAPPER_HPP
#define PROCESSOR_JSFUNC_WRAPPER_HPP

#include <string>
#include <string_view>
#include <future>
#include <iostream>
#include <tkrzw_dbm.h>
#include <tkrzw_dbm_poly.h>
#include <napi.h>
#include "tsfn_types.hpp"

/**
 * Wrapper class that implements Tkrzw's RecordProcessor interface
 * and bridges to JavaScript callback functions via ThreadSafeFunction
 */
class processor_jsfunc_wrapper : public tkrzw::DBM::RecordProcessor
{
    public:
        /**
         * Constructor
         * @param tsfn Thread-safe function to call JavaScript processor
         */
        explicit processor_jsfunc_wrapper(TSFN tsfn) : tsfn(tsfn)
        {}

        /**
         * Called by Tkrzw when processing an existing record
         * @param key Record key
         * @param value Current record value
         * @return New value, NOOP to keep unchanged, or REMOVE to delete
         */
        std::string_view ProcessFull(std::string_view key, std::string_view value) override;

        /**
         * Called by Tkrzw when processing a non-existent record
         * @param key Record key
         * @return New value to set, NOOP to do nothing, or REMOVE (same as NOOP for empty)
         */
        std::string_view ProcessEmpty(std::string_view key) override;

    private:
        std::string new_value_memory;  // Storage for new value returned from JS
        TSFN tsfn;                     // Thread-safe function for calling JavaScript
};

#endif //PROCESSOR_JSFUNC_WRAPPER_HPP