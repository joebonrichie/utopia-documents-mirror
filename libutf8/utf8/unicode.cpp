#include "unicode.h"

namespace utf8 {

    ssize_t utf8_advance_char(const uint8_t ** bytes)
    {
        ssize_t byte_count = utf8proc_utf8class[**bytes];
        *bytes += byte_count;
        return byte_count;
    }

}
