//
//  ImageProcessor.cpp
//  treasure
//
//  Created by Viktor Chibotaru on 9/24/14.
//  Copyright (c) 2014 Viktor Chibotaru. All rights reserved.
//

#include "ImageProcessor.h"

ImageProcessor::ImageProcessor(const std::shared_ptr<Image> &_Image) {
    theImage = _Image;
}

void ImageProcessor::whitenPixels() const{
    for (uint i = 0 ; i < theImage->n_rows / 2 ; ++i) {
        for (uint j = 0 ; j < theImage->n_cols / 2 ; ++j) {
            (*theImage)(i,j) = 0;
        }
    }
}

void ImageProcessor::printPixels() const {
    for (uint i = 0 ; i < theImage->n_rows / 2 ; ++i) {
        for (uint j = 0 ; j < theImage->n_cols / 2 ; ++j) {
                std::cout << (*theImage)(i,j);
        }
    }
}