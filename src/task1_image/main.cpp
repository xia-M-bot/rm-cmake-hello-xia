#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;
int main()
{
    string img_path ="resources/test_image.jpg";
    Mat src = imread(img_path);
    if(src.empty())
    {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    string out_dir = "result/task1_images/";
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    imwrite(out_dir + "gray_image.jpg", gray);
    Mat hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);
    vector<Mat> hsv_channels;
    split(hsv, hsv_channels);
    imwrite(out_dir + "h.png", hsv_channels[0]);
    imwrite(out_dir + "s.png", hsv_channels[1]);
    imwrite(out_dir + "v.png", hsv_channels[2]);
    Mat blur_img, gauss_img, median_img;
    blur(src, blur_img, Size(5, 5));
    GaussianBlur(src, gauss_img, Size(5, 5), 1.2);
    medianBlur(src, median_img, 5);
    imwrite(out_dir + "mean_filter.png", blur_img);
    imwrite(out_dir + "gaussian_filter.png", gauss_img);
    imwrite(out_dir + "median_filter.png", median_img);
    Mat mask1, mask2, red_mask;
    inRange(hsv, Scalar(0, 120, 80), Scalar(10, 255, 255), mask1);
    inRange(hsv, Scalar(170, 120, 80), Scalar(180, 255, 255), mask2);
    red_mask = mask1 | mask2;
    imwrite(out_dir + "red_mask.png", red_mask);
    Mat erode_det, dilate_dst,
open_dst, close_dst;
    Mat kernel =
    getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(red_mask, erode_det, kernel);
    dilate(red_mask, dilate_dst, kernel);
    morphologyEx(red_mask, open_dst, MORPH_OPEN, kernel);
    morphologyEx(red_mask, close_dst, MORPH_CLOSE, kernel);
    imwrite(out_dir + "erode.png", erode_det);
    imwrite(out_dir + "dilate.png", dilate_dst);
    imwrite(out_dir + "open.png", open_dst);
    imwrite(out_dir + "close.png", close_dst);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(close_dst, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat contour_draw = src.clone();
    double area_thresh = 200;
    for(auto &cnt : contours)
    {
        double area = contourArea(cnt);
        if(area > area_thresh)
        {
            drawContours(contour_draw, vector<vector<Point>>{cnt}, 0, Scalar(0, 255, 0), 2);
        }
    }
    imwrite(out_dir + "contours_boxes.png", contour_draw);
    Mat draw_img = src.clone();
    circle(draw_img, Point(200, 200), 50, Scalar(0, 0, 255), 2);
    rectangle(draw_img, Point(400, 100), Point(500, 200), Scalar(0, 255, 255), 2);
    putText(draw_img, "Tulip", Point(100, 400), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0), 2);
    imwrite(out_dir + "draw.png", draw_img);
    Point2f center(src.cols/2.0, src.rows/2.0);
    Mat rot_mat = getRotationMatrix2D(center, 35, 1.0);
    Mat rot_img;
    warpAffine(src, rot_img, rot_mat, src.size());
    imwrite(out_dir + "rotated_35deg.png", rot_img);
    Rect crop_rect(0,0, src.cols/2, src.rows/2);
    Mat crop_img = src(crop_rect);
    imwrite(out_dir + "crop_top_left.png", crop_img);
    cout << "任务1全部图片已经保存至result/task1_images/" << endl;
    return 0;
}