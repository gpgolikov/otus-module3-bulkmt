#include <catch2/catch.hpp>

#include <memory>

#include <statement.h>
#include <statement_factory.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch;
using namespace Catch::Matchers;

struct TestExecuter : Executer {
    string last_statement;

    void execute(const SomeStatement& stm) override {
        last_statement = "SomeStatement { "s + stm.value() + " }"s;
    }
};

TEST_CASE("StatementFactory", "[statement]") {
    StatementFactory factory;

    auto statement = factory.create("");
    REQUIRE(dynamic_pointer_cast<SomeStatement>(statement));

    statement = factory.create("cmd1");
    REQUIRE(dynamic_pointer_cast<SomeStatement>(statement));
}

TEST_CASE("Statement execution", "[statement]") {
    StatementFactory factory;
    TestExecuter executer;

    auto statement = factory.create("cmd");
    statement->execute(executer);
    REQUIRE_THAT(
        executer.last_statement,
        Equals("SomeStatement { cmd }"));
}
