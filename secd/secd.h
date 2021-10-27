#pragma once

#include "value.h"
#include "runtime.h"
#include "data_types.h"

/** SECD Virtual Machine Compiler & Interpreter

 */
namespace secd {

    void printCode(Value const & code);

    class Instruction {
    public:
        static int constexpr NIL = 0;
        static int constexpr LDC = 1;
        static int constexpr LD = 2;
        static int constexpr SEL = 3;
        static int constexpr JOIN = 4;
        static int constexpr LDF = 5;
        static int constexpr AP = 6;
        static int constexpr RTN = 7;
        static int constexpr DUM = 8;
        static int constexpr RAP = 9;
        static int constexpr DEFUN = 10;
        static int constexpr POP = 11;

        static int constexpr CONS = 90;
        static int constexpr CAR = 91;
        static int constexpr CDR = 92;
        static int constexpr CONSP = 94;
        
        static int constexpr ADD = 100;
        static int constexpr SUB = 101;
        static int constexpr MUL = 102;
        static int constexpr DIV = 103;
        static int constexpr EQ = 104;
        static int constexpr LT = 105;
        static int constexpr GT = 106;

        static int constexpr PRINT = 110;
        static int constexpr READ = 111;
        
    };

    /** Compiles the s-expressions into the SECD bytecode.
     */
    class Compiler {
    public:
        Compiler():
            code_(nullptr),
            envMap_(new EnvironmentMap(nullptr)) {
        }

        ~Compiler() {
            delete envMap_;
        }

        Value compileSource(Value const & source);

    private:

        /** Models the environment during the compilation so that local variables can be found.

            */
        class EnvironmentMap {
        public:

            EnvironmentMap(EnvironmentMap * parent):
                parent_(parent) {
            }

            ~EnvironmentMap() {
                delete parent_;
            }
            
            /** Adds new symbol to the environment map.
             */
            void addSymbol(Value const & name) {
                envMap_.insert(std::make_pair(name.name(), envMap_.size()));
            }

            /** Returns the index of the given symbol in the current compilation environment hierarchy.
             */
            Value indexOf(Value const & symbol) {
                assert(symbol.isSymbol() && "Expecting variable name");
                std::string const & name = symbol.name();
                EnvironmentMap * e = this;
                int depth = 0;
                while (e != nullptr) {
                    auto i = e->envMap_.find(name);
                    if (i != e->envMap_.end()) // found
                        return Value::Cons(Value::Integer(depth), Value::Integer(i->second));
                    ++depth;
                    e = e->parent_;
                }
                throw std::runtime_error(STR("Unknown variable " << symbol));
            }

            EnvironmentMap * parent() const {
                return parent_;
            }

            void detach() {
                assert(parent_ != nullptr && "Global env should not be detached");
                parent_ = nullptr;
            }

        private:
            /** Parent environment map.
             */
            EnvironmentMap * parent_;

            /** Current environment map, which goes from symbols to integers denoting their offset in the current environment.
             */
            std::unordered_map<std::string, int64_t> envMap_;
        };

        /** The code translated.
         */
        class Code {
        public:
            Code(Code * parent):
                parent_(parent) {
            }

            ~Code() {
                delete parent_;
            }

            Code * parent() const {
                return parent_;
            }

            operator Value () {
                return code_;
            }

            void detach() {
                assert(parent_ != nullptr && "Cannot detach global code");
                parent_ = nullptr;
            }

            void add(int opcode) {
                code_.append(Value::Integer(opcode));
            }

            void add(Value const & value) {
                code_.append(value);
            }

        private:
            Code * parent_;
            List code_;
        }; // Compiler::Code

        void enterNewEnv(Value names);
        void unrollEnvironmentMap();

        void enterNewCode();
        void unrollAndAppendCode();

        void compileInteger(Value const & code);
        void compileNil();
        void compileTrue();
        void compileVariableRead(Value const & code);
        void compileCall(Value const & code);
        void compileUnaryOperator(int opcode, Value args);
        void compileBinaryOperator(int opcode, Value args);
        void compileRead(Value const & args);
        void compileIf(Value args);
        void compileLambda(Value args);
        void compileLambda(Value argNames, Value body);
        void compileQuote(Value args);
        void compileApply(Value args);
        void compileLet(Value args);
        void compileLetrec(Value args);
        void compileProgn(Value args);
        void compileDefun(Value args);
        void compileCallArguments(Value call);
        //            void compileFunctionCall(Value const & func, Value const & args);
        void compileFunctionArgs(Value const & args);
        void compile(Value const & code);
        
        
        Code * code_;
        EnvironmentMap * envMap_;
    };

    /** Implements the environment and environment chain as required for the SECD machine implementation.

        */
    class Environment {
    public:
        /** Creates an empty environment.

            An empty environment is environment whose parent is nil, and which itself is an empty list.
        */
        Environment():
            v_(Value::Cons(Nil, Nil)) {
        }

        Environment(Value const & value):
            v_(value) {
            assert(v_.isCons() && "Environment must be at least an empty environment");
        }

        Value locate(Value const & index) {
            int64_t depth = index.car().valueInt();
            int64_t offset = index.cdr().valueInt();
            Value x = v_;
            while (depth-- > 0)
                x = x.cdr();
            x = x.car();
            while (offset-- > 0)
                x = x.cdr();
            return x.car();
        }

        void append(Value const & value) {
            Value e = v_.car();
            if (e == Nil) {
                v_.setCar(Value::Cons(value, Nil));
            } else {
                while (e.cdr() != Nil)
                    e = e.cdr();
                e.setCdr(Value::Cons(value, Nil));
            }
        }

        void insertDummyEnvironment() {
            v_ = Value::Cons(Nil, v_);
        }

        void popDummyEnvironment() {
            assert(v_.car() == Nil && "Dummy environment expected");
            v_ = v_.cdr();
        }

        operator Value & () {
            return v_;
        }


    private:

        Value v_;
    }; // secd::Environment

#ifdef HAHA    

    class Runtime : public ::tlisp::Runtime {
    public:
        
        Value compile(Value const & source) override {
            return compiler_.compileSource(source);
        }

        Value run(Value const & source) override;

        

    private:
        Compiler compiler_;

        /** The stack register.

            Holds the arguments to operations and function calls. Similar in function to the operand stack in stack-based ISAs.  
            */
        Stack s_;

        /** The environment register.
         */
        Environment e_;

        /** The control register.

            Holds the program to be executed. car(c) is the next instruction to be executed. Functionally similar to the program counter. 
            */
        Stack c_;

        /** The dump register.

            Stores the backups of the other three registers for non-linear control flow operations. Functionally similar to call stack. 
            */
        Stack d_;
    };

#endif
    
} // namespace secd
