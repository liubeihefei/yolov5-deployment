#include "detector.hpp"
#include <time.h>

//构造函数，构建模型
detector::detector(std::string path)
{
    // 创建核心
    ov::Core core;
    // 读取模型
    std::shared_ptr<ov::Model> model = core.read_model(path);
    // 编译模型
    ov::CompiledModel compiled_model = core.compile_model(model, "CPU");
    // 生成推理模型
    infer_request = compiled_model.create_infer_request();
}

//细狗函数
detector::~detector(){};

// 图像遍历回调函数
void detector::pixelCallback(int y, int x, const cv::Vec3b& pixel, float* pixelData)
{
    // 将像素值存储到float*类型的变量中
    const float b = static_cast<float>(pixel[0]);
    const float g = static_cast<float>(pixel[1]);
    const float r = static_cast<float>(pixel[2]);


    // 假设像素数据是按顺序存储的（B, G, R, B, G, R, ...），需要归一化
    const int index = (y * 640 + x) ;
    pixelData[index] = b / 255;
    pixelData[index + 640 * 640] = g / 255;
    pixelData[index + 640 * 640 * 2] = r / 255;
}

//预处理，得到输入tensor，image为输入图像，tensor为转换后的输入张量
void detector::preprocess(cv::Mat& image, ov::Tensor& tensor)
{
    auto data = tensor.data<float>();

    image.forEach<cv::Vec3b>(
            [&](cv::Vec3b & pixel, const int *position)
            {
                const int y = position[0];
                const int x = position[1];
                pixelCallback(y, x, pixel, data);
            });
}

//计算iou
float detector::cal_iou(armor a, armor b)
{
    int ax_min = std::min(std::min(std::min(a.x1, a.x2), a.x3), a.x4);
    int ax_max = std::max(std::max(std::max(a.x1, a.x2), a.x3), a.x4);
    int ay_min = std::min(std::min(std::min(a.y1, a.y2), a.y3), a.y4);
    int ay_max = std::max(std::max(std::max(a.y1, a.y2), a.y3), a.y4);

    int bx_min = std::min(std::min(std::min(b.x1, b.x2), b.x3), b.x4);
    int bx_max = std::max(std::max(std::max(b.x1, b.x2), b.x3), b.x4);
    int by_min = std::min(std::min(std::min(b.y1, b.y2), b.y3), b.y4);
    int by_max = std::max(std::max(std::max(b.y1, b.y2), b.y3), b.y4);

    float area1 = (ax_max - bx_min) * (ay_max - by_min);
    float area2 = (bx_max - ax_min) * (by_max - ay_min);
    if(area1 < area2)
        return area1 / area2;
    else
        return area2 / area1;
}

/*  nms非坤大值抑制
    先按conf_thr筛掉部分框
    然后降序排序把多余框筛掉
    结果放在armors中
*/
void detector::nms(float* result, float conf_thr, float iou_thr, std::vector<armor>& armors)
{
    //遍历result，如果conf大于阈值conf_thr，则放入armors
    for(int i = 0;i < 25200;++i)
    {
        if(result[8 + i * 38] >= conf_thr)
        {
            armor temp;
            //将四个角点放入
            temp.x1 = int((result[0 + i * 38] - padding_x) / scale);   temp.x2 = int((result[2 + i * 38] - padding_x) / scale);
            temp.x3 = int((result[4 + i * 38] - padding_x) / scale);   temp.x4 = int((result[6 + i * 38] - padding_x) / scale);
            temp.y1 = int((result[1 + i * 38] - padding_y) / scale);   temp.y2 = int((result[3 + i * 38] - padding_y) / scale);
            temp.y3 = int((result[5 + i * 38] - padding_y) / scale);   temp.y4 = int((result[7 + i * 38] - padding_y) / scale);

            //找到最大的条件类别概率并乘上conf作为类别概率
            float cls = result[i * 38 + 9];
            int cnt = 0;
            for(int j = i * 38 + 10;j < i * 38 + 38;++j)
            {
                if(cls < result[j])
                    {
                        cls = result[j];
                        cnt = (j - 9) % 38;
                    }
            }
            cls *= result[8 + i * 38];
            temp.score = cls;
            temp.label = cnt;
            armors.push_back(temp);
        }
    }
    
    //对得到的armor按score？进行降序排序（似乎应该按conf，但好像差不多）
    std::sort(armors.begin(), armors.end(), [](armor a, armor b) { return a.score > b.score; });

    //按iou_thr将重合度高的armor进行筛掉
    for(int i = 0;i < int(armors.size());++i)
    {
        for(int j = i + 1;j < int(armors.size());++j)
            //如果与当前的框iou大于阈值则erase掉
            if(cal_iou(armors[i], armors[j]) > iou_thr)
            {
                armors.erase(armors.begin() + j);
                --j;//万年不见--
            }
    }
}

//整个检测流程，目前只有输入图像
void detector::detect(const cv::Mat& image, float conf_thr, float iou_thr, std::vector<armor>& armors)
{
    //不要在原图上操作
    cv::Mat image0 = image;
    //计算缩放大小，padding个数，先缩放再padding
    scale = std::min(640 / image0.cols, 640 / image0.rows);
    padding_y = int((640 - image0.rows * scale) / 2);
    padding_x = int((640 - image0.cols * scale) / 2);
    cv::resize(image0, image0, cv::Size(image0.cols * scale, image0.rows * scale), cv::INTER_LINEAR);
    cv::copyMakeBorder(image0, image0, padding_y, padding_y, padding_x, padding_x, cv::BORDER_CONSTANT, (144, 144, 144));
    //获得输入张量
    ov::Tensor input_tensor = infer_request.get_input_tensor(0);
    preprocess(image0, input_tensor);

    // double dur;
    // clock_t start,end;
    // start = clock();

    infer_request.infer();

    // end = clock();
    // dur = double(end - start) / CLOCKS_PER_SEC * 1000;
    // std::cout << "time: " << dur << std::endl;

    //推理得float*结果
    ov::Tensor output_tensor = infer_request.get_output_tensor(0);
    auto result = output_tensor.data<float>();

   
    //得到最后的装甲板
    nms(result, conf_thr, iou_thr, armors);
    
}
