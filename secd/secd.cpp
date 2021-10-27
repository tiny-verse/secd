#include <iostream>

#include "runtime.h"
#include "secd.h"

namespace secd {

    namespace {

        void printCodeWithOffset(Value const & code, int offset) {
            // we take the code as stack and will be removing elements from it until empty
            Stack c(code);
            while (! c.empty()) {
                int64_t opcode = c.pop().valueInt();
                std::cout << std::string(offset, ' ');
                switch (opcode) {
                case Instruction::NIL:
                    std::cout << "NIL" << std::endl;
                    break;
                case Instruction::LDC:
                    std::cout << "LDC " << c.pop() << std::endl;
                    break;
                case Instruction::LD:
                    std::cout << "LD " << c.pop() << std::endl;
                    break;
                case Instruction::SEL: 
                    std::cout << "SEL" << std::endl;
                    printCodeWithOffset(c.pop(), offset + 4);
                    std::cout << "else" << std::endl;
                    printCodeWithOffset(c.pop(), offset + 4);
                    break;
                case Instruction::JOIN:
                    std::cout << "JOIN" << std::endl;
                    break;
                case Instruction::LDF:
                    std::cout << "LDF" << std::endl;
                    printCodeWithOffset(c.pop(), offset + 4);
                    break;
                case Instruction::AP:
                    std::cout << "AP" << std::endl;
                    break;
                case Instruction::RTN:
                    std::cout << "RTN" << std::endl;
                    break;
                case Instruction::DUM:
                    std::cout << "DUM" << std::endl;
                    break;
                case Instruction::RAP:
                    std::cout << "RAP" << std::endl;
                    break;
                case Instruction::DEFUN:
                    std::cout << "DEFUN " << std::endl;
                    break;
                case Instruction::POP:
                    std::cout << "POP" << std::endl;
                    break;
                case Instruction::CONS:
                    std::cout << "CONS" << std::endl;
                    break;
                case Instruction::CAR:
                    std::cout << "CAR" << std::endl;
                    break;
                case Instruction::CDR:
                    std::cout << "CDR" << std::endl;
                    break;
                case Instruction::CONSP:
                    std::cout << "CONSP" << std::endl;
                    break;
                case Instruction::ADD:
                    std::cout << "ADD" << std::endl;
                    break;
                case Instruction::SUB:
                    std::cout << "SUB" << std::endl;
                    break;
                case Instruction::MUL:
                    std::cout << "MUL" << std::endl;
                    break;
                case Instruction::DIV:
                    std::cout << "DIV" << std::endl;
                    break;
                case Instruction::EQ:
                    std::cout << "EQ" << std::endl;
                    break;
                case Instruction::LT:
                    std::cout << "LT" << std::endl;
                    break;
                case Instruction::GT:
                    std::cout << "GT" << std::endl;
                    break;
                case Instruction::PRINT:
                    std::cout << "PRINT" << std::endl;
                    break;
                case Instruction::READ:
                    std::cout << "READ" << std::endl;
                    break;
                default:
                    std::cout << "!!! Undefined opcode " << opcode;
                }
            }
        }
        
    } // anonymous namespace

    void printCode(Value const & code) {
        printCodeWithOffset(code, 0);
    }

    Value Compiler::compileSource(Value const & code) {
        assert(envMap_ != nullptr && envMap_->parent() == nullptr && "Valid global env assumed");
        assert(code_ == nullptr && "Leftover code object detected");
        try {
            // initialize the output code
            code_ = new Code(nullptr);
            compile(code);
            Value res = *code_;
            assert(code_->parent() == nullptr && "Global code object expected after successful compilation");
            delete code_;
            code_ = nullptr;
            return res;
            
        } catch (...) {
            // clear the code buffer and unroll environmentmaps if any
            delete code_;
            code_ = nullptr;
            while (envMap_->parent() != nullptr)
                unrollEnvironmentMap();
            throw;
        }
    }
    void Compiler::enterNewEnv(Value names) {
        envMap_ = new EnvironmentMap(envMap_);
        while (names != Nil) {
            Value arg = names.car();
            names = names.cdr();
            if (! arg.isSymbol())
                throw std::runtime_error(STR("Argument must be a symbol, but " << names << " found"));
            envMap_->addSymbol(arg);
        }
    }

    void Compiler::unrollEnvironmentMap() {
        EnvironmentMap * e = envMap_;
        envMap_ = e->parent();
        e->detach();
        delete e;
    }

    void Compiler::enterNewCode() {
        code_ = new Code(code_);
    }
    
    void Compiler::unrollAndAppendCode() {
        Code * c = code_;
        code_ = c->parent();
        code_->add(*c);
        c->detach();
        delete c;
    }

