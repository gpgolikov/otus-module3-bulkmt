#pragma once

#include <iostream>
#include <memory>

#include "forward.h"
#include "reader.h"
#include "logger.h"

namespace griha {

class Interpreter {

    struct Impl;

public:
    struct Context {
        Reader reader;
        Logger logger;
        std::ostream& output;
    };

public:
    explicit Interpreter(Context context);

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator= (const Interpreter&) = delete;

    void run(std::istream& input);

private:
    std::shared_ptr<Impl> priv_;
};

} // namespace griha