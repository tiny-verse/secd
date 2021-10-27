#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>

#include "gc.h"

namespace secd {

    /** Representation of a tinyLISP object.

        The value acts both as a smart pointer that registers itself automatically as a GC root when used and as a checked accessor to the contents of the underlying GC Cell. 
     */
    class Value {
    public:

        /** Default constructor for a value which initializes it to Nil.
         */
        Value();

        /** Copy constructor for a value.
         */
        Value(Value const & from):
            data_(from.data_) {
            GC::AddRoot(data_);
        }

        static Value Integer(int64_t value) {
            return Value(new GC::Cell(GC::CellKind::Integer, value));
        }

        static Value Cons(Value const & car, Value const & cdr) {
            return Value(new GC::Cell(GC::CellKind::Cons, car.data_, cdr.data_));
        }

        static Value Closure(Value const & body, Value const & environment) {
            return Value(new GC::Cell(GC::CellKind::Closure, body.data_, environment.data_));
        }

        /** Value destructor removes the value from the list of GC roots.
         */
        ~Value() {
            GC::RemoveRoot(data_);    
        }

        /** Returns the kind of the value, i.e. the kind of its underlying cell.
         */
        GC::CellKind kind() const {
            return data_->kind;
        }

        bool isInteger() const {
            return kind() == GC::CellKind::Integer;
        }
        
        bool isSymbol() const {
            return kind() == GC::CellKind::Symbol;
        }
        
        bool isCons() const {
            return kind() == GC::CellKind::Cons;
        }
        
        bool isClosure() const {
            return kind() == GC::CellKind::Closure;
        }

        int64_t valueInt() const {
            assert(isInteger() && "Accessing numeric value of non-integer cell");
            return data_->valueInt;
        }

        void setValueInt(int value) {
            data_->kind = GC::CellKind::Integer;
            data_->valueInt = value;
        }

        std::string const & name() const {
            assert(isSymbol() && "Accessing name of non-symbol cell");
            return * data_->name;
        }

        Value car() const {
            assert(isCons() && "Accessing car of non-cons cell");
            return data_->car;
        }

        Value cdr() const {
            assert(isCons() && "Accessing cdr of non-cons cell");
            return data_->cdr;
        }

        void setCar(Value const & value) {
            assert(isCons() && "Accessing car of non-cons cell");
            data_->car = value.data_;
        }
        
        void setCdr(Value const & value) {
            assert(isCons() && "Accessing cdr of non-cons cell");
            data_->cdr = value.data_;
        }

        Value body() const {
            assert(isClosure() && "Accessing body of non-closure cell");
            return data_->body;
        }
        
        Value environment() const {
            assert(isClosure() && "Accessing environment of non-closure cell");
            return data_->environment;
        }

        void setBody(Value const & value) {
            assert(isClosure() && "Accessing body of non-closure cell");
            data_->body = value.data_;
        }
        
        void setEnvironment(Value const & value) {
            assert(isClosure() && "Accessing environment of non-closure cell");
            data_->environment = value.data_;
        }
        
        /** Compares two Values.

            Values are equal iff they point to the same cell.
        */
        bool operator == (Value other) const {
            return data_ == other.data_;
        }

        /** Inequality of Values.
         */
        bool operator != (Value other) const {
            return data_ != other.data_;
        }



    protected:

        friend class Symbol;

        Value(GC::Cell * data):
            data_(data) {
            assert(data != nullptr && "nullptr is not allowed to be used in GC objects, use Nil instead");
            GC::AddRoot(data_);
        }

        GC::Cell * data_;
    }; // tlisp::Value

    class Symbol {
    public:
        static Value const Empty;
        static Value const ParOpen;
        static Value const ParClose;
        static Value const BackQuote;
        static Value const Comma;
        static Value const Dot;
        static Value const Add;
        static Value const Sub;
        static Value const Mul;
        static Value const Div;
        static Value const Eq;
        static Value const Lt;
        static Value const Gt;
        static Value const Print;
        static Value const Read;
        static Value const If;
        static Value const Lambda;
        static Value const Quote;
        static Value const Apply;
        static Value const Cons;
        static Value const Car;
        static Value const Cdr;
        static Value const Consp;
        static Value const Defun;
        static Value const Progn;
        static Value const Let;
        static Value const Letrec;
        static Value const T;
        static Value const QuoteChar;

        static Value ForName(std::string const & name) {
            return Value(GetCellForName(name));
        }

    private:
        struct KeyHash {
            size_t operator() (std::string const * const & str) const noexcept {
                return std::hash<std::string>()(*str);
            }
        };

        struct KeyEquals {
            bool operator()(std::string const * const & k1, std::string const * const & k2) const noexcept {
                return *k1 == *k2;
            }
        };

        static GC::Cell * GetCellForName(std::string const & name);

        static std::unordered_map<std::string const *, GC::Cell *, KeyHash, KeyEquals> symbols_;
        
    }; // tlisp::Symbol

    extern Value const Nil;

    extern Value const T;

    inline Value::Value():
        data_(Nil.data_) {
        GC::AddRoot(data_);
    }

    /** Prints the given value to the specified stream.
     */
    std::ostream & operator << (std::ostream & s, Value const & v);
    
    
} // namespace secd

