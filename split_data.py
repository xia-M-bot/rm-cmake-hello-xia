import os
import random
import shutil

# ===================== 修改路径 =====================
src_img_dir = "./all_data/images/images"
src_label_dir = "./all_data/labels/labels"

# 输出三分集目录
train_img_dir = "./rm_dataset/images/train"
val_img_dir = "./rm_dataset/images/val"
test_img_dir = "./rm_dataset/images/test"

train_label_dir = "./rm_dataset/labels/train"
val_label_dir = "./rm_dataset/labels/val"
test_label_dir = "./rm_dataset/labels/test"
# ====================================================

# 创建全部文件夹
os.makedirs(train_img_dir, exist_ok=True)
os.makedirs(val_img_dir, exist_ok=True)
os.makedirs(test_img_dir, exist_ok=True)
os.makedirs(train_label_dir, exist_ok=True)
os.makedirs(val_label_dir, exist_ok=True)
os.makedirs(test_label_dir, exist_ok=True)

# 收集同时拥有图片+txt的样本
img_list = []
for fname in os.listdir(src_img_dir):
    base, ext = os.path.splitext(fname)
    if ext.lower() in [".jpg", ".png", ".jpeg"]:
        txt_path = os.path.join(src_label_dir, base + ".txt")
        if os.path.exists(txt_path):
            img_list.append(fname)

random.shuffle(img_list)
total = len(img_list)
train_end = int(total * 0.8)
val_end = int(total * 0.9)

train_files = img_list[:train_end]
val_files = img_list[train_end:val_end]
test_files = img_list[val_end:]

# 复制train
for fname in train_files:
    base, ext = os.path.splitext(fname)
    shutil.copy2(os.path.join(src_img_dir, fname), os.path.join(train_img_dir, fname))
    shutil.copy2(os.path.join(src_label_dir, base+".txt"), os.path.join(train_label_dir, base+".txt"))

# 复制val
for fname in val_files:
    base, ext = os.path.splitext(fname)
    shutil.copy2(os.path.join(src_img_dir, fname), os.path.join(val_img_dir, fname))
    shutil.copy2(os.path.join(src_label_dir, base+".txt"), os.path.join(val_label_dir, base+".txt"))

# 复制test
for fname in test_files:
    base, ext = os.path.splitext(fname)
    shutil.copy2(os.path.join(src_img_dir, fname), os.path.join(test_img_dir, fname))
    shutil.copy2(os.path.join(src_label_dir, base+".txt"), os.path.join(test_label_dir, base+".txt"))

print(f"总样本数：{total}")
print(f"train训练集：{len(train_files)}")
print(f"val验证集：{len(val_files)}")
print(f"test测试集：{len(test_files)}")

