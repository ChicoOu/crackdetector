#ifndef __MR8FAST_HPP
#define __MR8FAST_HPP
#include <opencv2/opencv.hpp>

class CMR8Fast
{
  public:
    enum
    {
        FILTER_LENGTH = 8
    };
    CMR8Fast();
    CMR8Fast(CMR8Fast &mr8fast);
    ~CMR8Fast();
};
#endif