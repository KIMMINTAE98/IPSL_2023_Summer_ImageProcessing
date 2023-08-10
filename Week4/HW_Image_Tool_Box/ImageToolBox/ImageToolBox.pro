QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    imagethread.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    imagethread.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += D:\OpenCV\opencv\release\install\include

LIBS += \
    D:\OpenCV\opencv\release\bin\libopencv_core480.dll \
    D:\OpenCV\opencv\release\bin\libopencv_highgui480.dll \
    D:\OpenCV\opencv\release\bin\libopencv_imgproc480.dll \
    D:\OpenCV\opencv\release\bin\libopencv_imgcodecs480.dll \
