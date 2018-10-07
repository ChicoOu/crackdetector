#include <mr8fast.hpp>
#include <iostream>
#include <slic.hpp>
#include <util.hpp>

const float PI = (3.1415926);

CMR8Fast *CMR8Fast::s_instance = NULL;

CMR8Fast::CMR8Fast()
{
}

CMR8Fast::CMR8Fast(CMR8Fast &fast)
{
}

CMR8Fast::~CMR8Fast()
{
}

//response = np.exp(-x ** 2 / (2. * sigma ** 2))
void CMR8Fast::func1(float *response, float *lengths, float sigma, int size)
{
    for (int i = 0; i < size; i++)
        response[i] = exp(-lengths[i] * lengths[i] / (2 * sigma * sigma));
}

//response = -response * x
void CMR8Fast::func2(float *response, float *lengths, int size)
{
    for (int i = 0; i < size; i++)
        response[i] = -response[i] * lengths[i];
}

//response = response * (x ** 2 - sigma ** 2)
void CMR8Fast::func3(float *response, float *lengths, float sigma, int size)
{
    for (int i = 0; i < size; i++)
        response[i] = response[i] * (lengths[i] * lengths[i] - sigma * sigma);
}

// response /= np.abs(response).sum()
void CMR8Fast::normalize(float *response, int size)
{
    float summ = 0;
    for (int i = 0; i < size; i++)
        summ += std::abs(response[i]);
    for (int i = 0; i < size; i++)
        response[i] /= summ;
}

void CMR8Fast::makeGaussianFilter(float *response, float *lengths, float sigma, int size, int order)
{
    assert(order <= 2); //, "Only orders up to 2 are supported"

    // compute unnormalized Gaussian response
    func1(response, lengths, sigma, size);
    if (order == 1)
        func2(response, lengths, size);
    else if (order == 2)
        func3(response, lengths, sigma, size);

    normalize(response, size);
}

void CMR8Fast::getX(float *xCoords, Point2f *pts, int size)
{
    for (int i = 0; i < size; i++)
        xCoords[i] = pts[i].x;
}

void CMR8Fast::getY(float *yCoords, Point2f *pts, int size)
{
    for (int i = 0; i < size; i++)
        yCoords[i] = pts[i].y;
}

void CMR8Fast::multiplyArrays(float *gx, float *gy, float *response, int size)
{
    for (int i = 0; i < size; i++)
        response[i] = gx[i] * gy[i];
}

void CMR8Fast::makeFilter(float scale, int phasey, Point2f *pts, float *response, int size)
{
    float xCoords[size];
    float yCoords[size];
    getX(xCoords, pts, size);
    getY(yCoords, pts, size);

    float gx[size];
    float gy[size];
    makeGaussianFilter(gx, xCoords, 3 * scale, size);
    makeGaussianFilter(gy, yCoords, scale, size, phasey);
    multiplyArrays(gx, gy, response, size);
    normalize(response, size);
}

void CMR8Fast::createPointsArray(Point2f *pointsArray, int radius)
{
    int index = 0;
    for (int x = -radius; x <= radius; x++)
        for (int y = -radius; y <= radius; y++)
        {
            pointsArray[index] = Point2f(x, y);
            index++;
        }
}

void CMR8Fast::rotatePoints(float s, float c, Point2f *pointsArray, Point2f *rotatedPointsArray, int size)
{
    for (int i = 0; i < size; i++)
    {
        rotatedPointsArray[i].x = c * pointsArray[i].x - s * pointsArray[i].y;
        rotatedPointsArray[i].y = s * pointsArray[i].x - c * pointsArray[i].y;
    }
}

void CMR8Fast::computeLength(Point2f *pointsArray, float *length, int size)
{
    for (int i = 0; i < size; i++)
        length[i] = sqrt(pointsArray[i].x * pointsArray[i].x + pointsArray[i].y * pointsArray[i].y);
}

void CMR8Fast::toMat(float *edgeThis, Mat &edgeThisMat, int support)
{
    edgeThisMat = Mat::zeros(support, support, CV_32FC1);
    float *nextPts = (float *)edgeThisMat.data;
    for (int i = 0; i < support * support; i++)
    {
        nextPts[i] = edgeThis[i];
    }
}

