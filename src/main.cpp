#include <iostream>
#include <string>

#include "interpreter.h"

using namespace std;
using namespace griha;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: bulk <block_size> [<nthreads>]" << endl;
        return -1;
    }

    Interpreter interpreter;
    interpreter.run(
        cin,
        stoul(argv[1]), // size of block
        argc == 2 ? 2u : stoul(argv[2]) // number of threads
    );
    return 0;
}