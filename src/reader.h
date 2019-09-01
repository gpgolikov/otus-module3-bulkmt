#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "forward.h"

namespace griha {

class Reader {
public:
    Reader(size_t block_size);
    ~Reader();

    Reader(Reader&&);
    Reader& operator= (Reader&&);

    // delete copy operations to simplify Reader class
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;

    void subscribe(ReaderSubscriberPtr subscriber);

    void run(std::istream& input);

private:
    std::unique_ptr<struct ReaderImpl> priv_;
};

} // namespace griha