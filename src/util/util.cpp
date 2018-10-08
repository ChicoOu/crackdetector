#include <util.hpp>
#include <iostream>

shared_ptr<Util> Util::_instance = nullptr;

Util::Util()
{
}

Util::Util(Util &util)
{
}

Util::~Util()
{
}

shared_ptr<Util> &Util::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = make_shared<Util>();
    }

    return _instance;
}

void Util::mat2IntArray(cv::Mat &mat, uint *width, uint *height, uint32_t *dst)
{
    assert(dst != NULL);
    CV_Assert(mat.depth() != sizeof(uchar));
    int channels = mat.channels();
    assert(channels == 3);

    *height = mat.rows;
    *width = mat.cols;
    int nRows = (*height);
    int nCols = (*width);

    int i = 0, j = 0;
    uchar *p = NULL;
    for (i = 0; i < nRows; ++i)
    {
        int base = i * nRows;
        for (j = 0; j < nCols; j++)
        {
            p = mat.ptr<uchar>(i, j);
            int n = base + j;
            // B
            dst[n] = p[0];

            // G
            dst[n] |= (p[1] << 8);

            // R
            dst[n] |= (p[2] << 16);
        }
    }
}