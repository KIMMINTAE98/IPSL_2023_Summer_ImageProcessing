#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QPixmap>
#include <QtMath>
#include "opencv2/opencv.hpp"

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
BYTE*** MemAlloc_3D(int frame, int width, int height)
{
    BYTE*** arr = (BYTE***)malloc(sizeof(BYTE**) * frame);
    for (int j = 0; j < frame; j++)
    {
        arr[j] = (BYTE**)malloc(sizeof(BYTE*) * height);
        for (int i = 0; i < height; i++)
        {
            arr[j][i] = (BYTE*)malloc(sizeof(BYTE) * width);
        }
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
void MemFree_3D(BYTE*** arr, int frame, int height)
{
    for (int j = 0; j < frame; j++)
    {
        for (int i = 0; i < height; i++)
        {
            free(arr[j][i]);
        }
        free(arr[j]);
    }
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
    currentFrame(-1),
    fileMenu(nullptr)
{
    initUI();

    videoThread = new imagethread();
    connect(videoThread, SIGNAL(threadUpdate()), this, SLOT(updateVideo()));
}

MainWindow::~MainWindow()
{
    videoThread->stop();
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

    // thread 실습 UI
    threadButton = new QPushButton("Run Thread", this);
    threadButton->setGeometry(20, 300, 200, 50);
    videoLabel = new QLabel(this);
    videoLabel->setGeometry(20, 400, 200, 200);

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

    gaussianAction  = new QAction("Gaussian Filtering", this);
    sobelAction     = new QAction("Sobel Filtering", this);
    laplacianAction = new QAction("Laplacian Filtering", this);
    cannyAction     = new QAction("Canny Edge Detector", this);
    harrisAction    = new QAction("Harris Corner Detector", this);
    houghAction     = new QAction("Hough Transform", this);
    editMenu->addAction(gaussianAction);
    editMenu->addAction(sobelAction);
    editMenu->addAction(laplacianAction);
    editMenu->addAction(cannyAction);
    editMenu->addAction(harrisAction);
    editMenu->addAction(houghAction);

    // toolbar에 action 추가
    fileToolBar->addAction(openAction);
    editToolBar->addAction(gaussianAction);
    editToolBar->addAction(sobelAction);
    editToolBar->addAction(laplacianAction);
    editToolBar->addAction(cannyAction);
    editToolBar->addAction(harrisAction);
    editToolBar->addAction(houghAction);

    // connect the signals and slots
    connect(exitAction,         SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));

    connect(openAction,         SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(gaussianAction,     SIGNAL(triggered(bool)), this, SLOT(gaussianFiltering()));
    connect(sobelAction,        SIGNAL(triggered(bool)), this, SLOT(sobelFiltering()));
    connect(laplacianAction,    SIGNAL(triggered(bool)), this, SLOT(laplacianFiltering()));
    connect(cannyAction,        SIGNAL(triggered(bool)), this, SLOT(cannyEdgeDectection()));
    connect(harrisAction,       SIGNAL(triggered(bool)), this, SLOT(harrisCornerDectection()));
    connect(houghAction,        SIGNAL(triggered(bool)), this, SLOT(houghTransform()));

    connect(threadButton,       SIGNAL(clicked()),       this, SLOT(startThread()));
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
        uchar* output  = (BYTE*)malloc(sizeof(BYTE) * width * height * 3);

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
                output[i*3*width+3*j  ] = rgb[i           ][j];
                output[i*3*width+3*j+1] = rgb[i + height  ][j];
                output[i*3*width+3*j+2] = rgb[i + height*2][j];
            }
        }
        // image display
        img = QImage(output,width,height,QImage::Format_RGB888);
        QPixmap pix = QPixmap::fromImage(img);
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 저장 및 출력
        uchar* outputGray = (BYTE*)malloc(sizeof(BYTE) *  width * height);
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                outputGray[i*width + j] =  yuv444[i][j];
            }
        }
        imgGrayscale = QImage(outputGray,width,height,QImage::Format_Grayscale8);
        QPixmap pixGray = QPixmap::fromImage(imgGrayscale);
        imgLabel2->setPixmap(pixGray);
        imgLabel2->setGeometry(0, 0, width, height);

        MemFree_2D(yuv444, 3 * height);
        MemFree_2D(rgb,    3 * height);

    }
    /* YUV 420 format인 경우 */
    else if (type == 1) {

        // buffer 선언
        uchar** img_Y = MemAlloc_2D(width,      height);
        uchar** img_U = MemAlloc_2D(width >> 1, height >> 1);
        uchar** img_V = MemAlloc_2D(width >> 1, height >> 1);

        uchar** yuv444 = MemAlloc_2D(width, 3 * height);
        uchar** rgb    = MemAlloc_2D(width, 3 * height);
        uchar* output  = (BYTE*)malloc(sizeof(BYTE) * width * height * 3);

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
                output[i*3*width+3*j  ] = rgb[i           ][j];
                output[i*3*width+3*j+1] = rgb[i + height  ][j];
                output[i*3*width+3*j+2] = rgb[i + height*2][j];
            }
        }

        // image display
        img = QImage(output,width,height,QImage::Format_RGB888);
        QPixmap pix = QPixmap::fromImage(img);
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 저장 및 출력
        uchar* outputGray = (BYTE*)malloc(sizeof(BYTE) * width * height);
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                outputGray[i*width + j] =  img_Y[i][j];
            }
        }
        imgGrayscale = QImage(outputGray,width,height,QImage::Format_Grayscale8);
        QPixmap pixGray = QPixmap::fromImage(imgGrayscale);
        imgLabel2->setPixmap(pixGray);
        imgLabel2->setGeometry(0, 0, width, height);

        MemFree_2D(img_Y,  height);
        MemFree_2D(img_U,  height>>1);
        MemFree_2D(img_V,  height>>1);
        MemFree_2D(yuv444, height*3);
        MemFree_2D(rgb,    height*3);

    }
    /* YUV 400 format인 경우 */
    else if (type == 2) {

        uchar* outputGray = (BYTE*)malloc(sizeof(BYTE) * width * height);
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                outputGray[i*width + j] =  data[i*width + j];
            }
        }

        // image display
        img = QImage(outputGray, width, height, QImage::Format_Grayscale8);
        QPixmap pix = QPixmap::fromImage(img);
        imgLabel->setPixmap(pix);
        imgLabel->setGeometry(0, 0, width, height);

        // grayscale 이미지 출력
        imgGrayscale = QImage(outputGray, width, height, QImage::Format_Grayscale8);
        QPixmap pixGray = QPixmap::fromImage(imgGrayscale);
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
    imgFilePath = fileName;

    return;
}


