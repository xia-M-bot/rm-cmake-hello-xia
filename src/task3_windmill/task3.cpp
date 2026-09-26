#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
using namespace cv;
using namespace std;
// 目标跟踪结构体：保存ID、中心点、丢失计数、状态
struct TrackTarget {
    int id;
    Point2f center;
    int lost_cnt;
    bool is_detected;
    TrackTarget(int id_, Point2f c) : id(id_), center(c), lost_cnt(0), is_detected(true) {}
};
const int MAX_LOST_FRAMES = 15; // 丢失容忍帧数，超过则判定永久丢失
const float MAX_MATCH_DIST = 80.0f; // 匹配最大像素距离
// 轮廓筛选，返回轮廓中心点
vector<Point2f> getTargetCenters(Mat &frame, Mat &bin_out)
{
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);
    // 红色HSV阈值
    Mat mask1, mask2;
    inRange(hsv, Scalar(0,120,100), Scalar(10,255,255), mask1);
    inRange(hsv, Scalar(160,120,100), Scalar(180,255,255), mask2);
    Mat mask = mask1 | mask2;
    // 形态学操作
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
    morphologyEx(mask, mask, MORPH_CLOSE, kernel);
    bin_out = mask.clone();
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<Point2f> centers;
    for(auto &cnt : contours)
    {
        double area = contourArea(cnt);
        if(area < 30 || area > 6000) continue;
        Moments m = moments(cnt);
        if(m.m00 < 1e-6) continue;
        Point2f c(m.m10/m.m00, m.m01/m.m00);
        centers.push_back(c);
    }
    return centers;
}
int main(int argc, char** argv)
{
    if(argc != 4)
    {
        cout << "用法: ./task3 input.mp4 overlay.mp4 binary_process.mp4" << endl;
        return -1;
    }
    string video_path = argv[1];
    string out_path = argv[2];
    string bin_out_path = argv[3];
    VideoCapture cap(video_path);
    if(!cap.isOpened())
    {
        cout << "视频打开失败:" << video_path << endl;
        return -1;
    }
    int w = cap.get(CAP_PROP_FRAME_WIDTH);
    int h = cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(CAP_PROP_FPS);
    VideoWriter writer(out_path, VideoWriter::fourcc('m','p','4','v'), fps, Size(w,h), true);
    VideoWriter writer_bin(bin_out_path, VideoWriter::fourcc('m','p','4','v'), fps, Size(w,h), false);
    vector<TrackTarget> tracks;
    int next_id = 0;
    Mat frame, bin_img;
    while(cap.read(frame))
    {
        vector<Point2f> detect_centers = getTargetCenters(frame, bin_img);
        // 1. 所有现有track丢失计数+1
        writer_bin.write(bin_img);
        for(auto &t : tracks) t.lost_cnt ++;
        // 2. 检测结果与现有轨迹做距离匹配
        vector<bool> matched_det(detect_centers.size(), false);
        for(auto &track : tracks)
        {
            float min_dist = 1e9;
            int best_idx = -1;
            for(int i=0; i<detect_centers.size(); i++)
            {
                if(matched_det[i]) continue;
                float dx = detect_centers[i].x - track.center.x;
                float dy = detect_centers[i].y - track.center.y;
                float dist = sqrt(dx*dx + dy*dy);
                if(dist < MAX_MATCH_DIST && dist < min_dist)
                {
                    min_dist = dist;
                    best_idx = i;
                }
            }
            if(best_idx != -1)
            {
                // 匹配成功，更新目标
                track.center = detect_centers[best_idx];
                track.lost_cnt = 0;
                track.is_detected = true;
                matched_det[best_idx] = true;
            }
            else
            {
                // 没有匹配，标记待丢失
                track.is_detected = false;
            }
        }
        // 3. 新增没有匹配上的检测目标（新建ID）
        for(int i=0; i<detect_centers.size(); i++)
        {
            if(!matched_det[i])
            {
                tracks.emplace_back(next_id++, detect_centers[i]);
            }
        }
        // 4. 删除超过丢失阈值的轨迹
        vector<TrackTarget> new_tracks;
        for(auto &t : tracks)
        {
            if(t.lost_cnt <= MAX_LOST_FRAMES)
                new_tracks.push_back(t);
        }
        tracks.swap(new_tracks);
        // ========== 绘图 ==========
        Mat draw = frame.clone();
        // 绘制二值图窗口（可选）
        imshow("binary", bin_img);
        for(auto &t : tracks)
        {
            Scalar color = t.is_detected ? Scalar(0,255,0) : Scalar(0,0,255);
            circle(draw, t.center, 6, color, -1);
            string state = t.is_detected ? "detected" : "lost";
            putText(draw, "ID:"+to_string(t.id)+" "+state,
                    Point(t.center.x+10, t.center.y),
                    FONT_HERSHEY_SIMPLEX,0.5,color,1);
        }
        writer.write(draw);
        imshow("track", draw);
        if(waitKey(1) == 27) break;
    }
    cap.release();
    writer.release();
    writer_bin.release();
    destroyAllWindows();
    return 0;
}