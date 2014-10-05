//
//  ImageProcessor.h
//  treasure
//
//  Created by Viktor Chibotaru on 9/24/14.
//  Copyright (c) 2014 Viktor Chibotaru. All rights reserved.
//

#ifndef __treasure__ImageProcessor__
#define __treasure__ImageProcessor__

#include <memory>
#include <iostream>
#include <vector>
#include "io.h"


#define HIST_W 256

typedef unsigned int uint;

struct Point {
    uint x, y;
    Point(uint _x = 0, uint _y = 0) : x(_x), y(_y) {};
};

struct Line {
    double k, b;
    bool isVertical;
    Line(double _k = 0, double _b = 0, bool _vert = false) : k(_k), b(_b), isVertical(_vert) {};
};

struct ImageObject {
    Point topLeft, bottomRight;
    bool medsAssigned;
    uint num;
    double medX, medY;
    double greenMedX, greenMedY;
    double getMoment(std::shared_ptr<Matrix<uint>> image, uint i, uint j);
    double getElongation(std::shared_ptr<Matrix<uint>> image);
    double getAngle(std::shared_ptr<Matrix<uint> > image);
    void getMeds(std::shared_ptr<Matrix<uint> > image, std::shared_ptr<Image> rgbImage);
    uint getRedPixelsCount(std::shared_ptr<Image> image);
    uint getWidth() {
        return (bottomRight.x > topLeft.x) ? (bottomRight.x - topLeft.x) : 0;
    }
    uint getHeight() {
        return (bottomRight.y > topLeft.y) ? (bottomRight.y - topLeft.y) : 0;
    }
    uint getArea() {
        return getWidth() * getHeight();
    }
    Line getLineEq();
};

class ImageProcessor {
    std::shared_ptr<Image> theImage;
    std::shared_ptr<Matrix<uint>> grayscaleImage;
    std::shared_ptr<Matrix<uint>> binImage;
    std::shared_ptr<Matrix<uint>> labelImage;
    std::vector<uint> histogram;
    std::vector<ImageObject> objects;
    uint components;
public:
    
    //init the processor with a given image
    ImageProcessor(const std::shared_ptr<Image> &_Image);
    
    void binarize();
    void segment();
    void parseObjects();
    void drawRectangle(uint num);
    void showObjects();
    void drawLine(uint num);
    
    uint getRedArrowIndex();
    uint getPixelIntensity(uint i, uint j) const;
    uint computeThreshold() const;
    
    
    std::vector<ImageObject> getObjects() const;
    ImageObject getTreasure();
    //debug methods
    std::shared_ptr<Image> getHistogram() const;
    std::shared_ptr<Image> getGrayscale() const;
    std::shared_ptr<Image> getBin() const;
    std::shared_ptr<Matrix<uint>> getLabeled() const;
    std::vector<ImageObject> getPath();
    
    
//TODO: delete funcs below
    
};



#endif /* defined(__treasure__ImageProcessor__) */
