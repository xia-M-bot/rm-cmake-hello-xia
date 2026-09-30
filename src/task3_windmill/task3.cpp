#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

using namespace cv;
using namespace std;

const double PI = acos(-1.0);

struct Params {
    // 改成红色的 HSV 范围
    int red1Low = 0, red1High = 20;
    int red2Low = 160, red2High = 179;
    int redSMin = 40, redVMin = 80;

    // 形态学核
    int morphSize = 3;

    // 几何筛选，专治中心 R 标
    double minArea = 50.0, maxArea = 800.0;
    double minCircularity = 0.10, minRadius = 5.0, maxRadius = 60.0;
    double minDistFromR = 120.0; // 必须大于 120！强制避开中心 R 标！
    double maxDistFromR = 350.0;

    // 锁定与丢失
    int maxLostFrames = 2000; // 永不重选，纯靠光流硬跟
    double matchDist = 300.0;
    double matchAngle = 3.14;
    double matchRadius = 100.0;
    int maxLostRFrames = 30;
};

struct Detection {
    Point2f center;
    float radius = 0;
    double area = 0;
    double circularity = 0;
    double angle = 0;
};

// ---------- RANSAC 圆拟合（只用于精准锁定中心 R 标） ----------
bool ransacCircle(const vector<Point>& contour, Point2f& center, float& radius) {
    vector<Point2f> pts;
    for (const auto& p : contour) pts.push_back(Point2f((float)p.x, (float)p.y));
    if (pts.size() < 10) return false;
    int maxInliers = 0; Point2f bestC; float bestR = 0;
    for (int iter = 0; iter < 60; ++iter) {
        int i1 = rand() % pts.size(), i2 = rand() % pts.size(), i3 = rand() % pts.size();
        if (i1 == i2 || i1 == i3 || i2 == i3) continue;
        Point2f p1 = pts[i1], p2 = pts[i2], p3 = pts[i3];
        double a = p1.x - p2.x, b = p1.y - p2.y, c = p1.x - p3.x, d = p1.y - p3.y;
        double e = ((p1.x*p1.x - p2.x*p2.x) + (p1.y*p1.y - p2.y*p2.y)) / 2.0;
        double f = ((p1.x*p1.x - p3.x*p3.x) + (p1.y*p1.y - p3.y*p3.y)) / 2.0;
        double det = a * d - b * c;
        if (fabs(det) < 1e-6) continue;
        Point2f cCandidate((float)((d * e - b * f) / det), (float)((a * f - c * e) / det));
        float r = norm(cCandidate - p1);
        int inliers = 0;
        for (const auto& pt : pts) if (fabs(norm(pt - cCandidate) - r) < 3.0) inliers++;
        if (inliers > maxInliers) { maxInliers = inliers; bestC = cCandidate; bestR = r; }
    }
    if (maxInliers > pts.size() * 0.3) { center = bestC; radius = bestR; return true; }
    return false;
}

// ---------- 亚像素级中心提取 ----------
Point2f contourSubpixelCenter(const vector<Point>& c) {
    if (c.size() < 5) {
        Moments m = moments(c);
        if (fabs(m.m00) > 1e-9) return Point2f((float)(m.m10/m.m00), (float)(m.m01/m.m00));
        return Point2f(0, 0);
    }
    RotatedRect ell = fitEllipse(c);
    Moments m = moments(c);
    Point2f cW((float)(m.m10/m.m00), (float)(m.m01/m.m00));
    if (norm(ell.center - cW) > 4.0) return cW;
    return 0.5f * ell.center + 0.5f * cW;
}

