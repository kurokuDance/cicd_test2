#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPixmap>
#include <QFileDialog>
#include <QVector>
#include <QRgb>
#include <QImage>
#include <QtMath>
#include <omp.h>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_clicked);
    connect(ui->rotateButton, &QPushButton::clicked, this, &MainWindow::on_rotateButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void rotateImage(QImage &image, float angle) posss
{
    int width = image.width(); 
    int height = image.height();
    int centerX = width / 2;
    int centerY = height / 2;

    float angleRadians = qDegreesToRadians(angle);
    double rotCos = qCos(angleRadians);
    double rotSin = qSin(angleRadians);
    int newWidth = (int)qRound(width * qAbs(rotCos) + height * qAbs(rotSin));
    int newHeight = (int)qRound(height * qAbs(rotCos) + width * qAbs(rotSin));
    int newCenterX = newWidth / 2;
    int newCenterY = newHeight / 2;

    QImage rotatedImage(newWidth, newHeight, image.format());
    QRgb *pixels = reinterpret_cast<QRgb *>(image.bits());
    QRgb *rotatedPixels = reinterpret_cast<QRgb *>(rotatedImage.bits());

#pragma omp parallel for collapse(2)
    for (int x = 0; x < newWidth; ++x)
        for (int y = 0; y < newHeight; ++y)
        {
            int rotatedX = qCos(angleRadians) * (x - newCenterX) - qSin(angleRadians) * (y - newCenterY) + centerX;
            int rotatedY = qSin(angleRadians) * (x - newCenterX) + qCos(angleRadians) * (y - newCenterY) + centerY;

            if (rotatedX >= 0 && rotatedX < width && rotatedY >= 0 && rotatedY < height)
                rotatedPixels[x + y * newWidth] = pixels[rotatedX + rotatedY * width];
            else
                rotatedPixels[x + y * newWidth] = qRgb(0, 0, 0);
        }

    image = rotatedImage;
}

void MainWindow::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open Image"), "", tr("Images (*.png *.jpg *.bmp)"));

    if (!filePath.isEmpty())
    {
        QPixmap picture(filePath);
        currentImage = picture.toImage();

        ui->myLabel->setPixmap(picture);

        ui->myLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        ui->myLabel->setFixedSize(currentImage.width(), currentImage.height());

        ui->scrollArea->setWidgetResizable(true);
        ui->scrollAreaWidgetContents->setGeometry(0, 0, currentImage.width(), currentImage.height());
        ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

void MainWindow::on_rotateButton_clicked()
{
    QElapsedTimer timer;
    timer.start();

    if (currentImage.isNull())
    {
        return;
    }

    QImage originalImage = currentImage;

    bool ok;
    double angle = ui->angleLineEdit->text().toDouble(&ok);
    if (!ok)
        return;

    rotateImage(originalImage, angle);

    ui->myLabel->setPixmap(QPixmap::fromImage(originalImage));

    ui->myLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    ui->myLabel->setFixedSize(originalImage.width(), originalImage.height());

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollAreaWidgetContents->setGeometry(0, 0, originalImage.width(), originalImage.height());
    ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    qint64 elapsedMilliseconds = timer.elapsed();
    double seconds = static_cast<double>(elapsedMilliseconds) / 1000.0;

    ui->timeLabel->setText("Выполнилось за: " + QString::number(seconds) + " сек.");
}


