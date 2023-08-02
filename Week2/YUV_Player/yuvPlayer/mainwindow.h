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
    void initUI();
    void createActions();


private slots:
    void openImage();


private:
    QMenu           *fileMenu;
    QToolBar        *fileToolBar;
    QAction         *openAction;
    QAction         *exitAction;

    QGroupBox       *imgInfoGroupBox;
    QVBoxLayout     *imgInfoVBoxLayout;

    QHBoxLayout     *sizeHBoxLayout[2];
    QLabel          *sizeLabel[2];
    QLineEdit       *sizeLineEdit[2];

    QRadioButton    *formatRadioButtion[3];

    QLabel          *imgLabel;
    QScrollArea     *imgArea;

};

#endif // MAINWINDOW_H
