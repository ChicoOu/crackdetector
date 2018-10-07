#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <memory>
#include <opencv2/opencv.hpp>
using namespace std;

class Util
{
  private:
    static unique_ptr<Util> _instance;
    Util();
    Util(Util &util);
    ~Util();

  public:
    static unique_ptr<Util> &getInstance();
    void mat2IntArray(cv::Mat &mat, uint *width, uint *height, uint32_t **dst);
};
#endif