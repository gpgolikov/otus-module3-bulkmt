#include "reader.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "reader_subscriber.h"
#include "statement_factory.h"

namespace griha {

namespace {

using namespace std::string_view_literals;

constexpr auto c_explicit_block_begin   = "{"sv;
constexpr auto c_explicit_block_end     = "}"sv;

bool is_block_begin(std::string_view line) {
    using namespace std;
    return line == c_explicit_block_begin;
}

bool is_block_end(std::string_view line) {
    using namespace std;
    return line == c_explicit_block_end;
}

} // unnamed namespace

struct ReaderState : std::enable_shared_from_this<ReaderState> {
    virtual ~ReaderState() {}
    virtual bool process(std::istream& line) = 0;
};
using ReaderStatePtr = std::shared_ptr<ReaderState>;

struct ReaderImpl {

    inline ReaderImpl(size_t bsize)
        : state(nullptr)
        , block_size(bsize) {}

    ReaderStatePtr state;
    const size_t block_size;

    std::vector<ReaderSubscriberPtr> subscribers;

    StatementFactory statement_factory;
    StatementContainer statements;

    Reader::Metrics metrics;

    template <typename State> State& change_state(); 

    bool process(std::istream& is);
    void process(std::string line);

    bool read_line(std::istream& is, std::string& line);

    void notify_block();
    void notify_unexpected_eof();
};

struct InitialState : ReaderState {
    inline explicit InitialState(ReaderImpl& r_impl)
        : reader_impl(r_impl) {}

    bool process(std::istream& line) override;

    ReaderImpl& reader_impl;
    size_t count {};
};

struct BlockState : ReaderState {
    inline BlockState(ReaderImpl& r_impl) 
        : reader_impl(r_impl) {}

    bool process(std::istream& line) override;

    ReaderImpl& reader_impl;
    size_t level { 1 };
};

struct ErrorState : ReaderState {
    inline ErrorState(ReaderImpl& r_impl)
        : reader_impl(r_impl) {}

    bool process(std::istream& line) override;

    ReaderImpl& reader_impl;
    std::string error;
};

template <typename State>
State& ReaderImpl::change_state() {
    // for further optimization it looks pretty to create states pool
    state = std::make_shared<State>(*this);
    return dynamic_cast<State&>(*state);
} 

bool ReaderImpl::process(std::istream& is) {
    auto save_state_ptr = state->shared_from_this(); // protect against unexpected deletion
    return state->process(is);
}

void ReaderImpl::process(std::string line) {
    ++metrics.nstatements;
    statements.push_back(statement_factory.create(std::move(line)));
}

bool ReaderImpl::read_line(std::istream& is, std::string& line) {
    if (!getline(is, line))
        return false;
    
    ++metrics.nlines;
    return true;
}

void ReaderImpl::notify_block() {
    if (statements.empty())
        return; // empty block doesn't require notification

    ++metrics.nblocks;

    for (auto& subscriber : subscribers)
        subscriber->on_block(statements);
    statements.clear();
}

void ReaderImpl::notify_unexpected_eof() {
    if (statements.empty())
        return; // empty block doesn't require notification

    for (auto& subscriber : subscribers)
        subscriber->on_unexpected_eof(statements);
}

bool InitialState::process(std::istream& input) {
    using namespace std;

    string line;
    if (!reader_impl.read_line(input, line)) {
        // in initial state the end of the stream triggers end of block
        reader_impl.notify_block();
        return false; // end of file or another error
    }
    
    if (is_block_end(line)) {
        reader_impl.change_state<ErrorState>().error = "unexpected end of block"s;
    } else if (is_block_begin(line)) {
        // in initial state start of explicit block triggers end of block
        reader_impl.notify_block();
        reader_impl.change_state<BlockState>();
    } else {
        reader_impl.process(std::move(line));
        if (++count == reader_impl.block_size) {
            // fixed block size has been reached
            reader_impl.notify_block();
            count = 0;
        }
    }

    return true;
}

bool BlockState::process(std::istream& input) {
    using namespace std;

    string line;
    if (!reader_impl.read_line(input, line)) {
        reader_impl.notify_unexpected_eof();
        return false; // end of file or another error
    }

    if (is_block_begin(line)) {
        // nested explicit blocks are ignored but correction of syntax is required
        ++level;
    } else if (is_block_end(line)) {
        if (--level == 0) {
            // explicit block has been ended
            // block has statements - notify about end of block
            reader_impl.notify_block();
            reader_impl.change_state<InitialState>();
        }
    } else {
        reader_impl.process(std::move(line));
    }

    return true;
}

bool ErrorState::process([[maybe_unused]] std::istream& is) {
    std::cerr << error << std::endl;
    return false;
}

Reader::Reader(size_t block_size) 
    : priv_(std::make_unique<ReaderImpl>(block_size)) {}

Reader::~Reader() = default;
Reader::Reader(Reader&&) = default;
Reader& Reader::operator= (Reader&&) = default;

void Reader::subscribe(ReaderSubscriberPtr subscriber) {
    auto& subscribers = priv_->subscribers;
    auto it = std::find(subscribers.begin(), subscribers.end(), subscriber);
    if (it == subscribers.end())
        subscribers.push_back(std::move(subscriber));
}

auto Reader::run(std::istream& input) -> const Metrics& {
    priv_->metrics = {};
    priv_->statements.clear();
    priv_->change_state<InitialState>();
    while (priv_->process(input)) {
        // do nothing
    }

    return priv_->metrics;
}

} // namespace griha