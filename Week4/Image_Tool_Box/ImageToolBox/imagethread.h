#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QThread>


class imagethread : public QThread
{
    Q_OBJECT

public:
    imagethread();
    ~imagethread();

protected:
    void run() override;


};

#endif // IMAGETHREAD_H
