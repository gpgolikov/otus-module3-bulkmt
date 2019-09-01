#include "logger.h"

#include <chrono>
#include <string>
#include <string_view>

#include "statement.h"

namespace griha {

namespace {

constexpr auto c_prefix = "bulk"sv;

struct LoggerExecuter : Executer {

    std::ostream& output;

    LoggerExecuter(std::ostream& os) : output(os) {}

    void execute(const SomeStatement& stm) override {
        output << stm.value() << std::endl;
    }
};

} // unnamed namespace

void Logger::log(const StatementContainer& stms);
    using namespace std;

    const auto now = chrono::system_clock::now();
    const auto now_ms = chrono::duration_cast<chrono::microseconds>(now.time_since_epoch());
    const auto filename = c_prefix + 
        to_string(now_ms.count()) + '-' + to_string(++counter_) + ".log"s;

    if (output_.is_open())
        output_.close();
    
    output_.open(filename);

    LoggerExecuter executer(output_);
    for (auto& stm : stms)
        stm->execute(executer);

    output_.flush();
    output_.close();
}

} // namespace griha