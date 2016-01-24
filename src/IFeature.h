/*
  * Features.h
  *
  *  Created on: Jan 7, 2016
  *      Author: dan
  *

#ifndef SRC_IFEATURE_H_
#define SRC_IFEATURE_H_


#include <opencv2/core.hpp>

class IFeature
{
    public:
        // pure virtual function
        virtual double calculate( cv::Mat image, std::vector< int > points ) = 0;

    protected:
        virtual ~IFeature() {}
        IFeature(){}
};
#endif /* SRC_IFEATURE_H_ */
