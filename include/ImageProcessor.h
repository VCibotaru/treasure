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


class ImageProcessor {
    std::shared_ptr<Image> theImage;
    std::shared_ptr<Matrix<uint>> grayscaleImage;
    std::shared_ptr<Matrix<uint>> binImage;
    std::vector<uint> histogram;
public:
    
    //init the processor with a given image
    ImageProcessor(const std::shared_ptr<Image> &_Image);
    
    uint getPixelIntensity(uint i, uint j) const;
    uint computeThreshold() const;
    std::shared_ptr<Image> getHistogram() const;
    std::shared_ptr<Image> getGrayscale() const;
    std::shared_ptr<Image> getBin() const;
    
    void binarize();
    
//TODO: delete funcs below
    
};



#endif /* defined(__treasure__ImageProcessor__) */
