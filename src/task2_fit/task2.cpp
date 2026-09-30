#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

using namespace cv;
using namespace std;

// ---------- 亚像素圆心：图像矩 + 灰度加权 ----------
Point2f detectCyanCenterMoments(const Mat& bgr, float& radius, double& areaOut) {
    Mat hsv;
    cvtColor(bgr, hsv, COLOR_BGR2HSV);
    Mat mask;
    inRange(hsv, Scalar(80, 80, 80), Scalar(100, 255, 255), mask);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(mask, mask, MORPH_OPEN, kernel);
    Mat maskF;
    mask.convertTo(maskF, CV_32F, 1.0 / 255.0);
    Moments m = moments(maskF, true);
    if (m.m00 < 1e-6) return Point2f(-1, -1);
    Point2f c((float)(m.m10 / m.m00), (float)(m.m01 / m.m00));
    radius = (float)(std::sqrt(std::max(m.mu20 / m.m00, m.mu02 / m.m00) * 2.0));
    areaOut = m.m00;
    return c;
}

// ---------- 模型函数与雅可比（纯 C++ 数组）----------
double modelFunc(double t, const double* p) {
    double A = p[0], b = p[1], Om = p[2], phi = p[3], th0 = p[4];
    return th0 + b * t + (A / Om) * (cos(phi) - cos(Om * t + phi));
}

void jacobianFunc(double t, const double* p, double* jac) {
    double A = p[0], b = p[1], Om = p[2], phi = p[3];
    jac[0] = (cos(phi) - cos(Om * t + phi)) / Om;
    jac[1] = t;
    jac[2] = (A / (Om * Om)) * (cos(Om * t + phi) - cos(phi)) + (A / Om) * sin(Om * t + phi) * t;
    jac[3] = (A / Om) * (-sin(phi) + sin(Om * t + phi));
    jac[4] = 1.0;
}

