#include <iostream>
#include <fstream>

#include "decoder/decode.h"

int main(int argc, char* argv[]) {
    std::string bs_file_path = std::string(argv[2]);
    std::ifstream infile(bs_file_path.c_str(), std::fstream::binary );
    if(!infile) {
        std::cerr << "vvdecapp [error]: failed to open bitstream file " << std::endl;
        return -1;
    }

    AccessUnit* access_unit = access_unit_alloc();
    access_unit_alloc_payload(access_unit, MAX_CODED_PICTURE_SIZE);

    Frame* frame = NULL;
    Frame* prev_field = NULL;

    return 0;
}