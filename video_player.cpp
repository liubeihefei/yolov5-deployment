#include "video_player.hpp"

video_player::video_player() {}

video_player::~video_player() {}


void video_player::play_video(cv::Mat &image) {
    int key = cv::waitKey(1);
    respondKeyIfVideoContinuing(key);
    if (stop_video) {
        int key = cv::waitKey(0); // 无限制等待
        respondKeyIfVideoStopped(key, image);
    }
}

void video_player::respondKeyIfVideoContinuing(int key)
{
    if (key == 27)// esc键退出
    {
        exit(0);
    }
    else if (key == ' ')// 空格键暂停视频
    {
        stop_video = true;
    }
}

void video_player::respondKeyIfVideoStopped(int key, cv::Mat &img) {
    static int cnt_img = 0;
    if (key == 27)// esc键退出
    {
        exit(0);
    } else if (key == 115)// s键保存视频
    {
        std::cout << "save image" << std::endl;
        std::stringstream sstream;
        sstream.str("");
        sstream << "../save/" << cnt_img << ".jpg";
        std::cout << "sstream" << sstream.str() << std::endl;
        cv::imwrite(sstream.str(), img);
        cnt_img++;
        stop_video = true;
    }  else if (key == ' ')// 空格键暂停视频
    {
        stop_video = true;
    } else if (key == 119)// w 取消暂停视频
    {
        stop_video = false;
    }
}
