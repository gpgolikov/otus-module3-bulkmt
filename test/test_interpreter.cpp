
#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>
#include <memory>

#include <interpreter.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch;
using namespace Catch::Matchers;

TEST_CASE("Interpreter", "[interpreter]") {
    ostringstream output;
    Interpreter::Context context = {
        Reader{ 3 },
        Logger{},
        output};
    Interpreter interpreter(std::move(context));

    SECTION("Fixed blocks") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"s);

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2, cmd3\n"
            "bulk: cmd4, cmd5, cmd6\n"
        ));
    }

    SECTION("Fixed blocks - EOF before block has been ended") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "cmd4\n"
            "cmd5"s);

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2, cmd3\n"
            "bulk: cmd4, cmd5\n"
        ));
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

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2, cmd3\n"
            "bulk: cmd4, cmd5, cmd6, cmd7\n"
        ));
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

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2, cmd3\n"
            "bulk: cmd4, cmd5, cmd6, cmd7\n"
            "bulk: cmd8, cmd9, cmd10\n"
        ));
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

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2\n"
            "bulk: cmd4, cmd5, cmd6, cmd7\n"
        ));
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

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2\n"
            "bulk: cmd4, cmd5, cmd6, cmd7, cmd8, cmd9\n"
        ));
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

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1, cmd2\n"
        ));
    }
}

TEST_CASE("Interpreter_block_size_1", "[interpreter]") {
    ostringstream output;
    Interpreter::Context context = {
        Reader{ 1 },
        Logger{},
        output};
    
    Interpreter interpreter(std::move(context));

    SECTION("Fixed blocks") {
        istringstream is;
        is.str(
            "cmd1\n"
            "cmd2\n"
            "cmd3\n"
            "cmd4\n"
            "cmd5\n"
            "cmd6\n"s);

        interpreter.run(is);

        REQUIRE_THAT(output.str(), Equals(
            "bulk: cmd1\n"
            "bulk: cmd2\n"
            "bulk: cmd3\n"
            "bulk: cmd4\n"
            "bulk: cmd5\n"
            "bulk: cmd6\n"
        ));
    }
}