#include <iostream>

#include "runtime.h"
#include "common/colors.h"

namespace secd {

    void print(Value const & value) {
        std::cout << value << std::endl;
    }

    Value read() {
        std::cout << tiny::color::white << "Please enter an integer number: ";
        int i;
        std::cin >> i;
        std::cout << tiny::color::reset;
        return Value::Integer(i);
    }

}
