/*
  * FeatureAverageColor.h
  *
  *  Created on: Jan 7, 2016
  *      Author: dan
  *

#ifndef SRC_FEATUREAVERAGECOLOR_H_
#define SRC_FEATUREAVERAGECOLOR_H_

#include <opencv2/core.hpp>
#include "IFeature.h"

class FeatureAverageColor : public IFeature
{
    public:
        double calculate( cv::Mat image, std::vector< int > points );

        FeatureAverageColor();
        ~FeatureAverageColor();
};
#endif /* SRC_FEATUREAVERAGECOLOR_H_ */
