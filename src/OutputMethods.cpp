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
        //std::cout << objects[i].getMoment(labelImage, 1, 1) << " " << objects[i].getMoment(labelImage, 0, 2) << " " << objects[i].getMoment(labelImage, 2, 0) << std::endl;
        //std::cout << objects[i].getAngle(labelImage) << std::endl;
        std::cout << objects[i].greenMedX << " " << objects[i].greenMedY << std::endl;
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
