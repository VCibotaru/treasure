//
//  OutputMethods.cpp
//  treasure
//
//  Created by Viktor Chibotaru on 10/5/14.
//  Copyright (c) 2014 Viktor Chibotaru. All rights reserved.
//

#include "ImageProcessor.h"

#define HIST_H 1000

//convert YUV(grayscale) to RGB
std::shared_ptr<Image> getRGBFromYUV(const std::shared_ptr<Matrix<uint>> & grayscale){
    std::shared_ptr<Image> color = std::make_shared<Image>(grayscale->n_rows, grayscale->n_cols);
    for (uint i = 0 ; i < grayscale->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscale->n_cols ; ++j) {
            uint val = int((*grayscale)(i,j));
            (*color)(i,j) = std::make_tuple(val, val, val);
        }
    }
    return color;
}

std::shared_ptr<Image> ImageProcessor::getGrayscale() const {
    return getRGBFromYUV(grayscaleImage);
}
std::shared_ptr<Image> ImageProcessor::getBin() const {
    return getRGBFromYUV(binImage);
}
std::shared_ptr<Matrix<uint>> ImageProcessor::getLabeled() const {
    return labelImage;
}
std::shared_ptr<Image> ImageProcessor::getHistogram() const {
    std::shared_ptr<Image> hist_image = std::make_shared<Image>(HIST_H, HIST_W);
    uint total = theImage->n_cols * theImage->n_rows;
    for (uint i = 0 ; i < HIST_W ; ++i) {
        uint val = histogram[i] * HIST_H / total;
        if (val) {
            std::cout << i << " " << histogram[i] << " " << val << std::endl;
        }
        for (uint j = 0 ; j <= val ; ++j) {
            (*hist_image)(HIST_H - j - 1, i) = std::make_tuple(0xFF, 0xFF, 0xFF);
        }
    }
    return hist_image;
    //return getRGBFromYUV(histogram);
}
void ImageProcessor::showObjects() {
    for (uint i = 0 ; i < components ; ++i) {
        drawRectangle(i);
        drawLine(i);
    }
}

void ImageProcessor::drawRectangle(uint num) {
    uint x1 = objects[num].topLeft.x;
    uint x2 = std::min(theImage->n_rows, objects[num].bottomRight.x);
    uint y1 = objects[num].topLeft.y;
    uint y2 = std::min(theImage->n_cols, objects[num].bottomRight.y);
    
    for (uint x = x1 ; x <= x2 ; ++x) {
        (*theImage)(x, y1) = std::make_tuple(0xFF, 0, 0);
        (*theImage)(x, y2) = std::make_tuple(0xFF, 0, 0);
    }
    
    for (uint y = y1 ; y <= y2 ; ++y) {
        (*theImage)(x1, y) = std::make_tuple(0xFF, 0, 0);
        (*theImage)(x2, y) = std::make_tuple(0xFF, 0, 0);
    }
}

std::vector<ImageObject> ImageProcessor::getObjects() const {
    return objects;
}

void ImageProcessor::drawLine(uint num) {
    for (uint x = 0 ; x < theImage->n_cols ; ++x) {
        int y = 0;
        Line l = objects[num].getLineEq();
        if (l.isVertical) {
            y = objects[num].medY;
        }
        else {
            y = x * l.k + l.b;
        }
        if (y >= 0 && y < int(theImage->n_rows)) {
            (*theImage)(y, x) = std::make_tuple(0, 0, 0xFF);
        }
    }
}

void ImageProcessor::drawLineBetweenObjects(uint first, uint second) {
    double y1 = objects[first].medX, x1 = objects[first].medY;
    double y2 = objects[second].medX, x2 = objects[second].medY;
    if (x1 <= x2 + 0.1 && x1 >= x2 - 0.1) {
        for (uint y = std::min(y1, y2) ; y <= std::max(y1, y2) ; ++y) {
            (*theImage)(y, x1) = std::make_tuple(0, 0, 0xFF);
        }
        return;
    }
    double k = (y1 - y2) / (x1 - x2);
    double b = y2 - k *x2;
    for (uint x = std::min(x1, x2) ; x <= std::max(x1, x2) ; ++x) {
        int y = x * k + b;
        if (y >= 0 && y < int(theImage->n_rows)) {
            (*theImage)(y, x) = std::make_tuple(0, 0, 0xFF);
        }
    }
}

