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

#define THRESH_RATIO 0.2
#define THRESH 30

bool intersects(Line l, Point lu, Point rb) {
    for (uint x = lu.y ; x <= rb.y ; ++x) {
        int y = 0;
        if (l.isVertical) {
            y = l.k;
        }
        else {
            y = x * l.k + l.b;
        }
        if (y >= int(lu.x) && y <= int(rb.x)) {
            return true;
        }
    }
    return false;
}



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

void ImageObject::getMeds(std::shared_ptr<Matrix<uint> > image, std::shared_ptr<Image> rgbImage) {
    if (!medsAssigned) {
        uint count = 0, greenCount = 0;
        medX = medY = 0;
        greenMedX = greenMedY = 0;
        medsAssigned = true;
        for (uint x = topLeft.x ; x <= bottomRight.x ; ++x) {
            for (uint y = topLeft.y ; y <= bottomRight.y ; ++y) {
                if ((*image)(x, y) == num) {
                    //if pixel belongs to object
                    count++;
                    medX += x;
                    medY += y;
                    uint r, g, b;
                    std::tie(r,g,b) = (*rgbImage)(x, y);
                    if (!(r > g + b) && !(r + g + b > 600)) {
                        //if the pixel is not red && is not white => is green or yellow
                        greenMedX += x;
                        greenMedY += y;
                        greenCount++;
                    }
                }
            }
        }
        if (!count) return;
        medX /= count;
        medY /= count;
        greenMedX /= greenCount;
        greenMedY /= greenCount;
    }

}
double ImageObject::getElongation(std::shared_ptr<Matrix<uint>> image) {
    double m20 = getMoment(image, 2, 0);
    double m02 = getMoment(image, 0, 2);
    double m11 = getMoment(image, 1, 1);
    double tmp = std::sqrt((m20 - m02) * (m20 - m02) + 4 * m11 * m11);
    return (m20 + m02 + tmp) / (m20 + m02 - tmp);
}

double ImageObject::getAngle(std::shared_ptr<Matrix<uint> > image) {
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
            drawRectangle(i);
            return objects[i];
        }
    }
    return objects[0];
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
    for (uint i = 0 ; i < objects.size() ; ++i) {
        objects[i].getMeds(labelImage, theImage);
    }
    components = uint(objects.size());
}

std::vector<ImageObject> ImageProcessor::getPath() {
    std::vector<bool> checked(components);
    std::vector<ImageObject> path;
    uint cur = getRedArrowIndex();
    path.push_back(objects[cur]);
    double minDist = 10000000, dist;
    uint best = 0;
    while (objects[cur].getElongation(labelImage) > 3) {
        checked[cur] = true;
        minDist = 10000000;
        best = cur;
        Line l = objects[cur].getLineEq();
        for (uint i = 0 ; i < components ; ++i) {
            if (!checked[i] && intersects(l, objects[i].topLeft, objects[i].bottomRight)) {
                dist = std::pow(std::pow(objects[i].medX - objects[cur].medX, 2) + std::pow(objects[i].medY - objects[cur].medY, 2), 0.5);
                if (dist < minDist) {
                    minDist = dist;
                    best = i;
                }
            }
        }
        if (cur == best) {
            break;
        }
        path.push_back(objects[best]);
        drawLineBetweenObjects(cur, best);
        cur = best;
    }
    drawRectangle(cur);
    return path;
}

Line ImageObject::getLineEq() {
    double mY = medX;
    double mX = medY;
    double gmY = greenMedX;
    double gmX = greenMedY;
    if (gmX <= mX + 0.1 && gmX >= mX - 0.1) {
        return Line(medX);
    }
    double k = (gmY - mY) / (gmX - mX);
    double b = mY - k *mX;
    return Line(k, b);

    
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
    return threshold;
}

uint ImageProcessor::getPixelIntensity(uint i, uint j) const {
    uint r, g, b;
    std::tie(r, g , b) = (*theImage)(i, j);
    return uint(r*R + g*G + b*B);
}





