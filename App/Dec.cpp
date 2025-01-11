#include <iostream>
#include <fstream>
#include <algorithm>

#include "Decoder/Decode.h"

static inline int retrieveNalStartCode(unsigned char *pB, int iZerosInStartcode) {
    int info = 1;
    int i=0;
    for (i = 0; i < iZerosInStartcode; i++) {
        if( pB[i] != 0 ) {
            info = 0;
        }
    }

    if (pB[i] != 1 ){
        info = 0;
    }

    return info;
}

static int readBitstreamFromFile(std::ifstream *f, AccessUnit* pcAccessUnit, bool bLoop) {
    int info2=0;
    int info3=0;
    int pos = 0;

    int iStartCodeFound =0;
    int iRewind=0;
    uint32_t len;
    unsigned char* pBuf = pcAccessUnit->payload;
    pcAccessUnit->payloadUsedSize = 0;

    auto curfilpos = f->tellg();
    if(curfilpos < 0) {
        if(bLoop) {
            f->clear();
            f->seekg(0, f->beg);
            if(f->bad() || f->fail()) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    //jump over possible start code
    f->read((char*)pBuf, 5);
    size_t extracted = f->gcount();
    if(extracted < 4) {
        if( bLoop ) {
            f->clear();
            f->seekg( 0, f->beg );
            if(f->bad() || f->fail()) {
                return -1;
            }

            f->read((char*)pBuf, 5);
            size_t extracted = f->gcount();
            if(extracted < 4) {
                return -1;
            }
        }
        else {
            return -1;
        }
    }

    pos +=5;
    info2 = 0;
    info3 = 0;
    iStartCodeFound = 0;

    while(!iStartCodeFound) {
        if(f->eof()) {
            if(pos > 5) {
                len = pos - 1;
                pcAccessUnit->payloadUsedSize=len;
                return len;
            } else if(bLoop) {
                f->clear();
                f->seekg( 0, f->beg );
            } else {
                return -1;
            }
        }

        if( pos >= pcAccessUnit->payloadSize ) {
            int iNewSize = pcAccessUnit->payloadSize*2;
            unsigned char* newbuf = (unsigned char*) malloc(sizeof( unsigned char ) * iNewSize);
            if(newbuf == NULL) {
                fprintf( stderr, "ERR: readBitstreamFromFile: memory re-allocation failed!\n" );
                return -1;
            }
            std::copy_n( pcAccessUnit->payload, std::min(pcAccessUnit->payloadSize , iNewSize), newbuf);
            pcAccessUnit->payloadSize = iNewSize;
            free( pcAccessUnit->payload );

            pcAccessUnit->payload = newbuf;
            pBuf = pcAccessUnit->payload;
        }
        unsigned char* p= pBuf + pos;
        f->read((char*)p, 1);
        pos++;

        info3 = retrieveNalStartCode(&pBuf[pos-4], 3);
        if(info3 != 1)
        {
        info2 = retrieveNalStartCode(&pBuf[pos-3], 2);
        }
        iStartCodeFound = (info2 == 1 || info3 == 1);
    }


    // Here, we have found another start code (and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    iRewind = 0;
    if(info3 == 1)
        iRewind = -4;
    else if (info2 == 1)
        iRewind = -3;
    else
        fprintf( stderr, "ERR: readBitstreamFromFile: Error in next start code search \n");

    f->seekg (iRewind, f->cur);
    if(f->bad() || f->fail()) {
        fprintf( stderr, "ERR: readBitstreamFromFile: Cannot seek %d in the bit stream file", iRewind );
        return -1;
    }

    // Here the Start code, the complete NALU, and the next start code is in the pBuf.
    // The size of pBuf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU

    len = (pos+iRewind);
    pcAccessUnit->payloadUsedSize=len;
    return len;
}

static bool handle_frame() {
    return true;
}

int main(int argc, char* argv[]) {
    std::string bsFilePath = std::string(argv[2]);
    std::ifstream cInFile(bsFilePath.c_str(), std::fstream::binary );
    if(!cInFile) {
        std::cerr << "W266 [error]: failed to open bitstream file " << std::endl;
        return -1;
    }

    AccessUnit* accessUnit = accessUnitAlloc();
    accessUnitAllocPayload(accessUnit, MAX_CODED_PICTURE_SIZE);

    Frame* pcFrame = NULL;
    Frame* pcPrevField = NULL;

    Decoder* dec = decoderOpen();

    accessUnit->cts = 0; accessUnit->ctsValid = true;
    accessUnit->dts = 0; accessUnit->dtsValid = true;

    int iRet = -1;
    int iRead = 0;
    do {
        iRead = readBitstreamFromFile(&cInFile, accessUnit, false);
        NalType eNalType = getNalUnitType(accessUnit);

        iRet = decode(dec, accessUnit, &pcFrame);

        handle_frame();

    } while (iRead > 0);

    decoderClose();

    accessUnitFree();

    return 0;
}