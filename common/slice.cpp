#include "slice.h"

ReferencePictureList::ReferencePictureList() {
  ::memset( this, 0, sizeof( *this ) );
}

void ReferencePictureList::clear()
{
  ::memset( this, 0, sizeof( *this ) );
}