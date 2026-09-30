# Task 2: Synthetic rotating video parameter fitting

## Model

omega(t) = b + A*sin(Omega*t + phi)

theta(t) = theta0 + b*t + (A/Omega)*(cos(phi) - cos(Omega*t + phi))

## Estimated parameters

- A = 0.549943585 rad/s
- b = 1.350026230 rad/s
- Omega = 1.649870236 rad/s
- phi = 0.702897260 rad
- theta0 = 0.350314776 rad
- T = 2*pi/Omega = 3.808290599 s

## Errors

- valid angle samples = 1440
- angle RMSE = 0.002342272 rad
