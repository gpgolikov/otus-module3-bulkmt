#include <iostream>
#include <memory>
#include <string>

#include "interpreter.h"

using namespace std;
using namespace griha;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: bulk <block_size>" << endl;
        return -1;
    }
    
    const size_t block_size = stoul(argv[1]);

    Interpreter::Context context = {
        Reader{ block_size },
        Logger{},
        cout
    };

    Interpreter interpreter(std::move(context));
    interpreter.run(cin);
    return 0;
}