// ---------- 检测红色 R 标中心（保留，作为旋转中心） ----------
bool detectRCenter(const Mat& hsv, const Mat& gray, const Point2f& prevCenter, bool hasPrev,
                   Point2f& center, float& radius, Mat& redMask, const Params& P) {
    Mat m1, m2;
    inRange(hsv, Scalar(0, 70, 60), Scalar(15, 255, 255), m1);
    inRange(hsv, Scalar(160, 70, 60), Scalar(179, 255, 255), m2);
    bitwise_or(m1, m2, redMask);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(P.morphSize, P.morphSize));
    morphologyEx(redMask, redMask, MORPH_OPEN, kernel);
    morphologyEx(redMask, redMask, MORPH_CLOSE, kernel);
    vector<vector<Point>> contours;
    findContours(redMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return false;
    double bestScore = -1e18; bool found = false;
    for (const auto& c : contours) {
        double area = contourArea(c);
        if (area < 100.0) continue;
        Point2f cCenter; float rEnclose = 0;
        if (!ransacCircle(c, cCenter, rEnclose)) {
            cCenter = contourSubpixelCenter(c);
            Point2f tmp; minEnclosingCircle(c, tmp, rEnclose);
        }
        double per = arcLength(c, true);
        double circ = 4.0 * PI * area / (per * per + 1e-9);
        if (circ < 0.15) continue;
        double score = area;
        if (hasPrev) score -= norm(cCenter - prevCenter) * 30.0;
        if (score > bestScore) { bestScore = score; center = cCenter; radius = rEnclose; found = true; }
    }
    return found;
}

// ---------- 检测青色目标（凌晨3点纯净版，无物理挖空） ----------
vector<Detection> detectTargets(const Mat& hsv, const Point2f& rCenter, const Params& P, Mat& targetMask) {
    // 改为提取红色！
Mat m1, m2;
inRange(hsv, Scalar(P.red1Low, P.redSMin, P.redVMin), Scalar(P.red1High, 255, 255), m1);
inRange(hsv, Scalar(P.red2Low, P.redSMin, P.redVMin), Scalar(P.red2High, 255, 255), m2);
bitwise_or(m1, m2, targetMask);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(P.morphSize, P.morphSize));
    morphologyEx(targetMask, targetMask, MORPH_OPEN, kernel);
    morphologyEx(targetMask, targetMask, MORPH_CLOSE, kernel);
    
    // 注意：这里绝对没有任何“挖空中心”或者“物理环带”的强加限制！
    
    vector<vector<Point>> contours;
    findContours(targetMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<Detection> dets;
    for (const auto& c : contours) {
        double area = contourArea(c);
        if (area < P.minArea || area > P.maxArea) continue;
        Point2f cCenter = contourSubpixelCenter(c);
        Point2f tmp; float rEnclose = 0;
        minEnclosingCircle(c, tmp, rEnclose);
        if (rEnclose < P.minRadius || rEnclose > P.maxRadius) continue;
        double per = arcLength(c, true);
        double circ = 4.0 * PI * area / (per * per + 1e-9);
        if (circ < P.minCircularity) continue;
        double d = norm(cCenter - rCenter);
        if (d < P.minDistFromR || d > P.maxDistFromR) continue;
        Detection det;
        det.center = cCenter; det.radius = rEnclose; det.area = area;
        det.circularity = circ;
        det.angle = atan2(rCenter.y - cCenter.y, cCenter.x - rCenter.x);
        dets.push_back(det);
    }
    sort(dets.begin(), dets.end(), [](const Detection& a, const Detection& b) { return a.area > b.area; });
    if (!dets.empty()) dets.resize(1);
    return dets;
}

// ---------- 锁定管理器（光流+卡尔曼，凌晨3点版） ----------
class LockManager {
public:
    Params P;
    struct Track {
        int id = -1; bool valid = false; int lostFrames = 0; int confirmCount = 0;
        Point2f center; Point2f velocity; float radius = 0; double angle = 0; double lastTime = 0;
    } track;

    KalmanFilter kf; bool kfInit = false; int nextId = 1;
    Mat prevGray;

    enum State { SEARCHING, CONFIRMING, TRACKING, LOST };
    State state = SEARCHING;

    void initKalman() {
        kf.init(4, 2, 0);
        kf.transitionMatrix = (Mat_<float>(4,4) << 1,0,1,0, 0,1,0,1, 0,0,1,0, 0,0,0,1);
        setIdentity(kf.measurementMatrix);
        setIdentity(kf.processNoiseCov, Scalar::all(1e-4));
        setIdentity(kf.measurementNoiseCov, Scalar::all(1e-2));
        setIdentity(kf.errorCovPost, Scalar::all(1));
        kfInit = true;
    }