void CMR8Fast::makeRFSfilters(vector<Mat> &edge, vector<Mat> &bar, vector<Mat> &rot, vector<float> &sigmas, int n_orientations, int radius)
{
    int support = 2 * radius + 1;
    int size = support * support;
    Point2f orgpts[size];
    createPointsArray(orgpts, radius);

    for (uint sigmaIndex = 0; sigmaIndex < sigmas.size(); sigmaIndex++)
        for (int orient = 0; orient < n_orientations; orient++)
        {
            float sigma = sigmas[sigmaIndex];
            //Not 2pi as filters have symmetry
            float angle = PI * orient / n_orientations;
            float c = cos(angle);
            float s = sin(angle);
            Point2f rotpts[size];
            rotatePoints(s, c, orgpts, rotpts, size);
            float edgeThis[size];
            makeFilter(sigma, 1, rotpts, edgeThis, size);
            float barThis[size];
            makeFilter(sigma, 2, rotpts, barThis, size);
            Mat edgeThisMat;
            Mat barThisMat;
            toMat(edgeThis, edgeThisMat, support);
            toMat(barThis, barThisMat, support);

            edge.push_back(edgeThisMat);
            bar.push_back(barThisMat);
        }

    float length[size];
    computeLength(orgpts, length, size);

    float rotThis1[size];
    float rotThis2[size];
    makeGaussianFilter(rotThis1, length, 10, size);
    makeGaussianFilter(rotThis2, length, 10, size, 2);

    Mat rotThis1Mat;
    Mat rotThis2Mat;
    toMat(rotThis1, rotThis1Mat, support);
    toMat(rotThis2, rotThis2Mat, support);
    rot.push_back(rotThis1Mat);
    rot.push_back(rotThis2Mat);
}

void CMR8Fast::applyFilterbank(Mat &img, vector<vector<Mat>> filterbank, vector<vector<Mat>> &response, int n_sigmas, int n_orientations)
{
    response.resize(3);
    vector<Mat> &edges = filterbank[0];
    vector<Mat> &bar = filterbank[1];
    vector<Mat> &rot = filterbank[2];

    // Apply Edge Filters //
    int i = 0;
    for (int sigmaIndex = 0; sigmaIndex < n_sigmas; sigmaIndex++)
    {
        Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
        for (int orient = 0; orient < n_orientations; orient++)
        {
            Mat dst;
            filter2D(img, dst, -1, edges[i], Point(-1, -1), 0, BORDER_DEFAULT);
            newMat = cv::max(dst, newMat);
            i++;
        }
        Mat newMatUchar;
        newMat = cv::abs(newMat);
        newMat.convertTo(newMatUchar, CV_8UC1);
        response[0].push_back(newMatUchar);
    }

    // Apply Bar Filters //
    i = 0;
    for (int sigmaIndex = 0; sigmaIndex < n_sigmas; sigmaIndex++)
    {
        Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
        for (int orient = 0; orient < n_orientations; orient++)
        {
            Mat dst;
            filter2D(img, dst, -1, bar[i], Point(-1, -1), 0, BORDER_DEFAULT);
            newMat = max(dst, newMat);
            i++;
        }
        Mat newMatUchar;
        newMat = cv::abs(newMat);
        newMat.convertTo(newMatUchar, CV_8UC1);
        response[1].push_back(newMatUchar);
    }

    // Apply Gaussian and LoG Filters //
    for (uint i = 0; i < 2; i++)
    {
        Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
        Mat dst;
        filter2D(img, dst, -1, rot[i], Point(-1, -1), 0, BORDER_DEFAULT);
        newMat = max(dst, newMat);
        Mat newMatUchar;
        newMat = cv::abs(newMat);
        newMat.convertTo(newMatUchar, CV_8UC1);
        response[2].push_back(newMatUchar);
    }
    cout << "leaving apply filterbank" << endl;
}

void CMR8Fast::createFilterbank(vector<vector<Mat>> &filterbank, int &n_sigmas, int &n_orientations)
{
    vector<float> sigmas;
    sigmas.push_back(1);
    sigmas.push_back(2);
    sigmas.push_back(4);
    n_sigmas = sigmas.size();
    n_orientations = 6;

    vector<Mat> edge, bar, rot;
    makeRFSfilters(edge, bar, rot, sigmas, n_orientations);

    // Store created filters in fitlerbank 2d vector<Mat>
    filterbank.push_back(edge);
    filterbank.push_back(bar);
    filterbank.push_back(rot);
}

CMR8Fast *CMR8Fast::getInstance()
{
    if (s_instance == NULL)
    {
        s_instance = new CMR8Fast();
    }

    return s_instance;
}

void CMR8Fast::release()
{
    if (s_instance != NULL)
    {
        delete s_instance;
        s_instance = NULL;
    }
}

int main(int argc, char **argv)
{
    cv::Mat img = cv::imread(argv[1], -1);
    if (img.empty())
    {
        return -1;
    }

    uint32_t *dst[img.cols] = new uint32_t[img.cols][img.rows];
    uint width = img.cols;
    uint height = img.rows;
    unique_ptr<Util> pUtil = Util::getInstance();
    pUtil->mat2IntArray(img, &width, &height, dst);

    int *labels = new int[width * height];
    int numlabels(0);
    int nspcounts = 2000;
    int compacts = 40;
    unique_ptr<SLIC> pSlic = make_unique<SLIC>();
    pSlic->DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(dst, width, height, labels, numlabels, nspcounts, compacts);
    cout << "num labels:" << numlabels << endl;
    cout << "labels:" << endl;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            cout << labels[i * width + j] << ",";
        }
        cout << endl;
    }

    cv::namedWindow("Example1", cv::WINDOW_AUTOSIZE);
    cv::imshow("Example1", img);
    cv::waitKey(0);
    cv::destroyWindow("Example1");
    return 0;
}