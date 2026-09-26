#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
    VideoCapture cap("../resources/task_2.mp4");
    if (!cap.isOpened())
    {
        cout << "视频打开失败！" << endl;
        return -1;
    }

    double fps = cap.get(CAP_PROP_FPS);
    Size frameSize(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
    VideoWriter writer("../result/task2_fit/tracking_overlay.mp4",
                       VideoWriter::fourcc('m','p','4','v'), fps, frameSize);

    ofstream fout("../result/task2_fit/time_theta.csv");
    fout << "t,theta\n";

    const int cx = 480;
    const int cy = 360;

    Mat frame, hsv, mask;
    while (cap.read(frame))
    {
        double t = cap.get(CAP_PROP_POS_MSEC) / 1000.0;

        // HSV 青色阈值，提取青色圆点
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        Scalar lower_cyan(70, 100, 100);
        Scalar upper_cyan(100, 255, 255);
        inRange(hsv, lower_cyan, upper_cyan, mask);
        imshow("mask", mask);
        waitKey(1);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        Point2f circle_center;
        bool found = false;
        for (auto &cont : contours)
        {
            double area = contourArea(cont);
            if (area < 50) continue; // 忽略小面积噪声
            Moments m = moments(cont);
            if (m.m00 < 1e-6) continue;
            circle_center.x = m.m10 / m.m00;
            circle_center.y = m.m01 / m.m00;
            found = true;
            break;
        }

        Mat draw = frame.clone();
        circle(draw, Point(cx, cy), 5, Scalar(255,255,255), -1); //旋转中心白点

        double theta = 0.0;
        if(found)
        {
            circle(draw, circle_center, 6, Scalar(0,255,255), -1);
            line(draw, Point(cx,cy), circle_center, Scalar(0,255,255), 2);
            // 公式 theta = atan2(cy - yi, xi - cx)
            theta = atan2(cy - circle_center.y, circle_center.x - cx);
            if(theta < 0) theta += 2 * CV_PI; // 将角度范围调整为 [0, 2π]
            fout << t << "," << theta << "\n";
        }
        putText(draw, format("t=%.2fs theta=%.2f rad", t, theta), Point(20,40),
                FONT_HERSHEY_SIMPLEX,0.6,Scalar(255,255,0),2);

        writer.write(draw);
        imshow("track", draw);
        if(waitKey(1) == 27) break;
    }

    cap.release();
    writer.release();
    fout.close();
    destroyAllWindows();
    cout << "视频跟踪完成,csv已保存!" << endl;
    return 0;
}