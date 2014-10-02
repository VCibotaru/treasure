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

//83x44
void ImageProcessor::segment() {
    std::vector<uint> labels(1);
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            if ( (*binImage)(i,j)) {
                uint up = (i > 0) ? (*labelImage)(i - 1, j) : 0;
                uint left = (j > 0) ? (*labelImage)(i, j - 1) : 0;
                if (!up && !left) {
                    //both are not labeled
                    labels.push_back(labels.size());
                    (*labelImage)(i,j) = labels.back();
                    continue;
                }
                if ((up && !left) || (!up && left)) {
                    //one is labeled, other is not
                    (*labelImage)(i,j) = (up) ? up : left;
                    continue;
                }
                //both are labeled
                uint val = std::min(labels[up], labels[left]);
                (*labelImage)(i,j) = val;
                labels[up] = val;
                labels[left] = val;
            }
        }
    }
    components = 0;
    for (uint i = 0 ; i < labelImage->n_rows ; ++i) {
        for (uint j = 0 ; j < labelImage->n_cols ; ++j) {
            (*labelImage)(i,j) = labels[(*labelImage)(i,j)];
            if ((*labelImage)(i,j) > components) {
                components = (*labelImage)(i,j);
            }
        }
    }
}

void ImageProcessor::parseObjects() {
    objects.resize(components);
    
    for (uint i = 0 ; i < components ; ++i) {
        objects[i].topLeft.x = objects[i].topLeft.y = std::max(theImage->n_cols, theImage->n_rows) + 1;
    }
    for (uint i = 0 ; i < labelImage->n_rows ; ++i) {
        for (uint j = 0 ; j < labelImage->n_cols ; ++j) {
            if ((*labelImage)(i, j)) {
                uint component = (*labelImage)(i,j) - 1;
                if (i < objects[component].topLeft.x) {
                    objects[component].topLeft.x = i;
                }
                if (j < objects[component].topLeft.y) {
                    objects[component].topLeft.y = j;
                }
                if (i > objects[component].bottomRight.x) {
                    objects[component].bottomRight.x = i;
                }
                if (j > objects[component].bottomRight.y) {
                    objects[component].bottomRight.y = j;
                }
            }
        }
    }
}

void ImageProcessor::showObjects() {
    
    for (uint i = 0 ; i < components ; ++i) {
        drawRectangle(i);
    }
}

void ImageProcessor::drawRectangle(uint num) {
    uint x1 = objects[num].topLeft.x;
    uint x2 = objects[num].bottomRight.x;
    uint y1 = objects[num].topLeft.y;
    uint y2 = objects[num].bottomRight.y;
    
    for (uint x = x1 ; x <= x2 ; ++x) {
            (*theImage)(x, y1) = std::make_tuple(0xFF, 0, 0);
            (*theImage)(x, y2) = std::make_tuple(0xFF, 0, 0);
    }
    
    for (uint y = y1 ; y <= y2 ; ++y) {
        (*theImage)(x1, y) = std::make_tuple(0xFF, 0, 0);
        (*theImage)(x2, y) = std::make_tuple(0xFF, 0, 0);
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
    return uint(r*R + g*G + b*B);
}
