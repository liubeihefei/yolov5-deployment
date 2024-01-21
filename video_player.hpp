#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <openvino/openvino.hpp>

class video_player {
public:
    video_player();
    ~video_player();
    void play_video(cv::Mat &image);

private:
    bool stop_video = false;
    void respondKeyIfVideoContinuing(int key);
    void respondKeyIfVideoStopped(int key, cv::Mat &img);
};