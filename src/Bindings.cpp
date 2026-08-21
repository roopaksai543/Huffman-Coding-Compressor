#include <emscripten/bind.h>
#include "Huffman.h"

void runCompress(std::string in, std::string out) {
    Huffman h;
    h.compress(in, out);
}

void runDecompress(std::string in, std::string out) {
    Huffman h;
    h.decompress(in, out);
}

EMSCRIPTEN_BINDINGS(huffman_module) {
    emscripten::function("runCompress", &runCompress);
    emscripten::function("runDecompress", &runDecompress);
}