void MainWindow::gaussianFiltering()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    // 가우시안 필터 파라미터
    int filterSize = 9;  // 홀수!
    int filterStd = 3;

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat output;

    // Write your opencv code
    /*
        Gaussian filtering
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_Grayscale8);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}

void MainWindow::sobelFiltering()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    // 소벨 필터링 파라미터
    BYTE threshold = 40;

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat output;

    //  Write your opencv code
    /*
        Sobel filtering
        threashold 이진화
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_Grayscale8);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}

void MainWindow::laplacianFiltering()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat output;

    //  Write your opencv code
    /*
        Laplacian filtering
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_Grayscale8);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}

void MainWindow::cannyEdgeDectection()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    // Canny Edge Dectection 파라미터
    BYTE lowThreshold = 50;
    BYTE highThreshold = 150;

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat output;

    //  Write your opencv code
    /*
        Canny edge detection
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_Grayscale8);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}

void MainWindow::harrisCornerDectection()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    // Harris Corner Dectection 파라미터
    int blockSize = 3;
    BYTE threshold = 100;

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat harris;
    cv::Mat harrisNorm;
    cv::Mat output;

    //  Write your opencv code
    /*
        Harris corner detection
        Normalize harris value
        Change image format gray to color
        Draw circle corner by thresholding and finding local maxima
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}

void MainWindow::houghTransform()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no grayscale image!");
        return;
    }

    // Canny Edge Dectection 파라미터
    BYTE lowThreshold = 50;
    BYTE highThreshold = 150;

    int  voteThreshold = 250;

    cv::Mat input = cv::Mat(imgHeight, imgWidth, CV_8UC1, (void*)imgGrayscale.constBits(), imgGrayscale.bytesPerLine());
    cv::Mat canny;
    std::vector<cv::Vec2f> lines;
    cv::Mat output;

    //  Write your opencv code
    /*
        Canny Edge detection
        Get line using Hough transform
        Change image format gray to color
        Draw line
    */

    QImage imgOutput(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(imgOutput);
    imgLabel3->setPixmap(pix);
    imgLabel3->setGeometry(0, 0, imgWidth, imgHeight);

    return;
}


