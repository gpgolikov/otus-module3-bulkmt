#pragma once

#include <iostream>

#include "forward.h"
#include "reader.h"

namespace griha {

class Interpreter {

public:
    constexpr Interpreter() = default;

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator= (const Interpreter&) = delete;

    void run(std::istream& input, size_t block_size, size_t nthreads);
};

} // namespace griha