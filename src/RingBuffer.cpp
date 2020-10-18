#include "RingBuffer.h"

RingBuffer::RingBuffer(int t_maxSize) : maxSize(t_maxSize)
{
    pos = 0;
    currSize = 0;
    buf = new short[maxSize];
    memset(buf, 0, maxSize);
}

RingBuffer::~RingBuffer()
{
    delete[] buf;
    buf = NULL;
}

void RingBuffer::put(short value)
{
    buf[pos] = value;
    pos++;
    if (currSize < maxSize) 
    {
        currSize++;
    }
    if (pos >= maxSize)
    {
        pos = 0;
    }
}

void RingBuffer::read(RingBuffer::ReadFunction func)
{
    int idx = pos - 1;
    if (idx < 0)
    {
        idx = maxSize - 1;
    }
    int takenItems = 0;
    while (takenItems < currSize)
    {
        short item = buf[idx];
        if (!func(item))
        {
            return;
        }

        idx--;
        takenItems++;

        if (idx < 0)
        {
            idx = maxSize - 1;
        }
    }
}
