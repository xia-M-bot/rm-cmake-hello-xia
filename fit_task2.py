import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
# 简谐模型
def model_theta(t, A, Omega, phi, b):
    return A * np.cos(Omega * t + phi) + b
# 读取task2数据
df = pd.read_csv("result/task2_fit/time_theta.csv")
t = df["t"].values
theta_meas = df["theta"].values
# 拟合，初值和作业对应
p0 = [1, 2, 0, 3]
popt, pcov = curve_fit(model_theta, t, theta_meas, p0=p0)
A, Omega, phi, b = popt
theta_fit = model_theta(t, *popt)
# 角度残差、角度RMSE
residual_theta = theta_meas - theta_fit
rmse_theta = np.sqrt(np.mean(residual_theta**2))
# 解析角速度：dθ/dt = -A*Ω*sin(Ω*t + φ)
omega_fit = -A * Omega * np.sin(Omega * t + phi)
print("====拟合参数====")
print(f"A = {A:.4f} rad")
print(f"Ω = {Omega:.4f} rad/s")
print(f"φ = {phi:.4f} rad")
print(f"b = {b:.4f} rad")
print(f"角度RMSE = {rmse_theta:.4f} rad")
print(f"有效样本数量：{len(t)}")
print(f"帧范围:0 ~ {len(t)-1}")
# 图1：角度对比图
plt.figure(figsize=(12,5))
plt.plot(t, theta_meas, label="观测点", alpha=0.6)
plt.plot(t, theta_fit, "r", label="拟合曲线", linewidth=2)
plt.xlabel("t / s")
plt.ylabel("$\\theta$ / rad")
plt.grid(True)
plt.legend()
plt.title("角度观测值与拟合曲线对比")
plt.savefig("result/task2_fit/fit_comparison.png", dpi=300)
# 图2：残差图
plt.figure(figsize=(12,4))
plt.plot(t, residual_theta, 'g')
plt.xlabel("t / s")
plt.ylabel("角度残差 / rad")
plt.grid(True)
plt.title("角度拟合残差")
plt.savefig("result/task2_fit/residuals.png", dpi=300)
# 图3：角速度曲线
plt.figure(figsize=(12,5))
plt.plot(t, omega_fit, "r", label="拟合角速度")
plt.xlabel("t / s")
plt.ylabel("$\\omega$ / rad/s")
plt.grid(True)
plt.legend()
plt.title("角速度曲线")
plt.savefig("result/task2_fit/angular_velocity.png", dpi=300)
plt.show()