    /** Integer constant is compiled to the LDC instruction followed by the integer value itself.
     */
    void Compiler::compileInteger(Value const & code) {
        code_->add(Instruction::LDC);
        code_->add(code);
    }

    /** Nil is compiled to the NIL instruction.
     */
    void Compiler::compileNil() {
        code_->add(Instruction::NIL);    
    }
    
    void Compiler::compileTrue() {
        code_->add(Instruction::LDC);
        code_->add(T);
    }
    
    void Compiler::compileVariableRead(Value const & code) {
        code_->add(Instruction::LD);
        code_->add(envMap_->indexOf(code));
    }

    void Compiler::compileCall(Value const & code) {
        Value fname = code.car();
        Value args = code.cdr();
        if (fname.isSymbol()) {
            // handle special calls
            if (fname == Symbol::Cons) 
                compileBinaryOperator(Instruction::CONS, args);
            else if (fname == Symbol::Car)
                compileUnaryOperator(Instruction::CAR, args);
            else if (fname == Symbol::Cdr)
                compileUnaryOperator(Instruction::CDR, args);
            else if (fname == Symbol::Consp)
                compileUnaryOperator(Instruction::CONSP, args);
            else if (fname == Symbol::Add)
                compileBinaryOperator(Instruction::ADD, args);
            else if (fname == Symbol::Sub)
                compileBinaryOperator(Instruction::SUB, args);
            else if (fname == Symbol::Mul)
                compileBinaryOperator(Instruction::MUL, args);
            else if (fname == Symbol::Div)
                compileBinaryOperator(Instruction::DIV, args);
            else if (fname == Symbol::Eq)
                compileBinaryOperator(Instruction::EQ, args);
            else if (fname == Symbol::Lt)
                compileBinaryOperator(Instruction::LT, args);
            else if (fname == Symbol::Gt)
                compileBinaryOperator(Instruction::GT, args);
            else if (fname == Symbol::Print)
                compileUnaryOperator(Instruction::PRINT, args);
            else if (fname == Symbol::Read)
                compileRead(args);
            else if (fname == Symbol::If)
                compileIf(args);
            else if (fname == Symbol::Lambda)
                compileLambda(args);
            else if (fname == Symbol::Quote)
                compileQuote(args);
            else if (fname == Symbol::Apply)
                compileApply(args);
            else if (fname == Symbol::Defun)
                compileDefun(args);
            else if (fname == Symbol::Let)
                compileLet(args);
            else if (fname == Symbol::Letrec)
                compileLetrec(args);
            else if (fname == Symbol::Progn)
                compileProgn(args);
            else {
                compileFunctionArgs(args);
                compile(fname);
                code_->add(Instruction::AP);
            }
        } else {
            compileFunctionArgs(args);
            compile(fname);
            code_->add(Instruction::AP);
        }
    }
    
    void Compiler::compileUnaryOperator(int opcode, Value args) {
        compile(car(args));
        if (cdr(args) != Nil)
            throw std::runtime_error("Too many arguments to binary operator");
        code_->add(opcode);
    }

    void Compiler::compileBinaryOperator(int opcode, Value args) {
        Value lhs(Nil);
        Value rhs(Nil);
        List::Expand(args, lhs, rhs);
        compile(rhs);
        compile(lhs);
        code_->add(opcode);
    }

    void Compiler::compileRead(Value const & args) {
        if (args != Nil)
            throw std::runtime_error("Read does not take any arguments");
        code_->add(Instruction::READ);
    }

    void Compiler::compileIf(Value args) {
        compile(car(args));
        args = cdr(args);
        code_->add(Instruction::SEL);
        enterNewCode();
        compile(car(args));
        args = cdr(args);
        code_->add(Instruction::JOIN);
        unrollAndAppendCode();
        enterNewCode();
        compile(car(args));
        code_->add(Instruction::JOIN);
        unrollAndAppendCode();
        if (cdr(args) != Nil)
            throw std::runtime_error("Too many arguments to if");
    }

    void Compiler::compileLambda(Value args) {
        Value argNames;
        Value body;
        List::Expand(args, argNames, body);
        compileLambda(argNames, body);
    }

    void Compiler::compileLambda(Value argNames, Value body) {
        code_->add(Instruction::LDF);
        enterNewCode();
        // create new environment map for the function
        enterNewEnv(argNames);
        // compile the function
        compile(body);
        code_->add(Instruction::RTN);
        // restore the output code list and append the callee's code
        unrollAndAppendCode();
        // delete the callee's environment map
        unrollEnvironmentMap();
    }

