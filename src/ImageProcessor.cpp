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
std::shared_ptr<Image> ImageProcessor::getBin() const {
    return getRGBFromYUV(binImage);
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

//let`s try the the gradient variant
void ImageProcessor::binarize() {
    std::shared_ptr<Matrix<uint>> gr = grayscaleImage;
    uint sum = 0, expVal = 0, curSum = 0, curExpVal = 0, threshold = 0;
    double sigma, maxSigma = -1, w, m;
    for (uint i = 0 ; i < histogram.size() ; ++i) {
        expVal += i * histogram[i];
        sum += histogram[i];
    }
    
    for (uint i = 0 ; i < histogram.size() ; ++i) {
        curExpVal += i * histogram[i];
        curSum += histogram[i];
        w = double(curSum) / sum;
        m = double(curExpVal) / curSum - double(expVal - curExpVal) / (sum - curSum);
        sigma = w * (1 - w) * m * m;
        if (sigma > maxSigma) {
            maxSigma = sigma;
            threshold = i;
        }
    }
    
    binImage = std::make_shared<Matrix<uint>>(gr->n_rows, gr->n_cols);
    /*uint val1 = 0, val2 = 0;
    for (uint i = 1 ; i < grayscaleImage->n_rows - 1 ; ++i) {
        for (uint j = 1 ; j < grayscaleImage->n_cols - 1 ; ++j) {
            uint m = std::max(abs((*gr)(i + 1, j) - (*gr)(i - 1, j)), abs((*gr)(i, j + 1) - (*gr)(i, j - 1)));
            val1 += m * (*gr)(i, j);
            val2 += m;
        }
    }
     uint t = val1/val2;
     std::cout << t << std::endl;
    */
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            if ((*gr)(i, j) > threshold) {
                (*binImage)(i, j) = 0xFF;
            }
        }
    }
    
}

uint ImageProcessor::getPixelIntensity(uint i, uint j) const {
    uint r = std::get<0>((*theImage)(i,j));
    uint g = std::get<1>((*theImage)(i,j));
    uint b = std::get<2>((*theImage)(i,j));
    
    /*if (r + g + b > 100) {
        std::cout << r << " " << g << " " << b << std::endl;
        std::cout << uint(r*R + g*G + b*B) << std::endl;
    }*/
    
    return uint(r*R + g*G + b*B);
}
