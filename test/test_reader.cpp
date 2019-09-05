#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

#include <statement.h>
#include <reader.h>
#include <reader_subscriber.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch;
using namespace Catch::Matchers;

struct ReaderMonitor : ReaderSubscriber {

    std::vector<StatementContainer> blocks;
    StatementContainer broken_block;

    void clear() {
        blocks.clear();
        broken_block.clear();
    }

    void on_block(const StatementContainer& stms) override {
        blocks.push_back(stms);
    }

    void on_unexpected_eof(const StatementContainer& stms) override {
        broken_block = stms;
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(6));
        REQUIRE_THAT(metrics.nstatements, Equals(6));
        REQUIRE_THAT(metrics.nblocks, Equals(2));

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(3));
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
        auto& block2 = monitor->blocks.back();
        REQUIRE_THAT(block2.size(), Equals(3));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(5));
        REQUIRE_THAT(metrics.nstatements, Equals(5));
        REQUIRE_THAT(metrics.nblocks, Equals(2));

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(3));
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
        auto& block2 = monitor->blocks.back();
        REQUIRE_THAT(block2.size(), Equals(2));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(9));
        REQUIRE_THAT(metrics.nstatements, Equals(7));
        REQUIRE_THAT(metrics.nblocks, Equals(2));

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(3));
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
        auto& block2 = monitor->blocks.back();
        REQUIRE_THAT(block2.size(), Equals(4));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(12));
        REQUIRE_THAT(metrics.nstatements, Equals(10));
        REQUIRE_THAT(metrics.nblocks, Equals(3));

        REQUIRE_THAT(monitor->blocks.size(), Equals(3));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(3));
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
        auto& block2 = monitor->blocks[1];
        REQUIRE_THAT(block2.size(), Equals(4));
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
        auto& block3 = monitor->blocks.back();
        REQUIRE_THAT(block3.size(), Equals(3));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(8));
        REQUIRE_THAT(metrics.nstatements, Equals(6));
        REQUIRE_THAT(metrics.nblocks, Equals(2));

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(2));
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        // check second block
        auto& block2 = monitor->blocks.back();
        REQUIRE_THAT(block2.size(), Equals(4));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(12));
        REQUIRE_THAT(metrics.nstatements, Equals(8));
        REQUIRE_THAT(metrics.nblocks, Equals(2));

        REQUIRE_THAT(monitor->blocks.size(), Equals(2));
        REQUIRE(monitor->broken_block.empty());
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(2));
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));
        // check second block
        auto& block2 = monitor->blocks.back();
        REQUIRE_THAT(block2.size(), Equals(6));
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

        auto metrics = reader.run(is);
        REQUIRE_THAT(metrics.nlines, Equals(7));
        REQUIRE_THAT(metrics.nstatements, Equals(6));
        REQUIRE_THAT(metrics.nblocks, Equals(1));

        REQUIRE_FALSE(monitor->blocks.empty());
        REQUIRE_FALSE(monitor->broken_block.empty());

        REQUIRE_THAT(monitor->blocks.size(), Equals(1));
        // check first block
        auto& block1 = monitor->blocks.front();
        REQUIRE_THAT(block1.size(), Equals(2));
        auto statement = dynamic_pointer_cast<SomeStatement>(block1[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd1"));
        statement = dynamic_pointer_cast<SomeStatement>(block1[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd2"));

        // check broken block
        REQUIRE_THAT(monitor->broken_block.size(), Equals(4));
        statement = dynamic_pointer_cast<SomeStatement>(monitor->broken_block[0]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd4"));
        statement = dynamic_pointer_cast<SomeStatement>(monitor->broken_block[1]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd5"));
        statement = dynamic_pointer_cast<SomeStatement>(monitor->broken_block[2]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd6"));
        statement = dynamic_pointer_cast<SomeStatement>(monitor->broken_block[3]);
        REQUIRE(statement);
        REQUIRE_THAT(statement->value(), Equals("cmd7"));
    }
}