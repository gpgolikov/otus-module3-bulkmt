#pragma once

#include <fstream>

#include "forward.h"

namespace griha {

class Logger {

public:
    void log(const StatementContainer& stms);

private:
    std::ofstream output_;
    std::uint16_t counter_ {};
};

} // namespace griha