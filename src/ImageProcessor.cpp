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



#pragma line ImageObject methods
double ImageObject::getMoment(std::shared_ptr<Matrix<uint>> image, uint i, uint j) {
    double moment = 0;
    for (uint x = topLeft.x ; x <= bottomRight.x ; ++x) {
        for (uint y = topLeft.y ; y <= bottomRight.y ; ++y) {
            if ((*image)(x, y) == num) {
                moment += std::pow(x - medX, i) * std::pow(y - medY, j);
            }
        }
    }
    return moment;
}

void ImageObject::getMeds(std::shared_ptr<Matrix<uint> > image) {
    if (!medsAssigned) {
        uint count = 0;
        medX = 0;
        medY = 0;
        medsAssigned = true;
        for (uint x = topLeft.x ; x <= bottomRight.x ; ++x) {
            for (uint y = topLeft.y ; y <= bottomRight.y ; ++y) {
                if ((*image)(x, y) == num) {
                    //if pixel belongs to object
                    count++;
                    medX += x;
                    medY += y;
                }
            }
        }
        if (!count) return;
        medX /= count;
        medY /= count;
    }

}
double ImageObject::getElongation(std::shared_ptr<Matrix<uint>> image) {
    getMeds(image);
    double m20 = getMoment(image, 2, 0);
    double m02 = getMoment(image, 0, 2);
    double m11 = getMoment(image, 1, 1);
    double tmp = std::sqrt((m20 - m02) * (m20 - m02) + 4 * m11 * m11);
    return (m20 + m02 + tmp) / (m20 + m02 - tmp);
}

double ImageObject::getAngle(std::shared_ptr<Matrix<uint> > image) {
    getMeds(image);
    double m11 = getMoment(image, 1, 1);
    double m02 = getMoment(image, 0, 2);
    double m20 = getMoment(image, 2, 0);
    return std::atan(2 * m11 / (m20 - m02)) / 2;
}

uint ImageObject::getRedPixelsCount(std::shared_ptr<Image> image) {
    uint count = 0, r, g, b;
    for (uint x = topLeft.x ; x <= bottomRight.x ; ++x) {
        for (uint y = topLeft.y ; y <= bottomRight.y ; ++y) {
            std::tie(r, g , b) = (*image)(x, y);
            if (r > g + b) {
                count++;
            }
        }
    }
    return count;
}
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

std::vector<ImageObject> ImageProcessor::getObjects() const {
    return objects;
}

uint ImageProcessor::getRedArrowIndex() {
    uint max = 0, red = 0;
    for (uint i = 0 ; i < objects.size() ; ++i) {
        uint tmp = objects[i].getRedPixelsCount(theImage);
        if (tmp > max && objects[i].getElongation(labelImage) > 3) {
            max = tmp;
            red = i;
        }
    }
    return red;
}
ImageObject ImageProcessor::getTreasure() {
    for (uint i = 0 ; i < objects.size() ; ++i) {
        double val = objects[i].getElongation(labelImage);
        if (val < 3) {
            return objects[i];
        }
    }
    return objects[0];
}

void ImageProcessor::drawLine(uint num) {
    double theta = objects[num].getAngle(labelImage);
    double medY = objects[num].medX;
    double medX = objects[num].medY;
    for (uint x = 0 ; x < theImage->n_cols ; ++x) {
        int y = int((medX - x) * std::tan(theta) + medY);
        if (y >= 0 && y < theImage->n_rows) {
            (*theImage)(y, x) = std::make_tuple(0, 0, 0xFF);
        }
        
    }
}


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

//293 x 50
void ImageProcessor::segment() {
    std::vector<uint> labels(1);
    for (uint i = 0 ; i < grayscaleImage->n_rows ; ++i) {
        for (uint j = 0 ; j < grayscaleImage->n_cols ; ++j) {
            if ( (*binImage)(i,j)) {
                uint up = (i > 0) ? (*labelImage)(i - 1, j) : 0;
                uint left = (j > 0) ? (*labelImage)(i, j - 1) : 0;
                if (!up && !left) {
                    //both are not labeled
                    labels.push_back(uint(labels.size()));
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
            uint tmp = labels[(*labelImage)(i,j)];
            while (labels[tmp] != tmp) {
                tmp = labels[tmp];
            }
            (*labelImage)(i,j) = tmp;
            if ((*labelImage)(i,j) > components) {
                components = (*labelImage)(i,j);
            }
        }
    }
}

void ImageProcessor::parseObjects() {
    std::vector<ImageObject> tmp(components);
    for (uint i = 0 ; i < components ; ++i) {
        tmp[i].topLeft.x = tmp[i].topLeft.y = std::max(theImage->n_cols, theImage->n_rows) + 1;
        tmp[i].medsAssigned = false;
        tmp[i].num = i + 1;
    }
    for (uint i = 0 ; i < labelImage->n_rows ; ++i) {
        for (uint j = 0 ; j < labelImage->n_cols ; ++j) {
            if ((*labelImage)(i, j)) {
                uint component = (*labelImage)(i,j) - 1;
                if (i < tmp[component].topLeft.x) {
                    tmp[component].topLeft.x = i;
                }
                if (j < tmp[component].topLeft.y) {
                    tmp[component].topLeft.y = j;
                }
                if (i > tmp[component].bottomRight.x) {
                    tmp[component].bottomRight.x = i;
                }
                if (j > tmp[component].bottomRight.y) {
                    tmp[component].bottomRight.y = j;
                }
            }
        }
    }
    for (uint i = 0 ; i < components ; ++i) {
        if (tmp[i].getArea() > 600) {
            objects.push_back(tmp[i]);
        }
    }
    components = uint(objects.size());
}

uint ImageProcessor::computeThreshold() const {
    uint maxVal = histogram[0], maxIndex = 0, curSum = 0, sum = theImage->n_cols * theImage->n_rows, maxSum = sum;
    for (uint i = 1 ; i < 256 ; ++i) {
        if (histogram[i] > maxVal) {
            maxVal = histogram[i];
            maxIndex = i;
        }
    }
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
    uint r, g, b;
    std::tie(r, g , b) = (*theImage)(i, j);
    return uint(r*R + g*G + b*B);
}


#pragma debug
//debug methods, including output

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