    /** Quote simply loads its argument as value, i.e. compiles to LDC.
     */
    void Compiler::compileQuote(Value args) {
        if (args == Nil)
            throw std::runtime_error("Not enough arguments to quote");
        code_->add(Instruction::LDC);
        code_->add(car(args));
        if (cdr(args) != Nil)
            throw std::runtime_error("Too many arguments to quote");
    }
    
    void Compiler::compileApply(Value args) {
        // TODO make nice error messages and check that args is valid length
        Value func = car(args);
        args = cdr(args);
        if (cdr(args) != Nil)
            throw std::runtime_error("Too many arguments to apply");
        std::cout << args << std::endl;
        // compile the actual call
        compileFunctionArgs(args);
        // compile the function
        compile(func);
        // arguments compiled, emit the AP instruction
        code_->add(Instruction::AP);
    }
    
    void Compiler::compileLet(Value args) {
        Value argNames;
        Value values;
        Value body;
        List::Expand(args, argNames, values, body);
        compileFunctionArgs(values);
        compileLambda(argNames, body);
        code_->add(Instruction::AP);
    }
    
    void Compiler::compileLetrec(Value args) {
        code_->add(Instruction::DUM);
        Value argNames;
        Value values;
        Value body;
        List::Expand(args, argNames, values, body);
        enterNewEnv(argNames);
        compileFunctionArgs(values);
        compileLambda(argNames, body);
        // arguments compiled, emit the AP instruction
        code_->add(Instruction::RAP);
        unrollEnvironmentMap();
    }

    void Compiler::compileProgn(Value args) {
        if (args == Nil) {
            code_->add(Instruction::NIL);
        } else {
            while (true) {
                compile(args.car());
                args = cdr(args);
                if (args == Nil)
                    break;
                code_->add(Instruction::POP);
            }
        }
    }
    
    /** Defun has its own bytecode.
     */
    void Compiler::compileDefun(Value args) {
        if (code_->parent() != nullptr)
            throw std::runtime_error("defun can only appear at global scope");
        Value fname = car(args);
        if (! fname.isSymbol())
            throw std::runtime_error(STR("Name of the function expected, but " << args << " found"));
        envMap_->addSymbol(fname);
        args = cdr(args);
        compileLambda(args);
        code_->add(Instruction::DEFUN);
    }
    
    void Compiler::compileCallArguments(Value args) {
        if (args == Nil)
            return;
        compileCallArguments(cdr(args));
        compile(car(args));
        code_->add(Instruction::CONS);
    }
    
    void Compiler::compileFunctionArgs(Value const & args) {
        // now compile the arguments
        code_->add(Instruction::NIL);
        compileCallArguments(args);
    }

    void Compiler::compile(Value const & code) {
        switch (code.kind()) {
        case GC::CellKind::Integer:
            compileInteger(code);
            break;
        case GC::CellKind::Symbol:
            if (code == Nil)
                compileNil();
            else if (code == Symbol::T)
                compileTrue();
            else
                compileVariableRead(code);
            break;
        case GC::CellKind::Cons:
            compileCall(code);
            break;
        default:
            assert(false && "Not aware of any other values to compile");
        }
    }

#ifdef HAHA
    
