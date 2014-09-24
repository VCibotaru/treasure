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
#include "io.h"

typedef unsigned int uint;


class ImageProcessor {
    std::shared_ptr<Image> theImage;
    
public:
    
    //init the processor with a given image
    ImageProcessor(const std::shared_ptr<Image> &_Image);
    
    //just for testing the io system
    void whitenPixels() const;
    
    //for debugging
    void printPixels() const;
    
};



#endif /* defined(__treasure__ImageProcessor__) */
