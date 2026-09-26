# RM CMake 图像任务 task1

## 项目介绍
本项目基于OpenCV实现图像读取、形态学操作（腐蚀、膨胀、开运算、闭运算）与轮廓检测，输出处理后的图像结果。

## 环境依赖
- Ubuntu Linux
- CMake >=3.16
- OpenCV 4.x
- GCC/G++

## 编译步骤

```bash
# 1.创建并进入build文件夹
mkdir build
cd build
# 2.cmake配置
cmake ..
# 3.编译
make
# 4.运行程序
./task1_image
```

## 文件结构
```
src
├── task1_image
│   └── main.cpp
resources
└── test_image.jpg
result
├── task1_images
│   ├── close.png
│   ├── contours_boxes.png
│   ├── crop_top_left.png
│   ├── dilate.png
│   ├── draw.png
│   ├── erode.png
│   ├── gaussian_filter.png
│   ├── gray_image.jpg
│   ├── h.png
│   ├── mean_filter.png
│   ├── median_filter.png
│   ├── open.png
│   ├── red_mask.png
│   ├── rotated_35deg.png
│   ├── s.png
│   └── v.png
```

# task2

## 项目介绍
读取视频，识别视频中青色标记圆点，计算标记点相对旋转圆心的转角，采集时间-角度数据；使用Python对采集的数据进行拟合，获取运动参数。

## 环境依赖
-Ubuntu Linux
-CMake >=3.16
-OpenCV 4.x
-GCC/G++
-Python3
-numpy、matplotlib、scipy、pandas

## 编译步骤

```bash
# 1.进入build目录
cd build
# 2.cmake配置
cmake ..
# 3.编译
make
# 4.运行C++程序
./task2_fit
# 5.Python拟合运行
cd ..
python3 fit_task2.py
```

## 文件结构
```
src
├── task2_fit
│   └── task2.cpp
resources
├── task_2.mp4
result
├── task2_fit
│   ├── angular_velocity.png
│   ├── fit_comparison.png
│   ├── residuals.png
│   ├── time_theta.csv
│   └── tracking_overlay.mp4
```

## 原理
# 1.HSV颜色分割
在HSV色彩空间设置阈值筛选青色标记点，得到掩码图像mask。
# 2.轮廓检测
findContours 检测掩码中的外轮廓，通过图像矩 moments 计算标记圆点坐标。
# 3.角度计算
利用 atan2(y_diff,x_diff) 计算标记点极角；将负角度加上2π，统一映射区间
# 4.拟合
scipy.optimize.curve_fit 拟合，求解参数

## 实验结果
A = 0.2169 rad
Ω = 1.9456 rad/s
φ = 1.8562 rad
b = 3.0981 rad
角度RMSE = 1.8040 rad
有效样本数量：1440
帧范围:0 ~ 1439
# 参数估计说明
采用非线性最小二乘优化方法 `scipy.optimize.curve_fit` 进行参数估计。
- **优化方法**：Levenberg-Marquardt（LM）算法
- **模型**：$\theta(t)=A\cos(\Omega t+\varphi)+b$
- **约束**：振幅 $A>0$，角频率 $\Omega>0$；初相位、基线偏移无硬性边界约束
- **初值**：$p_0=[1,\ 2,\ 0,\ 3]$，依次为 $A,\Omega,\varphi,b$ 的初始猜测值
- **求解状态**：优化收敛，成功得到一组最优拟合参数。
# 输出文件
1. `fit_comparison.png`：同一图展示观测散点与角度拟合曲线
2. `residuals.png`：角度残差图
3. `angular_velocity.png`：估计得到的角速度曲线
4. `tracking_overlay.mp4`：带有圆心、标记点、连线识别标记的视频

# task3

## 项目介绍
基于OpenCV实现能量机关视频中灯珠的颜色识别、轮廓检测与多目标追踪，对识别到的灯珠标记ID、绘制轨迹，输出带可视化标注的结果视频。

## 编译步骤

```bash
# 在项目build目录执行
cmake ..
make -j4
```

## 运行命令

```bash
# 运行小能量机关 task3.mp4
./task3 ../resources/task_3.mp4 ../result/task3_windmill/task_3/recognition_overlay.mp4 ../result/task3_windmill/task_3/binary_process.mp4
#运行大能量机关 task4.mp4
./task3 ../resources/task_3.mp4 ../result/task3_windmill/task_4/recognition_overlay.mp4 ../result/task3_windmill/task_4/binary_process.mp4
```

## 文件结构
```
src
└── task3_windmill
    └── task3.cpp
resources
├── task_3.mp4
├── task_4.mp4
result
└── task3_windmill
    ├── task_3
        ├── binary_process.mp4
        └── recognition_overlay.mp4
    └── task_4
        ├── binary_process.mp4
        └── recognition_overlay.mp4
```

## 文件说明
- `recognition_overlay.mp4`：原图叠加灯珠ID、轨迹、检测状态的可视化结果视频
- `binary_process.mp4`：HSV颜色筛选+形态学处理后的二值掩码中间过程视频
# 视频结果
【可视化追踪视频】(./task3_windmill/task_3/recognition_overlay.mp4,./task3_windmill/task_4/recognition_overlay.mp4)
【二值化中间过程视频】(./task3_windmill/task_3/binary_process.mp4,./task3_windmill/task_4/binary_process.mp4)

## 检测方法
# 1.将BGR图像转换至HSV颜色空间，设置青色灯珠的HSV阈值，提取灯珠候选区域，生成二值掩码。
# 2.对掩码进行开、闭形态学运算，去除孤立噪点，填补孔洞。
# 3.查找轮廓，通过轮廓面积、圆形度筛选，过滤非灯珠干扰轮廓，得到有效灯珠中心点坐标。

## 锁定与重选（目标跟踪）规则
# 1. **目标锁定**： 新检测到的灯珠，计算与历史轨迹的欧式距离；距离小于阈值则判定为同一目标，沿用原有ID；超出阈值则分配新ID，新建轨迹。
# 2. **目标丢失判定**： 连续若干帧检测不到该目标，标记为`lost`。
# 3. **重选规则**：丢失后的目标，在后续帧重新出现时，若位置匹配上历史轨迹，则重新恢复原有ID；无法匹配则重建ID。

## 已知失败情况
# 1.灯珠被能量机关叶片遮挡时，轮廓消失，目标短暂丢失，标记`lost`；灯珠重新露出后可重新识别。
# 2.画面反光、杂点噪声会产生误检测，生成临时ID，出现虚假目标。
# 3.多灯珠距离很近时，距离匹配容易混淆ID，发生ID切换。