#pragma once

#include <ostream>
#include <sstream>
#include <stdexcept>

#include "common/helpers.h"

#include "value.h"


namespace secd {

    class Runtime {
    public:
        virtual Value compile(Value const & source) = 0;
        virtual Value run(Value const & code) = 0;
    }; // tlisp::Runtime

    /** Returns the car of given value.
     */
    inline Value car(Value const & from) {
        if (! from.isCons())
            throw std::runtime_error(STR("Cannot obtain car from cell " << from));
        return from.car();
    }

    /** Returns the cdr of given value.
     */
    inline Value cdr(Value const & from) {
        if (! from.isCons())
            throw std::runtime_error(STR("Cannot obtain cdr from cell " << from));
        return from.cdr();
    }

    /** Converts the given value into bool.

        Only nil and 0 convert to false, everything else is true. 
     */
    inline bool toBoolean(Value const & value) {
        if (value == Nil)
            return false;
        if (value.isInteger() && value.valueInt() == 0)
            return false;
        return true;
    }
    
    /** Prints given value to standard output.
     */
    void print(Value const & value);

    /** Reads a number from the standard input.
     */
    Value read();

} // namespace secd
