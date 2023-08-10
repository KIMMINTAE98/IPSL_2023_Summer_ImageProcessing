#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QPixmap>
#include <QtMath>

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
double** MemAlloc_2D_double(int width, int height)
{
    double** arr = (double**)malloc(sizeof(double*) * height);
    for (int i = 0; i < height; i++)
    {
        arr[i] = (double*)malloc(sizeof(double) * width);
    }
    return arr;
}

void MemFree_2D(BYTE** arr, int height)
{
    for (int i = 0; i < height; i++)
        free(arr[i]);
    free(arr);
}
void MemFree_2D_double(double** arr, int height)
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
      isImage(false),
      imgWidth(-1),
      imgHeight(-1),
      imgType(-1),
      img(nullptr),
      imgGrayscale(nullptr),
      fileMenu(nullptr)
{
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(1920, 810);

    // menubar 셋팅
    fileMenu = menuBar()->addMenu("&File");
    editMenu = menuBar()->addMenu("&Edit");

    // toolbar 셋팅
    fileToolBar = addToolBar("File");
    editToolBar = addToolBar("Edit");

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
    txtLabel = new QLabel("Original Image", this);
    txtLabel->setGeometry(245, 620, 200, 50);

    imgLabel2 = new QLabel(this);
    imgArea2 = new QScrollArea(this);
    imgArea2->setGeometry(800, 80, 540, 540);
    imgArea2->setWidget(imgLabel2);
    txtLabel2 = new QLabel("Grayscale Image", this);
    txtLabel2->setGeometry(805, 620, 200, 50);

    imgLabel3 = new QLabel(this);
    imgArea3 = new QScrollArea(this);
    imgArea3->setGeometry(1360, 80, 540, 540);
    imgArea3->setWidget(imgLabel3);
    txtLabel3 = new QLabel("Processed Image", this);
    txtLabel3->setGeometry(1365, 620, 200, 50);

    // font 설정
    QFont boldFont;
    boldFont.setBold(true);
    sizeLabel[0]->setFont(boldFont);
    sizeLabel[1]->setFont(boldFont);
    txtLabel->setFont(boldFont);
    txtLabel2->setFont(boldFont);
    txtLabel3->setFont(boldFont);

    createActions();
}

void MainWindow::createActions()
{
    // menubar에 action 생성 및 추가
    openAction = new QAction("&Open", this);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(openAction);
    fileMenu->addAction(exitAction);

    filterAction = new QAction("Gaussian Filtering", this);
    edgeAction   = new QAction("Edge Detection", this);
    editMenu->addAction(filterAction);
    editMenu->addAction(edgeAction);

    // toolbar에 action 추가
    fileToolBar->addAction(openAction);

    editToolBar->addAction(filterAction);
    editToolBar->addAction(edgeAction);

    // connect the signals and slots
    connect(exitAction,         SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction,         SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(filterAction,       SIGNAL(triggered(bool)), this, SLOT(gaussianFiltering()));
    connect(edgeAction,         SIGNAL(triggered(bool)), this, SLOT(edgeDetection()));
}

void MainWindow::openImage()
{
    // 입력 이미지 사이즈 저장
    QString w = sizeLineEdit[0]->text();
    QString h = sizeLineEdit[1]->text();
    int width  = w.toInt();
    int height = h.toInt();

    if (width <= 0 || height <= 0) {
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
        uchar** yuv444 = MemAlloc_2D(width, 3 * height);
        uchar** rgb    = MemAlloc_2D(width, 3 * height);
        img  = (BYTE*)malloc(sizeof(BYTE) * width * height * 3);

        // yuv data 저장
        for(int i=0; i<3*height; i++)
        {
            for(int j=0; j<width; j++)
            {
                yuv444[i][j] = data[i * width + j];
            }
        }

        YUV444_to_RGB(yuv444, rgb, width, height);

        // rgb rgb rgb ... 형태로 변환
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                img[i*3*width+3*j  ] = rgb[i              ][j];
                img[i*3*width+3*j+1] = rgb[i + height  ][j];
                img[i*3*width+3*j+2] = rgb[i + height*2][j];
            }
        }

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(img,width,height,QImage::Format_RGB888));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 저장 및 출력
        imgGrayscale = (BYTE*)malloc(sizeof(BYTE) *  width * height);
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                imgGrayscale[i*width + j] =  yuv444[i][j];
            }
        }
        QPixmap pixGray = QPixmap::fromImage(QImage(imgGrayscale,width,height,QImage::Format_Grayscale8));
        imgLabel2->setPixmap(pixGray);
        imgLabel2->setGeometry(0, 0, width, height);

    }
    /* YUV 420 format인 경우 */
    else if (type == 1) {

        // buffer 선언
        uchar** img_Y = MemAlloc_2D(width,      height);
        uchar** img_U = MemAlloc_2D(width >> 1, height >> 1);
        uchar** img_V = MemAlloc_2D(width >> 1, height >> 1);

        uchar** yuv444 = MemAlloc_2D(width, 3 * height);
        uchar** rgb    = MemAlloc_2D(width, 3 * height);
        img = (BYTE*)malloc(sizeof(BYTE) * width * height * 3);

        // y 성분 저장
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                img_Y[i][j] = data[i*width + j];
            }
        }
        // u,v 성분 저장
        for(int i=0; i<height/2; i++)
        {
            for(int j=0; j<width/2; j++)
            {
                img_U[i][j] = data[height*width + i*width/2 + j];
                img_V[i][j] = data[5*height*width/4 + i*width/2 + j];
            }
        }

        YUV420_to_444(img_Y, img_U, img_V, yuv444, width, height);
        YUV444_to_RGB(yuv444, rgb, width, height);

        // rgb rgb rgb ... 형태로 변환
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                img[i*3*width+3*j  ] = rgb[i           ][j];
                img[i*3*width+3*j+1] = rgb[i + height  ][j];
                img[i*3*width+3*j+2] = rgb[i + height*2][j];
            }
        }

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(img,width,height,QImage::Format_RGB888));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 저장 및 출력
        imgGrayscale = (BYTE*)malloc(sizeof(BYTE) * width * height);
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                imgGrayscale[i*width + j] =  img_Y[i][j];
            }
        }
        QPixmap pixGray = QPixmap::fromImage(QImage(imgGrayscale,width,height,QImage::Format_Grayscale8));
        imgLabel2->setPixmap(pixGray);
        imgLabel2->setGeometry(0, 0, width, height);

    }
    /* YUV 400 format인 경우 */
    else if (type == 2) {

        img = (BYTE*)malloc(sizeof(BYTE) * width * height);
        imgGrayscale = (BYTE*)malloc(sizeof(BYTE) * width * height);

        // y 성분 및 grayscale 이미지 저장
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                img[i*width + j] = data[i*width + j];
                imgGrayscale[i*width + j] = data[i*width + j];
            }
        }

        // image display
        QPixmap pix = QPixmap::fromImage(QImage(img,width,height,QImage::Format_Grayscale8));
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 출력
        QPixmap pixGray = QPixmap::fromImage(QImage(imgGrayscale,width,height,QImage::Format_Grayscale8));
        imgLabel2->setPixmap(pixGray);
        imgLabel2->setGeometry(0, 0, width, height);

    }
    else {
        QMessageBox::warning(this,"Error","invalid image format!");
        return;
    }

    isImage = true;
    imgWidth = width;
    imgHeight = height;
    imgType = type;

    return;
}



