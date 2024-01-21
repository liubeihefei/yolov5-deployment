#include <openvino/openvino.hpp>
#include <string.h>
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"
#include <algorithm>

typedef struct armor
{
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;
    float score;
    int label;
} armor;


class detector
{
private:
    //推理模型
    ov::InferRequest infer_request;
    //缩放比例，xy方向padding
    float scale;
    int padding_y;
    int padding_x;

public:
    detector(std::string path);

    ~detector();

    void pixelCallback(int y, int x, const cv::Vec3b& pixel, float* pixelData);

    void preprocess(cv::Mat& image, ov::Tensor& tensor);

    float cal_iou(armor a, armor b);

    void nms(float* result, float conf_thr, float iou_thr, std::vector<armor>& armors);

    void detect(const cv::Mat& image, float conf_thr, float iou_thr, std::vector<armor>& armors);
};