## 第四次培训
# 文件结构
.rm_yolo_project
├── all_data
│   ├── images
│   │   └── images
│   └── labels
│       └── labels
├── create_empty_label.py
├── README.md
├── rm_dataset
│   ├── images
│   │   ├── test
│   │   ├── train
│   │   └── val
│   └── labels
│       ├── test
│       ├── train
│       ├── train.cache
│       ├── val
│       └── val.cache
├── rm_data.yaml
├── runs
│   └── detect
│       ├── predict
│       ├── predict-2
│       └── predict-3
├── split_data.py
├── weights
│   └── yolo26n.pt
├── yolo26n.pt
├── yolov8s.pt
├── 夏雨晨.pt
│   ├── yolo26n_train_result
│   └── yolov8s_train_result
└── 第四次培训.odt
rm_yolo_project/rm_dataset/images/train/images为原文件中unlabeled中的文件
rm_yolo_project/rm_dataset/labels/train/labels为上面文件的对应txt文件