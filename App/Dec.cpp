#include <iostream>
#include <fstream>

#include "Decoder/Decode.h"

int main(int argc, char* argv[]) {
    std::string bsFilePath = std::string(argv[2]);
    std::ifstream infile(bsFilePath.c_str(), std::fstream::binary );
    if(!infile) {
        std::cerr << "W266 [error]: failed to open bitstream file " << std::endl;
        return -1;
    }

    AccessUnit* accessUnit = accessUnitAlloc();
    accessUnitAllocPayload(accessUnit, MAX_CODED_PICTURE_SIZE);

    Frame* frame = NULL;
    Frame* prev_field = NULL;

    return 0;
}