void MainWindow::startThread()
{
    // 원본 이미지 없는 경우 에러 출력
    if (!isImage) {
        QMessageBox::warning(this,"Error","no image!");
        return;
    }

    // [YUV444]Suzie_CIF_352x288 아닌 경우 에러 출력
    if (imgType != 0 || imgWidth != 352 || imgHeight != 288) {
        QMessageBox::warning(this,"Error","input [YUV444]Suzie_CIF_352x288 image!");
        return;
    }

    // 입력 이미지 파일 오픈
    QFile file(imgFilePath);
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,"Error","can not open image file!");
        return;
    }

    // 입력 이미지 파일 bitstream 읽어오기
    QByteArray arr = file.readAll();
    uchar* data = (uchar*)arr.constData();

    int frame = 30;
    int frameSize = imgWidth * 3 * imgHeight;

    // buffer 선언
    uchar*** yuv444 = MemAlloc_3D(frame, imgWidth, 3 * imgHeight);
    uchar*** rgb    = MemAlloc_3D(frame, imgWidth, 3 * imgHeight);
    video = (BYTE*)malloc(sizeof(BYTE) * frame * imgWidth * imgHeight * 3);

    // yuv data 저장 & RGB 변환
    for (int f = 0; f < 30; f++)
    {
        for (int j = 0; j < 3*imgHeight; j++)
        {
            for (int i = 0; i < imgWidth; i++)
            {
                yuv444[f][j][i] = data[f * frameSize + j * imgWidth + i];
            }
        }
        YUV444_to_RGB(yuv444[f], rgb[f], imgWidth, imgHeight);
    }

    // rgb rgb rgb ... 형태로 변환
    for (int f = 0; f < 30; f++)
    {
        for (int j = 0; j < imgHeight; j++)
        {
            for (int i = 0; i < imgWidth; i++)
            {
                video[f*frameSize + j*3*imgWidth + 3*i    ] = rgb[f][j              ][i];
                video[f*frameSize + j*3*imgWidth + 3*i + 1] = rgb[f][j + imgHeight  ][i];
                video[f*frameSize + j*3*imgWidth + 3*i + 2] = rgb[f][j + imgHeight*2][i];
            }
        }

    }

    currentFrame = 0;
    int currentIdx = currentFrame * frameSize;

    // image display
    QImage imgFrame = QImage(&(video[currentIdx]), imgWidth, imgHeight, QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(imgFrame);
    videoLabel->setPixmap(pix.scaled(200,200,Qt::KeepAspectRatio));

    MemFree_3D(yuv444, frame, 3 * imgHeight);
    MemFree_3D(rgb,    frame, 3 * imgHeight);

    // video thread 실행
    videoThread->start();

    return;
}

void MainWindow::updateVideo()
{
    currentFrame++;
    if (currentFrame >= 30)
        currentFrame = 0;

    int currentIdx = currentFrame * imgWidth * imgHeight * 3;

    QImage imgFrame = QImage(&(video[currentIdx]), imgWidth, imgHeight, QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(imgFrame);
    videoLabel->setPixmap(pix.scaled(200,200,Qt::KeepAspectRatio));

    return;
}
