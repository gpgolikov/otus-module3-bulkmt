#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>
#include <memory>

#include <statement.h>
#include <reader.h>
#include <reader_subscriber.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch;
using namespace Catch::Matchers;

struct ReaderMonitor : ReaderSubscriber {

    std::vector<std::pair<StatementPtr, StatementContainer>> blocks;

    void clear() {
        blocks.clear();
    }

    void on_block_begin() override {
        blocks.emplace_back(nullptr, StatementContainer{});
    }

    void on_block_end() override {
        REQUIRE_FALSE(blocks.empty());
        REQUIRE_FALSE(blocks.back().second.empty());
        blocks.back().first = blocks.back().second.front(); 
    }

    void on_new_statement(StatementPtr stm) override {
        REQUIRE_FALSE(blocks.empty());
        blocks.back().second.push_back(std::move(stm));
    }
};

TEST_CASE("Reader", "[reader]") {
    
    Reader reader(3);
    auto monitor = make_shared<ReaderMonitor>();
    reader.subscribe(monitor);

    SECTION("Fixed blocks") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(3));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd3"));
        // check second block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks.back().second;
        REQUIRE_THAT(block2.size(), Equals(3));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
    }

    SECTION("Fixed blocks - EOF before block has been ended") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "cmd4\n"
            "cmd5"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(3));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd3"));
        // check second block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks.back().second;
        REQUIRE_THAT(block2.size(), Equals(2));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
    }

    SECTION("Explicit blocks") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "{\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"
            "cmd7\n"
            "}"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(3));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd3"));
        // check second block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks.back().second;
        REQUIRE_THAT(block2.size(), Equals(4));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[3]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd7"));
    }

    SECTION("Explicit blocks - fixed block after explicit") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "{\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"
            "cmd7\n"
            "}\n"
            "cmd8\n"
            "cmd9\n"
            "cmd10"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(3));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(3));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd3"));
        // check second block
        REQUIRE(monitor->blocks[1].first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks[1].first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks[1].second;
        REQUIRE_THAT(block2.size(), Equals(4));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[3]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd7"));
        // check third block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd8"));
        auto& block3 = monitor->blocks.back().second;
        REQUIRE_THAT(block3.size(), Equals(3));
        REQUIRE(block3[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block3[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd8"));
        statement = dynamic_pointer_cast<SomeStatement>(block3[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd9"));
        statement = dynamic_pointer_cast<SomeStatement>(block3[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd10"));
    }

    SECTION("Explicit blocks - explicit block before block has been ended") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "{\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"
            "cmd7\n"
            "}"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(2));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        // check second block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks.back().second;
        REQUIRE_THAT(block2.size(), Equals(4));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[3]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd7"));
    }

    SECTION("Explicit blocks - nested blocks") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "{\n"
            "cmd4\n"
            "cmd5\n"
            "{\n"
            "cmd6\n"
            "cmd7\n"
            "cmd8\n"
            "}\n"
            "cmd9\n"
            "}"s);

        reader.run(is);

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(2));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        // check second block
        REQUIRE(monitor->blocks.back().first);
        first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.back().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd4"));
        auto& block2 = monitor->blocks.back().second;
        REQUIRE_THAT(block2.size(), Equals(6));
        REQUIRE(block2[0] == first_statement);
        statement = dynamic_pointer_cast<SomeStatement>(block2[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[3]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd7"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[4]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd8"));
        statement = dynamic_pointer_cast<SomeStatement>(block2[5]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd9"));
    }

    SECTION("Explicit blocks - EOF before explicit block has been ended") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "{\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"
            "cmd7"s);

        reader.run(is);

        REQUIRE_FALSE(monitor->blocks.empty());
        REQUIRE(monitor->blocks.back().first == nullptr);
        REQUIRE_THAT(monitor->blocks.back().second.size(), Equals(4));
        // cleanup blocks
        monitor->blocks.pop_back();

        REQUIRE_THAT(monitor->blocks.size(), Equals(1));
        // check first block
        REQUIRE(monitor->blocks.front().first);
        auto first_statement = dynamic_pointer_cast<SomeStatement>(monitor->blocks.front().first);
        REQUIRE(first_statement);
        REQUIRE_THAT(first_statement->value(), Equals("cmd1"));
        auto& block1 = monitor->blocks.front().second;
        REQUIRE_THAT(block1.size(), Equals(2));
        REQUIRE(block1[0] == first_statement);
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
    }
}