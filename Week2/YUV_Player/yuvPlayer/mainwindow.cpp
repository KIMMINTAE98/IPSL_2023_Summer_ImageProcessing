#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QPixmap>

#define CLIP(x) (x > 255 ? 255 : (x < 0 ? 0 : x))
typedef unsigned char BYTE;


BYTE** MemAlloc_2D(int width, int height)
{
    BYTE** arr = (BYTE**)malloc(sizeof(BYTE*) * height);
    for (int i = 0; i < height; i++)
    {
        arr[i] = (BYTE*)malloc(sizeof(BYTE) * width);
    }
    return arr;
}

void MemFree_2D(BYTE** arr, int height)
{
    for (int i = 0; i < height; i++)
        free(arr[i]);
    free(arr);
}

void YUV444_to_RGB(BYTE** img_in, BYTE** img_out, int width, int height)
{
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
        {
            int Y_temp = img_in[i             ][j] - 16;
            int U_temp = img_in[i + height    ][j] - 128;
            int V_temp = img_in[i + height * 2][j] - 128;
            img_out[i           ][j] = BYTE(CLIP((298 * Y_temp + 409 * V_temp + 128) >> 8));                // R
            img_out[i + height  ][j] = BYTE(CLIP((298 * Y_temp - 100 * U_temp - 208 * V_temp + 128) >> 8)); // G
            img_out[i + height*2][j] = BYTE(CLIP((298 * Y_temp + 516 * U_temp + 128) >> 8));                // B
        }
}

