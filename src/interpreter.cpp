#include "interpreter.h"

#include <string>

#include "reader.h"
#include "reader_subscriber.h"
#include "statement.h"
#include "logger.h"

namespace griha {

struct Interpreter::Impl : std::enable_shared_from_this<Impl>
                         , ReaderSubscriber
                         , Executer {

    Reader reader;
    Logger logger;
    std::ostream& output;

    explicit Impl(Interpreter::Context context)
        : reader(std::move(context.reader))
        , logger(std::move(context.logger))
        , output(context.output) {}

    void on_block(const StatementContainer& stms) override {
        logger.log(stms);
        
        output << "bulk: ";
        bool f = true;
        for (auto& stm : statements) {
            if (!f) {
                output << ", ";
            }
            stm->execute(*this);
            f = false;
        }
        output << std::endl;
        statements.clear();
    }

    void execute(const SomeStatement& stm) override {
        output << stm.value();
    }
};

Interpreter::Interpreter(Context context)
    : priv_(std::make_shared<Impl>(std::move(context))) {
    priv_->reader.subscribe(priv_->shared_from_this());
}

void Interpreter::run(std::istream& input) {
    priv_->reader.run(input);
}

} // namespace griha