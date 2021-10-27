#pragma once

#include <ostream>

#include "value.h"
#include "runtime.h"


namespace secd {

    class Stack {
    public:
        /** Creates an empty stack.
         */
        Stack():
            v_(Nil) {
        }

        Stack(Value const & v):
            v_(v) {
            assert((v == Nil || v.isCons()) && "Invalid value used as stack");
        }

        bool empty() const {
            return v_ == Nil;
        }

        Value top() {
            return v_.car();
        }

        Value pop() {
            Value result = v_.car();
            v_ = v_.cdr();
            return result;
        }

        void push(Value what) {
            v_ = Value::Cons(what, v_);
        }

        operator Value () {
            return v_;
        }
        
    private:
        friend std::ostream & operator << (std::ostream & s, Stack const & stack) {
            s << "Stack: " << stack.v_;
            return s;
        }
        Value v_;

    }; // tlisp::Stack

    /** Wrapper around the cons cells that makes using the tinyLISP lists from C++ easier.

     */
    class List {
    public:

        static void Expand(Value x, Value & first, Value & second) {
            first = x.car();
            x = x.cdr();
            second = x.car();
            if (x.cdr() != Nil)
                throw "Expected two elements in a list, but more found";
        }

        static void Expand(Value x, Value & first, Value & second, Value & third) {
            first = x.car();
            x = x.cdr();
            second = x.car();
            x = x.cdr();
            third = x.car();
            if (x.cdr() != Nil)
                throw "Expected two elements in a list, but more found";
        }
        
        List():
            first_(Nil),
            last_(Nil) {
        }

        List(std::initializer_list<Value> values):
            first_(Nil),
            last_(Nil) {
            if (values.size() > 0) {
                auto i = values.begin();
                first_ = last_ = Value::Cons(*i, Nil);
                ++i;
                for (auto e = values.end(); i != e; ++i) {
                    Value x = Value::Cons(*i, Nil);
                    last_.setCdr(x);
                    last_ = x;
                }
            }
        }

        void prepend(Value const & value) {
            first_ = Value::Cons(value, first_);
            // if this is the first element to be added to the list, update last as well
            if (last_ == Nil)
                last_ = first_;
        }

        void append(Value const & value) {
            if (last_ == Nil) {
                prepend(value);
            } else {
                Value x = Value::Cons(value, Nil);
                last_.setCdr(x);
                last_ = x;
            }
        }

        bool empty() const {
            return first_ == Nil;
        }

        operator Value () {
            return first_;
        }

    private:
        Value first_;
        Value last_;
    }; // tlisp::List

} // namespace secd


