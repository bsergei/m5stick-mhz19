#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <functional>
#include <Arduino.h>

class RingBuffer
{
public:
    RingBuffer(int t_maxSize);
    ~RingBuffer();

    const int maxSize;

    void put(short value);

    typedef std::function<bool(short)> ReadFunction;
    void read(ReadFunction func);

private:
    int currSize;
    int pos;
    short *buf;
};

#endif