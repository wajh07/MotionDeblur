#include "ImageUtils.h"


QImage *ImageUtils::buildKernelImage(const Blur* motionBlur) {
    // motionLength plus 2*3 pixels to ensure that generated kernel will be fitted inside the image
    double motionLength = motionBlur->radius * 2;
    double motionAngle = motionBlur->angle;

    int size = motionLength + 6;
    size += size%2;

    QImage* kernelImage = new QImage(size, size, QImage::Format_RGB32);
    kernelImage->fill(Qt::black);

    // Prepare painter to have antialiasing and sub-pixel accuracy
    QPainter kernelPainter(kernelImage);
    kernelPainter.setRenderHint(QPainter::Antialiasing);

    // Workarround to have high accuracy, otherwise drawLine method has some micro-mistakes in the rendering
    QPen pen = kernelPainter.pen();
    pen.setColor(Qt::white);
    pen.setWidthF(1.01);
    kernelPainter.setPen(pen);

    double center = 0.5 + size/2;
    double motionAngleRad = M_PI*motionAngle/180;
    QLineF line(center - motionLength*cos(motionAngleRad)/2,
                center - motionLength*sin(motionAngleRad)/2,
                center + motionLength*cos(motionAngleRad)/2,
                center + motionLength*sin(motionAngleRad)/2);
    kernelPainter.drawLine(line);
    kernelPainter.end();

    return kernelImage;
}


void ImageUtils::fillMatrixFromImage(ProcessingContext *processingContext, const CurrentChannel channel) {
    for (int y=0; y<processingContext->height; y++) {
        QRgb *line = (QRgb*)processingContext->inputImage->constScanLine(y);
        for (int x=0; x<processingContext->width; x++) {
            int value = 0;
            switch (channel) {
            case RED:   value = qRed(line[x]); break;
            case GREEN: value = qGreen(line[x]); break;
            case BLUE:  value = qBlue(line[x]); break;
            case GRAY:  value = qGray(line[x]); break;
            }
            processingContext->inputImageMatrix[y*processingContext->width + x] = value;
        }
    }
}

void ImageUtils::fillImageFromMatrix(ProcessingContext* processingContext, const CurrentChannel channel) {
    double k = 1.0/(processingContext->width * processingContext->height);
    for (int y = 0; y < processingContext->height; y++) {
        QRgb *line = (QRgb*) processingContext->outputImage->scanLine(y);
        for (int x = 0; x < processingContext->width; x++) {
            double value =  k * processingContext->outputImageMatrix[y*processingContext->width + x];
            if (value < 0) {
                value = 0;
            }
            if (value > 255) {
                value = 255;
            }
            switch (channel) {
            case RED:   line[x] = qRgb(value, 0, 0); break;
            case GREEN: line[x] = line[x] | qRgb(0, value, 0); break;
            case BLUE:  line[x] = line[x] | qRgb(0, 0, value); break;
            case GRAY:  line[x] = qRgb(value, value, value); break;
            }
        }
    }
}
