#ifndef __STRINGPRINTER_H__
#define __STRINGPRINTER_H__

#include <Arduino.h>

class StringPrinter : public Print
{
public:
    StringPrinter(char *buffer);
    virtual size_t write(uint8_t c);

private:
    char *buf;
    size_t pos;
};

#endif