void YUV420_to_444(BYTE** Y, BYTE** U, BYTE** V, BYTE** img_out, int width, int height)
{
    for (int i = 0; i < height / 2; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            img_out[i           ][j            ] = Y[i             ][j            ];    // Y
            img_out[i           ][j + width / 2] = Y[i             ][j + width / 2];
            img_out[i+height / 2][j            ] = Y[i + height / 2][j            ];
            img_out[i+height / 2][j + width / 2] = Y[i + height / 2][j + width / 2];

            img_out[2 * i + height    ][2 * j    ] = U[i][j];                           // U
            img_out[2 * i + height    ][2 * j + 1] = U[i][j];
            img_out[2 * i + height + 1][2 * j    ] = U[i][j];
            img_out[2 * i + height + 1][2 * j + 1] = U[i][j];

            img_out[2 * i + 2*height    ][2 * j    ] = V[i][j];                         // V
            img_out[2 * i + 2*height    ][2 * j + 1] = V[i][j];
            img_out[2 * i + 2*height + 1][2 * j    ] = V[i][j];
            img_out[2 * i + 2*height + 1][2 * j + 1] = V[i][j];
        }
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      fileMenu(nullptr)
{
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(1440, 810);

    // menubar 셋팅
    fileMenu = menuBar()->addMenu("&File");

    // toolbar 셋팅
    fileToolBar = addToolBar("File");

    // image Info UI 셋팅
    imgInfoGroupBox = new QGroupBox("Input Image Info", this);
    imgInfoVBoxLayout = new QVBoxLayout;

    sizeHBoxLayout[0] = new QHBoxLayout;
    sizeHBoxLayout[1] = new QHBoxLayout;

    sizeLabel[0] = new QLabel("Width  : ", this);
    sizeLabel[1] = new QLabel("Height : ", this);

    sizeLineEdit[0] = new QLineEdit("", this);
    sizeLineEdit[1] = new QLineEdit("", this);

    sizeHBoxLayout[0]->addWidget(sizeLabel[0]);
    sizeHBoxLayout[0]->addWidget(sizeLineEdit[0]);
    sizeHBoxLayout[1]->addWidget(sizeLabel[1]);
    sizeHBoxLayout[1]->addWidget(sizeLineEdit[1]);

    imgInfoVBoxLayout->addLayout(sizeHBoxLayout[0]);
    imgInfoVBoxLayout->addLayout(sizeHBoxLayout[1]);

    formatRadioButtion[0] = new QRadioButton("YUV 4:4:4", this);
    formatRadioButtion[1] = new QRadioButton("YUV 4:2:0", this);
    formatRadioButtion[2] = new QRadioButton("YUV 4:0:0", this);
    formatRadioButtion[1]->setChecked(true);

    imgInfoVBoxLayout->addWidget(formatRadioButtion[0]);
    imgInfoVBoxLayout->addWidget(formatRadioButtion[1]);
    imgInfoVBoxLayout->addWidget(formatRadioButtion[2]);

    imgInfoGroupBox->setLayout(imgInfoVBoxLayout);
    imgInfoGroupBox->setGeometry(20, 70, 200, 200);

    // image 출력부 UI 셋팅
    imgLabel = new QLabel(this);
    imgArea = new QScrollArea(this);
    imgArea->setGeometry(240, 80, 540, 540);
    imgArea->setWidget(imgLabel);

    // font 설정
    QFont boldFont;
    boldFont.setBold(true);
    sizeLabel[0]->setFont(boldFont);
    sizeLabel[1]->setFont(boldFont);

    createActions();
}

void MainWindow::createActions()
{
    // menubar에 action 생성 및 추가
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    // toolbar에 action 추가
    fileToolBar->addAction(openAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
}

void MainWindow::openImage()
{
    // 입력 이미지 사이즈 저장
    QString w = sizeLineEdit[0]->text();
    QString h = sizeLineEdit[1]->text();
    int imgWidth  = w.toInt();
    int imgHeight = h.toInt();

    if (imgWidth <= 0 || imgHeight <= 0) {
        QMessageBox::warning(this,"Error","invalid image size!");
        return;
    }

    // 입력 이미지 format 저장
    int type = 1;
    if (formatRadioButtion[0]->isChecked())
        type = 0;
    else if (formatRadioButtion[1]->isChecked())
        type = 1;
    else if (formatRadioButtion[2]->isChecked())
        type = 2;
    else {
        QMessageBox::warning(this,"Error","invalid image format!");
        return;
    }

    // 입력 이미지 파일 오픈
    QString fileName = QFileDialog::getOpenFileName(this,"Open Image",QDir::homePath()); // filedialog 띄우기
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,"Error","can not open image file!");
        return;
    }

    // 입력 이미지 파일 bitstream 읽어오기
    QByteArray arr = file.readAll();
    uchar* data = (uchar*)arr.constData();

    /* YUV 444 format인 경우 */
    if (type == 0) {

        // buffer 선언
        uchar** yuv444 = MemAlloc_2D(imgWidth, 3 * imgHeight);
        uchar** rgb    = MemAlloc_2D(imgWidth, 3 * imgHeight);
        uchar* output  = (BYTE*)malloc(sizeof(BYTE) * imgWidth * imgHeight * 3);

        // yuv data 저장
        for(int i=0; i<3*imgHeight; i++)
        {
            for(int j=0; j<imgWidth; j++)
            {
                yuv444[i][j] = data[i * imgWidth + j];
            }
        }

        YUV444_to_RGB(yuv444, rgb, imgWidth, imgHeight);

        // rgb rgb rgb ... 형태로 변환
        for(int i=0; i<imgHeight; i++)
        {
            for(int j=0; j<imgWidth; j++)
            {
                output[i*3*imgWidth+3*j  ] = rgb[i              ][j];
                output[i*3*imgWidth+3*j+1] = rgb[i + imgHeight  ][j];
                output[i*3*imgWidth+3*j+2] = rgb[i + imgHeight*2][j];
            }
        }

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(output,imgWidth,imgHeight,QImage::Format_RGB888));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, imgWidth, imgHeight);

    }
    /* YUV 420 format인 경우 */
    else if (type == 1) {

        // buffer 선언
        uchar** img_Y = MemAlloc_2D(imgWidth,      imgHeight);
        uchar** img_U = MemAlloc_2D(imgWidth >> 1, imgHeight >> 1);
        uchar** img_V = MemAlloc_2D(imgWidth >> 1, imgHeight >> 1);

        uchar** yuv444 = MemAlloc_2D(imgWidth, 3 * imgHeight);
        uchar** rgb    = MemAlloc_2D(imgWidth, 3 * imgHeight);
        uchar* output  = (BYTE*)malloc(sizeof(BYTE) * imgWidth * imgHeight * 3);

        // y 성분 저장
        for(int i=0; i<imgHeight; i++)
        {
            for(int j=0; j<imgWidth; j++)
            {
                img_Y[i][j] = data[i*imgWidth + j];
            }
        }
        // u,v 성분 저장
        for(int i=0; i<imgHeight/2; i++)
        {
            for(int j=0; j<imgWidth/2; j++)
            {
                img_U[i][j] = data[imgHeight*imgWidth + i*imgWidth/2 + j];
                img_V[i][j] = data[5*imgHeight*imgWidth/4 + i*imgWidth/2 + j];
            }
        }

        YUV420_to_444(img_Y, img_U, img_V, yuv444, imgWidth, imgHeight);
        YUV444_to_RGB(yuv444, rgb, imgWidth, imgHeight);

        // rgb rgb rgb ... 형태로 변환
        for(int i=0; i<imgHeight; i++)
        {
            for(int j=0; j<imgWidth; j++)
            {
                output[i*3*imgWidth+3*j  ] = rgb[i              ][j];
                output[i*3*imgWidth+3*j+1] = rgb[i + imgHeight  ][j];
                output[i*3*imgWidth+3*j+2] = rgb[i + imgHeight*2][j];
            }
        }

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(output,imgWidth,imgHeight,QImage::Format_RGB888));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, imgWidth, imgHeight);

    }
    /* YUV 400 format인 경우 */
    else if (type == 2) {

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(data,imgWidth,imgHeight,QImage::Format_Grayscale8));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, imgWidth, imgHeight);

    }
    else {
        QMessageBox::warning(this,"Error","invalid image format!");
        return;
    }

}