    void setStateFromKalman(const Point2f& c) {
        if (!kfInit) initKalman();
        kf.statePost.at<float>(0) = c.x; kf.statePost.at<float>(1) = c.y;
        kf.statePost.at<float>(2) = 0;   kf.statePost.at<float>(3) = 0;
    }

    void update(const vector<Detection>& dets, const Mat& gray, double t,
                int& outId, Point2f& outCenter, float& outRadius,
                bool& outDetected, bool& outLost, bool& outReselected, const char*& outState) {
        outDetected = false; outLost = false; outReselected = false;
        if (!track.valid) {
            if (dets.empty()) { outId = -1; outLost = true; state = LOST; outState = "lost"; return; }
            const Detection& d = dets[0];
            track.id = nextId++; track.valid = true; track.lostFrames = 0; track.confirmCount = 1;
            track.center = d.center; track.velocity = Point2f(0,0); track.radius = d.radius; track.angle = d.angle; track.lastTime = t;
            setStateFromKalman(d.center);
            state = CONFIRMING; outId = track.id; outCenter = track.center; outRadius = track.radius;
            outDetected = true; outReselected = true; outState = "confirming"; return;
        }
        double dt = max(1e-3, t - track.lastTime);
        Mat pred = kf.predict();
        Point2f predCenter(pred.at<float>(0), pred.at<float>(1));
        float predRadius = track.radius; double predAngle = track.angle;
        int bestIdx = -1; double bestCost = 1e18;
        for (int i = 0; i < (int)dets.size(); ++i) {
            const Detection& d = dets[i];
            double dist = norm(d.center - predCenter);
            double da = fabs(d.angle - predAngle); while (da > PI) da = fabs(da - 2.0 * PI);
            double dr = fabs(d.radius - predRadius);
            if (dist > P.matchDist || da > P.matchAngle || dr > P.matchRadius) continue;
            double cost = dist / P.matchDist;
            if (cost < bestCost) { bestCost = cost; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            const Detection& d = dets[bestIdx];
            Point2f measured = d.center;
            if (!prevGray.empty() && prevGray.size() == gray.size() && track.lostFrames == 0) {
                vector<Point2f> prevPts = {track.center}; vector<Point2f> nextPts;
                vector<uchar> status; vector<float> err;
                calcOpticalFlowPyrLK(prevGray, gray, prevPts, nextPts, status, err, Size(31,31), 3,
                                     TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 30, 0.01));
                if (!status.empty() && status[0]) measured = 0.6f * d.center + 0.4f * nextPts[0];
            }
            if (bestCost > 0.5) setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));
            else setIdentity(kf.measurementNoiseCov, Scalar::all(1e-3));
            Mat meas = (Mat_<float>(2, 1) << measured.x, measured.y);
            Mat est = kf.correct(meas);
            Point2f fused(est.at<float>(0), est.at<float>(1));
            Point2f newVel = (fused - track.center) / (float)dt;
            track.velocity = 0.7f * track.velocity + 0.3f * newVel;
            track.center = fused; track.radius = 0.7f * track.radius + 0.3f * d.radius;
            track.angle = d.angle; track.lastTime = t; track.lostFrames = 0; track.confirmCount++;
            if (state == SEARCHING || state == LOST || state == CONFIRMING) {
                if (track.confirmCount >= 3) state = TRACKING;
            } else state = TRACKING;
            outId = track.id; outCenter = track.center; outRadius = track.radius;
            outDetected = true; outState = (state == TRACKING) ? "detected" : "confirming"; return;
        }
        track.lostFrames++;
        Point2f predictedByLK = predCenter;
        if (!prevGray.empty() && prevGray.size() == gray.size()) {
            vector<Point2f> prevPts = {track.center}; vector<Point2f> nextPts;
            vector<uchar> status; vector<float> err;
            calcOpticalFlowPyrLK(prevGray, gray, prevPts, nextPts, status, err, Size(51,51), 4,
                                 TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 30, 0.01));
            if (!status.empty() && status[0]) predictedByLK = nextPts[0];
        }
        if (track.lostFrames <= P.maxLostFrames) {
            track.center = predictedByLK; track.radius = predRadius; track.velocity *= 0.95f; track.lastTime = t;
            state = LOST; outId = track.id; outCenter = track.center; outRadius = track.radius;
            outLost = true; outState = "lost"; return;
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 3) { cerr << "Usage: " << argv[0] << " <input_video> <output_overlay.mp4> [binary_output.mp4]\n"; return 1; }
    string inPath = argv[1], outPath = argv[2], binPath = argc >= 4 ? argv[3] : "";
    auto makeDir = [](const string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos != string::npos) (void)system(("mkdir -p " + path.substr(0, pos)).c_str());
    };
    makeDir(outPath); if (!binPath.empty()) makeDir(binPath);
    VideoCapture cap(inPath);
    if (!cap.isOpened()) { cerr << "Cannot open video: " << inPath << "\n"; return 1; }
    int W = (int)cap.get(CAP_PROP_FRAME_WIDTH), H = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(CAP_PROP_FPS);
    if (fps <= 0 || std::isnan(fps)) fps = 30.0;
    VideoWriter writer(outPath, VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(W, H));
    VideoWriter binWriter;
    if (!binPath.empty()) binWriter.open(binPath, VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(W, H));
    Params P; LockManager locker; locker.P = P;
    bool hasR = false; Point2f rCenter; float rRadius = 0; int rLostFrames = 0;
    Mat frame; int frameIdx = 0;
    while (cap.read(frame)) {
        double t = cap.get(CAP_PROP_POS_MSEC) / 1000.0;
        if (t <= 0) t = frameIdx / fps;
        Mat hsv, gray;
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        Mat redMask; Point2f newR; float newRr = 0;
        bool okR = detectRCenter(hsv, gray, rCenter, hasR, newR, newRr, redMask, P);
        if (okR) { rCenter = newR; rRadius = newRr; hasR = true; rLostFrames = 0; }
        else { rLostFrames++; if (rLostFrames > P.maxLostRFrames) hasR = false; }
        Mat targetMask = Mat::zeros(frame.size(), CV_8UC1);
        vector<Detection> dets;
        if (hasR) dets = detectTargets(hsv, rCenter, P, targetMask);
        int outId = -1; Point2f outCenter; float outRadius = 0;
        bool detected = false, lost = false, reselected = false; const char* stateStr = "searching";
        locker.update(dets, gray, t, outId, outCenter, outRadius, detected, lost, reselected, stateStr);
        if (hasR) {
            circle(frame, rCenter, max(3, (int)rRadius), Scalar(0, 0, 255), 2);
            circle(frame, rCenter, 4, Scalar(0, 0, 255), -1);
            line(frame, Point(rCenter.x - 15, rCenter.y), Point(rCenter.x + 15, rCenter.y), Scalar(0, 0, 255), 2);
            line(frame, Point(rCenter.x, rCenter.y - 15), Point(rCenter.x, rCenter.y + 15), Scalar(0, 0, 255), 2);
        }
        if (outId != -1) {
            Scalar color = detected ? Scalar(0, 255, 0) : Scalar(0, 165, 255);
            circle(frame, outCenter, max(3, (int)outRadius), color, 2);
            circle(frame, outCenter, 4, Scalar(255, 0, 0), -1);
            if (hasR) line(frame, rCenter, outCenter, Scalar(0, 255, 255), 2);
            string label = format("ID:%d %s", outId, stateStr);
            if (reselected) label += " reselected";
            putText(frame, label, Point(30, 50), FONT_HERSHEY_SIMPLEX, 1.0, detected ? Scalar(0, 255, 0) : Scalar(0, 0, 255), 2);
        } else {
            putText(frame, "lost", Point(30, 50), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
        }
        putText(frame, format("Frame:%d", frameIdx), Point(30, H - 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
        writer.write(frame);
        if (binWriter.isOpened()) {
            Mat binVis = Mat::zeros(frame.size(), CV_8UC3);
            binVis.setTo(Scalar(0, 0, 255), redMask);
            binVis.setTo(Scalar(0, 255, 0), targetMask);
            for (const auto& d : dets) circle(binVis, d.center, 4, Scalar(255, 255, 255), -1);
            binWriter.write(binVis);
        }
        gray.copyTo(locker.prevGray);
        frameIdx++;
    }
    cap.release(); writer.release(); if (binWriter.isOpened()) binWriter.release();
    return 0;
}