#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QImage>
#include <QPushButton>

#include "imagethread.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool            isImage;
    int             imgWidth;
    int             imgHeight;
    int             imgType;
    QString         imgFilePath;
    QImage          img;
    QImage          imgGrayscale;
    imagethread     *videoThread;
    uchar           *video;
    int             currentFrame;

private:
    void initUI();
    void createActions();

private slots:
    void openImage();
    void gaussianFiltering();
    void sobelFiltering();
    void laplacianFiltering();
    void cannyEdgeDectection();
    void harrisCornerDectection();
    void houghTransform();
    void startThread();
    void updateVideo();

private:
    QMenu           *fileMenu;
    QMenu           *editMenu;

    QToolBar        *fileToolBar;
    QToolBar        *editToolBar;

    QAction         *openAction;
    QAction         *exitAction;
    QAction         *gaussianAction;
    QAction         *sobelAction;
    QAction         *laplacianAction;
    QAction         *cannyAction;
    QAction         *harrisAction;
    QAction         *houghAction;

    QGroupBox       *imgInfoGroupBox;
    QVBoxLayout     *imgInfoVBoxLayout;

    QHBoxLayout     *sizeHBoxLayout[2];
    QLabel          *sizeLabel[2];
    QLineEdit       *sizeLineEdit[2];

    QRadioButton    *formatRadioButtion[3];

    QLabel          *txtLabel;
    QLabel          *imgLabel;
    QScrollArea     *imgArea;

    QLabel          *txtLabel2;
    QLabel          *imgLabel2;
    QScrollArea     *imgArea2;

    QLabel          *txtLabel3;
    QLabel          *imgLabel3;
    QScrollArea     *imgArea3;

    QPushButton     *threadButton;
    QLabel          *videoLabel;

};

#endif // MAINWINDOW_H
