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


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool isImage;
    int imgWidth;
    int imgHeight;
    int imgType;
    uchar* img;
    uchar* imgGrayscale;

private:
    void initUI();
    void createActions();

private slots:
    void openImage();
    void gaussianFiltering();
    void edgeDetection();

private:
    QMenu           *fileMenu;
    QMenu           *editMenu;

    QToolBar        *fileToolBar;
    QToolBar        *editToolBar;

    QAction         *openAction;
    QAction         *exitAction;
    QAction         *filterAction;
    QAction         *edgeAction;

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

};

#endif // MAINWINDOW_H
