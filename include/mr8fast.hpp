#ifndef __MR8FAST_HPP
#define __MR8FAST_HPP
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

class CMR8Fast
{
  private:
    static CMR8Fast *s_instance;

  private:
    CMR8Fast();
    CMR8Fast(CMR8Fast &mr8fast);
    void func1(float *response, float *lengths, float sigma, int size);
    void func2(float *response, float *lengths, int size);
    void func3(float *response, float *lengths, float sigma, int size);
    void normalize(float *response, int size);
    void makeGaussianFilter(float *response, float *lengths, float sigma, int size, int order = 0);
    void getX(float *xCoords, Point2f *pts, int size);
    void getY(float *yCoords, Point2f *pts, int size);
    void multiplyArrays(float *gx, float *gy, float *response, int size);
    void makeFilter(float scale, int phasey, Point2f *pts, float *response, int size);
    void createPointsArray(Point2f *pointsArray, int radius);
    void rotatePoints(float s, float c, Point2f *pointsArray, Point2f *rotatedPointsArray, int size);
    void computeLength(Point2f *pointsArray, float *length, int size);
    void toMat(float *edgeThis, Mat &edgeThisMat, int support);
    void makeRFSfilters(vector<Mat> &edge, vector<Mat> &bar, vector<Mat> &rot, vector<float> &sigmas, int n_orientations = 6, int radius = 24);

  public:
    enum
    {
        FILTER_LENGTH = 8
    };
    ~CMR8Fast();
    static CMR8Fast *getInstance();
    void applyFilterbank(Mat &img, vector<vector<Mat>> filterbank, vector<vector<Mat>> &response, int n_sigmas, int n_orientations)
};
#endif