    Value Runtime::run(Value const & code) {
        try {
            assert(c_.empty() && "Control register should be empty before executing new code");
            c_ = code;
            Value lhs(Nil);
            Value rhs(Nil);
            Value fun(Nil);
            while (! c_.empty()) {
                int64_t opcode = c_.pop().valueInt();
                switch (opcode) {
                    /* Simply pushes Nil on the stack.
                        */
                case Instruction::NIL:
                    s_.push(Nil);
                    break;
                    /* Pushes the argument of the instruction directly on the stack.
                        */
                case Instruction::LDC:
                    s_.push(c_.pop());
                    break;
                case Instruction::LD:
                    s_.push(e_.locate(c_.pop()));
                    break;
                    /* Pops the value in s_ and based on its value selects either its first or second argument. The rest of the c register is pushed on the dump.
                        */
                case Instruction::SEL: {
                    Value cond = s_.pop();
                    Value trueCase = c_.pop();
                    Value falseCase = c_.pop();
                    d_.push(c_);
                    if (toBoolean(cond))
                        c_ = trueCase;
                    else
                        c_ = falseCase;
                    break;
                }
                    /* Pops the backup of the control register from the dump and restores its value.
                        */
                case Instruction::JOIN:
                    c_ = d_.pop();
                    break;
                    /* Packages the current environment and the code for a function (as argument on the control stack) and pushes it on the S stack.
                        */
                case Instruction::LDF: {
                    Value body = c_.pop();
                    s_.push(Value::Closure(body, e_));
                    break;
                }
                    /* The S register contains a closure and a list of arguments given to it.
                        */
                case Instruction::AP: {
                    // get the argument list and the closure from the S register
                    Value closure = s_.pop();
                    if (! closure.isClosure())
                        throw std::runtime_error(STR("AP expects closure on stack, but " << closure << " found"));
                    Value argList = s_.pop();
                    // backup the values of S, E and C registers to the dump
                    d_.push(List({s_, e_, c_}));
                    s_ = Nil;
                    e_ = Value::Cons(argList, closure.environment());
                    c_ = closure.body();
                    break;
                }
                    /* Gets the result value from the S register, then recovers the S E and C registers of the caller function from the D register and pushes the result on the caller's S register.
                        */
                case Instruction::RTN: {
                    // get the result
                    Value result = s_.pop();
                    // pop the caller's context from the D register, expand and restore the S E C registers
                    Value callerContext(d_.pop());
                    s_ = callerContext.car();
                    callerContext = callerContext.cdr();
                    e_ = callerContext.car();
                    callerContext = callerContext.cdr();
                    c_ = callerContext.car();
                    // push the result to the caller's stack now
                    s_.push(result);
                    break;
                }
                    /* Inserts a dummy environment in the environment chain.
                        */
                case Instruction::DUM: {
                    e_.insertDummyEnvironment();
                    break;
                }
                    /* Recursive apply is perhaps the hardest, and is the only instruction that actually has to do in-place change. 
                        */
                case Instruction::RAP: {
                    // pop the dummy environment from the current env list since it is no longer needed
                    e_.popDummyEnvironment();
                    // get the argument list and the closure from the S register
                    Value closure = s_.pop();
                    Value argList = s_.pop();
                    // get the closure environment and patch its car, which is the dummy Nil env from DUM
                    Value closureEnv = closure.environment();
                    assert(closureEnv.car() == Nil && "Expected dummy env from DUM");
                    closureEnv.setCar(argList);
                    // backup the values of S, E and C registers to the dump
                    d_.push(List({s_, e_, c_}));
                    // prepare stuff for the application
                    s_ = Nil;
                    e_ = closureEnv;
                    c_ = closure.body();
                    break;
                }
                    /* Defines a function in the global environment. This is not part of the original SECD machine, but has been added so that we can use the interpreter in a REPL mode.
                        */
                case Instruction::DEFUN: {
                    fun = s_.pop();
                    e_.append(fun);
                    s_.push(Nil);
                    break;
                }
                /* Pops from the s_ stack. This enables sequence programming.
                    */
                case Instruction::POP: {
                    s_.pop();
                    break;
                }
                    /* Pops two values from S register, creates a cons cell from them and pushes it back on the S stack.
                        */
                case Instruction::CONS: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Cons(lhs, rhs));
                    break;
                }
                    /* Pops value from the S register and pushes on it its car value.
                        */
                case Instruction::CAR:
                    s_.push(car(s_.pop()));
                    break;
                    /* Pops value from the S register and pushes on it its cdr value.
                        */
                case Instruction::CDR:
                    s_.push(cdr(s_.pop()));
                    break;
                case Instruction::CONSP:
                    if (cdr(s_.pop()).isCons())
                        s_.push(T);
                    else
                        s_.push(Nil);
                    break;
                case Instruction::ADD: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() + rhs.valueInt()));
                    break;
                }
                case Instruction::SUB: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() - rhs.valueInt()));
                    break;
                }
                case Instruction::MUL: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() * rhs.valueInt()));
                    break;
                }
                case Instruction::DIV: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() / rhs.valueInt()));
                    break;
                }
                case Instruction::EQ: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    int64_t result = 0;
                    if (lhs.isInteger() && rhs.isInteger())
                        result = lhs.valueInt() == rhs.valueInt();
                    else
                        result = lhs == rhs;
                    s_.push(Value::Integer(result));
                    break;
                }
                case Instruction::LT: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() < rhs.valueInt()));
                    break;
                }
                case Instruction::GT: {
                    lhs = s_.pop();
                    rhs = s_.pop();
                    s_.push(Value::Integer(lhs.valueInt() > rhs.valueInt()));
                    break;
                }
                case Instruction::PRINT: {
                    lhs = s_.top();
                    print(lhs);
                    break;
                }
                case Instruction::READ: {
                    s_.push(read());
                    break;
                }
                default:
                    assert(false && "Unexpected opcode");
                }
            }
            assert(! s_.empty() && "Malformed program");
            Value result = s_.pop();
            assert(s_.empty() && "Malformed program");
            return result;
        } catch (...) {
            // if there is an error, make sure the leave th SECD machine in proper-ish state
            c_ = Nil;
            s_ = Nil;
            d_ = Nil;
            while (cdr(e_) != Nil)
                e_ = cdr(e_);
            throw;
        }
    }
#endif
}