void MainWindow::gaussianFiltering()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    BYTE* output = (BYTE*)malloc(sizeof(BYTE) * imgWidth * imgHeight);

    // 가우시안 필터 파라미터
    int filterSize = 9;  // 홀수!
    int filterStd = 3;

    // 가우시안 필터 저장 or 구하기
    double** filter = MemAlloc_2D_double(filterSize, filterSize);

    int filterRange = (filterSize - 1) / 2;
    double scaleValue = 0;
    for (int x = -1 * filterRange; x <= filterRange; x++)
    {
        for (int y = -1 * filterRange; y <= filterRange; y++)
        {
            double gaussianValue = 1 / (2 * M_PI * pow(filterStd, 2)) * pow(M_E, -1 * (pow(x, 2) + pow(y, 2)) / (2 * pow(filterStd, 2)));
            filter[x + filterRange][y + filterRange] = gaussianValue;
            scaleValue += gaussianValue;
        }
    }

    // 입력 이미지에 가우시안 필터링 적용
    for(int i=0; i<imgHeight; i++)
    {
        for(int j=0; j<imgWidth; j++)
        {
            double filteringValue = 0;
            for (int x = -1 * filterRange; x <= filterRange; x++)
            {
                for (int y = -1 * filterRange; y <= filterRange; y++)
                {
                    // 필터링 위치 저장
                    int h = i + x;
                    int w = j + y;

                    // padding 동작
                    if (h < 0 || h >= imgHeight) {
                        h = i;
                    }
                    if (w < 0 || w >= imgWidth) {
                        w = j;
                    }

                    filteringValue += (imgGrayscale[h * imgWidth + w] * filter[x + filterRange][y + filterRange] / scaleValue);
                }
            }
            output[i*imgWidth + j] = BYTE(CLIP(filteringValue));
        }
    }

    // 필터링 된 이미지 출력
    QPixmap pix = QPixmap::fromImage(QImage(output,imgWidth,imgHeight,QImage::Format_Grayscale8));
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    MemFree_2D_double(filter, filterSize);
    free(output);

    return;
}

void MainWindow::edgeDetection()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    BYTE* output = (BYTE*)malloc(sizeof(BYTE) * imgWidth * imgHeight);

    // 소벨 필터링 파라미터
    BYTE threshold = 50; // 0 ~ 255 범위

    // 소벨 필터 저장
    int filter[3][3] = {{-1,  0,  1},
                        {-2,  0,  2},
                        {-1,  0,  1}};
    //int filter[3][3] = {{-1, -2, -1},
    //                    { 0,  0,  0},
    //                    { 1,  2,  1}};
    int filterRange = (3 - 1) / 2;

    // 입력 이미지에 소벨 필터링 적용
    for(int i=0; i<imgHeight; i++)
    {
        for(int j=0; j<imgWidth; j++)
        {
            int filteringValue = 0;
            for (int x = -1 * filterRange; x <= filterRange; x++)
            {
                for (int y = -1 * filterRange; y <= filterRange; y++)
                {
                    // 필터링 위치 저장
                    int h = i + x;
                    int w = j + y;

                    // padding 동작
                    if (h < 0 || h >= imgHeight) {
                        h = i;
                    }
                    if (w < 0 || w >= imgWidth) {
                        w = j;
                    }

                    filteringValue += (imgGrayscale[h * imgWidth + w] * filter[x + filterRange][y + filterRange]);
                }
            }
            // edge 판단
            if (abs(filteringValue) > threshold)
                output[i*imgWidth + j] = 255;   // edge O
            else
                output[i*imgWidth + j] = 0;     // edge X
        }
    }

    // 필터링 된 이미지 출력
    QPixmap pix = QPixmap::fromImage(QImage(output,imgWidth,imgHeight,QImage::Format_Grayscale8));
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    free(output);

    return;
}
