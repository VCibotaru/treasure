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
#define THRESH_RATIO 0.2
#define THRESH 30


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
    labelImage = std::make_shared<Matrix<uint>>(theImage->n_rows, theImage->n_cols);

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


//let`s try the the gradient variant
void ImageProcessor::binarize() {
    std::shared_ptr<Matrix<uint>> gr = grayscaleImage;
    binImage = std::make_shared<Matrix<uint>>(gr->n_rows, gr->n_cols);
    
    uint threshold = THRESH;//computeThreshold();
    
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            if ((*gr)(i, j) > threshold) {
                (*binImage)(i, j) = 0xFF;
            }
        }
    }
    
}

void ImageProcessor::segment() {
    std::vector<uint> labels(1);
    
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            if ( (*grayscaleImage)(i,j) ) {
                uint up = (i > 0) ? (*labelImage)(i - 1, j) : 0;
                uint left = (j > 0) ? (*labelImage)(i, j - 1) : 0;
                if (!up && !left) {
                    //both are not labeled
                    labels.push_back(labels.size());
                    (*labelImage)(i,j) = labels.back();
                    continue;
                }
                if (up ^ left) {
                    //one is labeled, other is not
                    uint theLabel = (left) ? left : up;
                    (*labelImage)(i,j) = theLabel;
                    continue;
                }
                //both are labeled
                (*labelImage)(i,j) = std::min(up, left);
                labels[up] = std::min(up, left);
                labels[left] = std::min(up, left);
            }
        }
    }
    
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            (*labelImage)(i,j) = labels[(*labelImage)(i,j)];
            if ((*labelImage)(i,j)) {
                (*theImage)(i,j) = std::make_tuple(0xFF, 0xFF, 0);
            }
            else {
                (*theImage)(i,j) = std::make_tuple(0, 0, 0);
            }
            
        }
    }
    
}

uint ImageProcessor::computeThreshold() const {
    uint maxVal = histogram[0], maxIndex = 0, curSum = 0, sum = theImage->n_cols * theImage->n_rows, maxSum = sum;
    for (uint i = 1 ; i < 256 ; ++i) {
        if (histogram[i] > maxVal) {
            maxVal = histogram[i];
            maxIndex = i;
        }
    }
    std::cout << "max = " << maxIndex << std::endl;
    for (uint i = 0 ; i <= maxIndex; ++i) {
        maxSum -= histogram[i];
    }
    curSum = maxSum;
    uint threshold;
    for (threshold = maxIndex + 1 ; threshold < 256 ; ++threshold) {
        curSum -= histogram[threshold];
        if (curSum <= THRESH_RATIO * maxSum) {
            break;
        }
    }
    std::cout << threshold << std::endl;
    return threshold;
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
