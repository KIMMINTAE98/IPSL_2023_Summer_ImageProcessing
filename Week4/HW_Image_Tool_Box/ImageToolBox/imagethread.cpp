#include "imagethread.h"


imagethread::imagethread()
    : stopFlag(false)
{

}

imagethread::~imagethread()
{

}


void imagethread::run()
{
    while(!stopFlag)
    {
        emit threadUpdate();
        msleep(100);
    }
}

void imagethread::stop()
{
    stopFlag = true;
}
