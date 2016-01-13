/*
  * Training.h
  *
  *  Created on: Jan 7, 2016
  *      Author: dan
  *

#ifndef SRC_TRAINING_H_
#define SRC_TRAINING_H_

#include <memory>
#include "IFeature.h"

class Training
{
    private:
        std::string picturesDir;
        std::vector< std::unique_ptr<IFeature> > featureList;
    public:
        Training( std::string dir, std::vector< std::unique_ptr<IFeature> >& featureList );
        virtual ~Training();
};
#endif /* SRC_TRAINING_H_ */
