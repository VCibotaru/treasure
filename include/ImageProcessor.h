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

typedef unsigned int uint;

struct Point {
    uint x, y;
    Point(uint _x = 0, uint _y = 0) : x(_x), y(_y) {};
};

struct ImageObject {
    Point topLeft, bottomRight;
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
    
    uint getPixelIntensity(uint i, uint j) const;
    uint computeThreshold() const;
    
    
    //debug methods
    std::shared_ptr<Image> getHistogram() const;
    std::shared_ptr<Image> getGrayscale() const;
    std::shared_ptr<Image> getBin() const;
    std::shared_ptr<Matrix<uint>> getLabeled() const;
    
    
//TODO: delete funcs below
    
};



#endif /* defined(__treasure__ImageProcessor__) */
