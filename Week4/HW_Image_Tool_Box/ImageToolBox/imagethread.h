#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QThread>
#include <unistd.h>


class imagethread : public QThread
{
    Q_OBJECT

public:
    imagethread();
    ~imagethread();
    void stop();

private:
    bool stopFlag;

private:
    void run();

signals:
    void threadUpdate();

};

#endif // IMAGETHREAD_H
