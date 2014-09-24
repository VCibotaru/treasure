//
//  ImageProcessor.cpp
//  treasure
//
//  Created by Viktor Chibotaru on 9/24/14.
//  Copyright (c) 2014 Viktor Chibotaru. All rights reserved.
//

#include "ImageProcessor.h"

#define R 0.299
#define G 0.587
#define B 0.114
#define HIST_W 256
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



ImageProcessor::ImageProcessor(const std::shared_ptr<Image> &_Image) : histogram(HIST_W) {
    theImage = _Image;
    
    grayscaleImage = std::make_shared<Matrix<uint>>(theImage->n_rows, theImage->n_cols);

    for (uint i = 0 ; i < theImage->n_rows ; ++i) {
        for (uint j = 0 ; j < theImage->n_cols ; ++j) {
            uint val = getPixelIntensity(i, j);
            (*grayscaleImage)(i,j) = val;
            histogram[val]++;
        }
    }
    
    
    
}

std::shared_ptr<Image> ImageProcessor::getGrayscale() const {
    return getRGBFromYUV(grayscaleImage);
}

std::shared_ptr<Image> ImageProcessor::getHistogram() const {
    std::shared_ptr<Image> hist_image = std::make_shared<Image>(HIST_H, HIST_W);
    uint total = theImage->n_cols * theImage->n_rows;
    for (uint i = 0 ; i < HIST_W ; ++i) {
        uint val = uint(float(histogram[i] * HIST_H) / total);
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

uint ImageProcessor::getPixelIntensity(uint i, uint j) const {
    uint r = std::get<0>((*theImage)(i,j));
    uint g = std::get<1>((*theImage)(i,j));
    uint b = std::get<2>((*theImage)(i,j));
    return uint(r*R + g*G + b*B);
}