// ---------- 曲线绘制（仅使用 cv::Mat 进行绘图，绝不做矩阵运算）----------
Mat plotSeries(const vector<double>& x, const vector<double>& obs, const vector<double>& fit, const string& title, const string& ylabel) {
    const int W = 1400, H = 700;
    Mat canvas(H, W, CV_8UC3, Scalar(255, 255, 255));
    double xmin = *min_element(x.begin(), x.end());
    double xmax = *max_element(x.begin(), x.end());
    double ymin = min(*min_element(obs.begin(), obs.end()), *min_element(fit.begin(), fit.end()));
    double ymax = max(*max_element(obs.begin(), obs.end()), *max_element(fit.begin(), fit.end()));
    double dy = ymax - ymin; if (dy < 1e-12) dy = 1.0;
    ymin -= 0.1 * dy; ymax += 0.1 * dy;
    auto mapX = [&](double v) { return int(70 + (v - xmin) / (xmax - xmin + 1e-12) * (W - 140)); };
    auto mapY = [&](double v) { return int(H - 70 - (v - ymin) / (ymax - ymin + 1e-12) * (H - 140)); };
    line(canvas, Point(70, H - 70), Point(W - 70, H - 70), Scalar(0, 0, 0), 1);
    line(canvas, Point(70, 70), Point(70, H - 70), Scalar(0, 0, 0), 1);
    for (size_t i = 0; i < x.size(); ++i)
        circle(canvas, Point(mapX(x[i]), mapY(obs[i])), 2, Scalar(0, 0, 255), -1);
    for (size_t i = 0; i + 1 < x.size(); ++i)
        line(canvas, Point(mapX(x[i]), mapY(fit[i])), Point(mapX(x[i + 1]), mapY(fit[i + 1])), Scalar(0, 128, 0), 2);
    putText(canvas, title, Point(80, 40), FONT_HERSHEY_SIMPLEX, 0.9, Scalar(0, 0, 0), 2);
    putText(canvas, ylabel, Point(80, 70), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 0), 1);
    putText(canvas, "Obs", Point(W - 220, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
    putText(canvas, "Fit", Point(W - 120, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 128, 0), 2);
    return canvas;
}

Mat plotResidual(const vector<double>& x, const vector<double>& res, const string& title) {
    const int W = 1400, H = 700;
    Mat canvas(H, W, CV_8UC3, Scalar(255, 255, 255));
    double xmin = *min_element(x.begin(), x.end());
    double xmax = *max_element(x.begin(), x.end());
    double ymin = *min_element(res.begin(), res.end());
    double ymax = *max_element(res.begin(), res.end());
    double dy = ymax - ymin; if (dy < 1e-12) dy = 1.0;
    ymin -= 0.1 * dy; ymax += 0.1 * dy;
    auto mapX = [&](double v) { return int(70 + (v - xmin) / (xmax - xmin + 1e-12) * (W - 140)); };
    auto mapY = [&](double v) { return int(H - 70 - (v - ymin) / (ymax - ymin + 1e-12) * (H - 140)); };
    line(canvas, Point(70, H - 70), Point(W - 70, H - 70), Scalar(0, 0, 0), 1);
    line(canvas, Point(70, 70), Point(70, H - 70), Scalar(0, 0, 0), 1);
    int y0 = mapY(0.0);
    line(canvas, Point(70, y0), Point(W - 70, y0), Scalar(128, 128, 128), 1);
    for (size_t i = 0; i + 1 < x.size(); ++i)
        line(canvas, Point(mapX(x[i]), mapY(res[i])), Point(mapX(x[i + 1]), mapY(res[i + 1])), Scalar(255, 0, 0), 2);
    putText(canvas, title, Point(80, 40), FONT_HERSHEY_SIMPLEX, 0.9, Scalar(0, 0, 0), 2);
    return canvas;
}

int main(int argc, char** argv) {
    string videoPath = "resources/task_2.mp4";
    string outDir = "result/task2_fit";
    if (argc >= 2) videoPath = argv[1];
    if (argc >= 3) outDir = argv[2];
    (void)system(("mkdir -p " + outDir).c_str());

    VideoCapture cap(videoPath);
    if (!cap.isOpened()) { cerr << "Cannot open video: " << videoPath << endl; return 1; }

    int W = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int H = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(CAP_PROP_FPS);
    if (fps <= 0 || std::isnan(fps)) fps = 60.0;

    VideoWriter writer(outDir + "/tracking_overlay.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(W, H));

    const double cx = 480.0, cy = 360.0;
    vector<double> times, thetaWrapped;
    deque<Point2f> centerBuf; const int MED_WIN = 1;
    Mat frame; int idx = 0; double lastFallbackT = 0.0;

    while (cap.read(frame)) {
        double t = cap.get(CAP_PROP_POS_MSEC) / 1000.0;
        if (t <= 0 || t <= lastFallbackT) t = idx / fps;
        lastFallbackT = t;

        float r = 0; double area = 0;
        Point2f c = detectCyanCenterMoments(frame, r, area);
        bool ok = (c.x >= 0 && c.y >= 0 && area > 20);

        if (ok) {
            centerBuf.push_back(c);
            while ((int)centerBuf.size() > MED_WIN) centerBuf.pop_front();
            Point2f cMed = c;
            if ((int)centerBuf.size() >= 3) {
                vector<float> xs, ys;
                for (auto& p : centerBuf) { xs.push_back(p.x); ys.push_back(p.y); }
                sort(xs.begin(), xs.end()); sort(ys.begin(), ys.end());
                cMed = Point2f(xs[xs.size()/2], ys[ys.size()/2]);
            }
            double theta = atan2(cy - cMed.y, cMed.x - cx);
            times.push_back(t); thetaWrapped.push_back(theta);
            circle(frame, cMed, (int)r, Scalar(0, 255, 0), 2);
            circle(frame, cMed, 3, Scalar(0, 0, 255), -1);
            line(frame, Point2f(cx, cy), cMed, Scalar(0, 255, 255), 2);
            putText(frame, format("theta=%.3f", theta), Point(30, 40), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
        }
        writer.write(frame); ++idx;
    }
    cap.release(); writer.release();

    if (times.size() < 10) { cerr << "Too few valid detections." << endl; return 1; }

    // 角度展开
    vector<double> thetaUnwrap(times.size());
    thetaUnwrap[0] = thetaWrapped[0];
    for (size_t i = 1; i < times.size(); ++i) {
        double d = thetaWrapped[i] - thetaWrapped[i - 1];
        while (d > M_PI) d -= 2.0 * M_PI;
        while (d < -M_PI) d += 2.0 * M_PI;
        thetaUnwrap[i] = thetaUnwrap[i - 1] + d;
    }

    // ================== 纯 C++ 数组 DFT + 线性最小二乘求初值 ==================
    vector<double> omegaT, omegaObs;
    for (size_t i = 1; i + 1 < times.size(); ++i) {
        double dt = times[i + 1] - times[i - 1];
        if (dt <= 0) continue;
        omegaT.push_back(times[i]);
        omegaObs.push_back((thetaUnwrap[i + 1] - thetaUnwrap[i - 1]) / dt);
    }

    double meanW = 0;
    for (double w : omegaObs) meanW += w;
    meanW /= std::max(1.0, (double)omegaObs.size());
    vector<double> sig(omegaObs.size());
    for (size_t i = 0; i < omegaObs.size(); ++i) sig[i] = omegaObs[i] - meanW;

    double bestMag = -1.0, Om_fft = 1.0;
    int M = (int)sig.size();
    for (double Om = 0.1; Om <= 15.0; Om += 0.01) {
        double re = 0, im = 0;
        for (int i = 0; i < M; ++i) {
            double phase = Om * omegaT[i];
            re += sig[i] * cos(phase);
            im -= sig[i] * sin(phase);
        }
        double mag = re*re + im*im;
        if (mag > bestMag) { bestMag = mag; Om_fft = Om; }
    }

    double bestSse = 1e300;
    double Om_best = Om_fft, A_best = 0.1, b_best = meanW, phi_best = 0.0;

    for (double Om = max(0.01, Om_fft - 0.2); Om <= Om_fft + 0.2; Om += 0.005) {
        double S00=0,S01=0,S02=0,S11=0,S12=0,S22=0;
        double T0=0,T1=0,T2=0;
        for (size_t i = 0; i < omegaObs.size(); ++i) {
            double t = omegaT[i];
            double s1 = sin(Om * t), c1 = cos(Om * t);
            S00 += s1*s1; S01 += s1*c1; S02 += s1;
            S11 += c1*c1; S12 += c1;   S22 += 1.0;
            T0 += s1*omegaObs[i]; T1 += c1*omegaObs[i]; T2 += omegaObs[i];
        }
        double A3[3][3] = {{S00,S01,S02},{S01,S11,S12},{S02,S12,S22}};
        double b3[3] = {T0,T1,T2};
        auto det3 = [](double m[3][3]) {
            return m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
                 - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
                 + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
        };
        double D = det3(A3);
        if (fabs(D) < 1e-12) continue;
        double A1[3][3] = {{b3[0],A3[0][1],A3[0][2]},{b3[1],A3[1][1],A3[1][2]},{b3[2],A3[2][1],A3[2][2]}};
        double A2[3][3] = {{A3[0][0],b3[0],A3[0][2]},{A3[1][0],b3[1],A3[1][2]},{A3[2][0],b3[2],A3[2][2]}};
        double A4[3][3] = {{A3[0][0],A3[0][1],b3[0]},{A3[1][0],A3[1][1],b3[1]},{A3[2][0],A3[2][1],b3[2]}};
        double coef[3] = {det3(A1)/D, det3(A2)/D, det3(A4)/D};

        double sse = 0;
        for (size_t i = 0; i < omegaObs.size(); ++i) {
            double pred = coef[0]*sin(Om*omegaT[i]) + coef[1]*cos(Om*omegaT[i]) + coef[2];
            sse += (omegaObs[i] - pred) * (omegaObs[i] - pred);
        }
        if (sse < bestSse) {
            bestSse = sse; Om_best = Om;
            A_best = sqrt(coef[0]*coef[0] + coef[1]*coef[1]);
            phi_best = atan2(coef[1], coef[0]);
            b_best = coef[2];
        }
    }

    double p[5] = {A_best, b_best, Om_best, phi_best, thetaUnwrap[0]};
    if (p[0] < 1e-4) p[0] = 1e-4;
    if (p[1] <= p[0]) p[1] = p[0] + 1e-4;
    if (p[2] < 1e-4) p[2] = 1e-4;
    cout << "纯C++ DFT初值: A=" << p[0] << " b=" << p[1] << " Om=" << p[2] << " phi=" << p[3] << endl;

    // ================== 纯 C++ LM 优化 ==================
    int maxIter = 200;
    double lambda = 1e-3;
    double lastCost = 1e18;

    for (int iter = 0; iter < maxIter; ++iter) {
        double J[2000][5]; double r[2000];
        int N = (int)times.size();
        double cost = 0;

        for (int i = 0; i < N; ++i) {
            double t = times[i];
            double pred = modelFunc(t, p);
            r[i] = thetaUnwrap[i] - pred;
            cost += r[i] * r[i];
            double jac[5];
            jacobianFunc(t, p, jac);
            for (int j = 0; j < 5; ++j) J[i][j] = jac[j];
        }
        if (fabs(lastCost - cost) < 1e-15) break;
        lastCost = cost;

        double H[5][5] = {0}, g[5] = {0};
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < 5; ++j) {
                g[j] += J[i][j] * r[i];
                for (int k = 0; k < 5; ++k) H[j][k] += J[i][j] * J[i][k];
            }
        }

        double H_lm[5][5];
        for (int j = 0; j < 5; ++j)
            for (int k = 0; k < 5; ++k)
                H_lm[j][k] = H[j][k] + (j == k ? lambda : 0.0);

        double delta[5] = {0};
        for (int i = 0; i < 5; ++i) {
            int pivot = i;
            for (int j = i + 1; j < 5; ++j)
                if (fabs(H_lm[j][i]) > fabs(H_lm[pivot][i])) pivot = j;
            if (fabs(H_lm[pivot][i]) < 1e-12) continue;
            if (pivot != i) {
                for (int k = 0; k < 5; ++k) std::swap(H_lm[i][k], H_lm[pivot][k]);
                std::swap(g[i], g[pivot]);
            }
            double div = H_lm[i][i];
            for (int k = i; k < 5; ++k) H_lm[i][k] /= div;
            g[i] /= div;
            for (int j = 0; j < 5; ++j) {
                if (j != i) {
                    double factor = H_lm[j][i];
                    for (int k = i; k < 5; ++k) H_lm[j][k] -= factor * H_lm[i][k];
                    g[j] -= factor * g[i];
                }
            }
        }
        for (int i = 0; i < 5; ++i) delta[i] = g[i];

        double p_new[5];
        for (int i = 0; i < 5; ++i) p_new[i] = p[i] + delta[i];
        if (p_new[0] <= 0) p_new[0] = 1e-5;
        if (p_new[1] <= p_new[0]) p_new[1] = p_new[0] + 1e-5;
        if (p_new[2] <= 0) p_new[2] = 1e-5;

        double newCost = 0;
        for (int i = 0; i < N; ++i) {
            double pred = modelFunc(times[i], p_new);
            double res = thetaUnwrap[i] - pred;
            newCost += res * res;
        }
        if (newCost < cost) {
            for (int i = 0; i < 5; ++i) p[i] = p_new[i];
            lambda *= 0.5;
            if (lambda < 1e-12) lambda = 1e-12;
        } else {
            lambda *= 2.0;
            if (lambda > 1e8) break;
        }
    }

    double A = p[0], b = p[1], Om = p[2], phi = p[3], th0 = p[4];
    while (phi >= M_PI) phi -= 2.0 * M_PI;
    while (phi < -M_PI) phi += 2.0 * M_PI;

    vector<double> thetaFit(times.size());
    for (size_t i = 0; i < times.size(); ++i)
        thetaFit[i] = th0 + b * times[i] + (A / Om) * (cos(phi) - cos(Om * times[i] + phi));

    double angleSse = 0;
    vector<double> angleRes(times.size());
    for (size_t i = 0; i < times.size(); ++i) {
        angleRes[i] = thetaUnwrap[i] - thetaFit[i];
        angleSse += angleRes[i] * angleRes[i];
    }
    double angleRmse = sqrt(angleSse / times.size());

    imwrite(outDir + "/fit_comparison.png", plotSeries(times, thetaUnwrap, thetaFit, "Angle fitting", "theta (rad)"));
    imwrite(outDir + "/residuals.png", plotResidual(times, angleRes, "Angle residual"));

    ofstream md(outDir + "/task2_fit_result.md");
    md << fixed << setprecision(9);
    md << "# Task 2: Synthetic rotating video parameter fitting\n\n";
    md << "## Model\n\n";
    md << "omega(t) = b + A*sin(Omega*t + phi)\n\n";
    md << "theta(t) = theta0 + b*t + (A/Omega)*(cos(phi) - cos(Omega*t + phi))\n\n";
    md << "## Estimated parameters\n\n";
    md << "- A = " << A << " rad/s\n";
    md << "- b = " << b << " rad/s\n";
    md << "- Omega = " << Om << " rad/s\n";
    md << "- phi = " << phi << " rad\n";
    md << "- theta0 = " << th0 << " rad\n";
    md << "- T = 2*pi/Omega = " << 2.0 * M_PI / Om << " s\n\n";
    md << "## Errors\n\n";
    md << "- valid angle samples = " << times.size() << "\n";
    md << "- angle RMSE = " << angleRmse << " rad\n";
    md.close();

    cout << "A=" << A << " b=" << b << " Omega=" << Om << " phi=" << phi << " theta0=" << th0 << "\n";
    cout << "angle RMSE=" << angleRmse << "\n";
    return 0;
}