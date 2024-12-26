#include "DecLib.h"

void DecLib::create() {
    // run constructor again to ensure all variables, especially in DecLibParser have been reset
    this->~DecLib();
    new(this) DecLib;
}