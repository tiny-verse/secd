#include "value.h"

namespace secd {


    std::unordered_map<std::string const *, GC::Cell *, Symbol::KeyHash, Symbol::KeyEquals> Symbol::symbols_;

    Value const Symbol::Empty = Symbol::ForName("");
    Value const Symbol::ParOpen = Symbol::ForName("(");
    Value const Symbol::ParClose = Symbol::ForName(")");
    Value const Symbol::BackQuote = Symbol::ForName("`");
    Value const Symbol::Comma = Symbol::ForName(",");
    Value const Symbol::Dot = Symbol::ForName(".");
    Value const Symbol::Add = Symbol::ForName("+");
    Value const Symbol::Sub = Symbol::ForName("-");
    Value const Symbol::Mul = Symbol::ForName("*");
    Value const Symbol::Div = Symbol::ForName("/");
    Value const Symbol::Eq = Symbol::ForName("eq");
    Value const Symbol::Lt = Symbol::ForName("<");
    Value const Symbol::Gt = Symbol::ForName(">");
    Value const Symbol::Print = Symbol::ForName("print");
    Value const Symbol::Read = Symbol::ForName("read");
    Value const Symbol::If = Symbol::ForName("if");
    Value const Symbol::Lambda = Symbol::ForName("lambda");
    Value const Symbol::Quote = Symbol::ForName("quote");
    Value const Symbol::Apply = Symbol::ForName("apply");
    Value const Symbol::Cons = Symbol::ForName("cons");
    Value const Symbol::Car = Symbol::ForName("car");
    Value const Symbol::Cdr = Symbol::ForName("cdr");
    Value const Symbol::Consp = Symbol::ForName("consp");
    Value const Symbol::Defun = Symbol::ForName("defun");
    Value const Symbol::Progn = Symbol::ForName("progn");
    Value const Symbol::Let = Symbol::ForName("let");
    Value const Symbol::Letrec = Symbol::ForName("letrec");
    Value const Symbol::T = Symbol::ForName("t");
    Value const Symbol::QuoteChar = Symbol::ForName("'");

    GC::Cell * Symbol::GetCellForName(std::string const & name) {
        auto i = symbols_.find(&name);
        if (i == symbols_.end()) {
            std::string * str = new std::string(name);
            i = symbols_.insert(std::make_pair(str, new GC::Cell(str))).first;
        }
        return i->second;
    }

    Value const Nil = Symbol::ForName("nil");
    Value const T = Value::Integer(1);

    std::ostream & operator << (std::ostream & s, Value const & value) {
        switch (value.kind()) {
        case GC::CellKind::Integer:
            s << value.valueInt();
            break;
        case GC::CellKind::Symbol:
            s << value.name();
            break;
        case GC::CellKind::Cons: {
                s << "(";
                Value c(value);
                s << c.car();
                while (c.cdr().isCons()) {
                    s << " ";
                    c = c.cdr();
                    s << c.car();
                }
                if (c.cdr() != Nil) {
                    s << " . " << c.cdr();
                }
                s << ")";
                break;
            }
        case GC::CellKind::Closure: {
                // TODO 
                s << "Dunno yet how to print closures properly";
                break;
            }
        }
        return s;
    }
    
} // namespace secd
