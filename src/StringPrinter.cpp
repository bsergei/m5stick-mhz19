#include "StringPrinter.h"

StringPrinter::StringPrinter(char *buffer) : buf(buffer), pos(0)
{
}

size_t StringPrinter::write(uint8_t c)
{
    buf[pos++] = c;
    buf[pos] = 0;
    return 1;
}
