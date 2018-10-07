#include <util.hpp>
#include <iostream>

unique_ptr<Util> Util::_instance = nullptr;

Util::Util()
{
}

Util::Util(Util &util)
{
}

Util::~Util()
{
}

unique_ptr<Util> &Util::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = make_unique<Util>();
    }

    return _instance;
}

void Util::mat2IntArray(cv::Mat &mat, uint *width, uint *height, uint32_t **dst)
{
    assert(dst != NULL);
    CV_Assert(mat.depth() != sizeof(uchar));
    int channels = mat.channels();
    *height = mat.rows;
    *width = mat.cols;
    int nRows = (*height);
    int nCols = (*width) * channels;

    int i = 0, j = 0;
    uchar *p = NULL;
    for (i = 0; i < nRows; ++i)
    {
        p = mat.ptr<uchar>(i);
        for (j = 0; j < nCols;)
        {
            // B
            dst[i][j] = p[j];
            j++;
            // G
            dst[i][j] |= (p[j] << 8);
            j++;
            // R
            dst[i][j] |= (p[j] << 16);
        }
    }
}