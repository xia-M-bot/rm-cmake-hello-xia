import os

def create_empty_labels(image_folder, label_folder):
    os.makedirs(label_folder, exist_ok=True)

    img_suffix = (".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff")
    created = 0

    for filename in sorted(os.listdir(image_folder)):
        base, ext = os.path.splitext(filename)
        if ext.lower() in img_suffix:
            txt_path = os.path.join(label_folder, base + ".txt")
            if not os.path.exists(txt_path):
                open(txt_path, "w").close()
                created += 1

    print(f"{image_folder} → 新建空txt: {created}")


# ========== 修改为你的实际路径 ==========
image_dir = "/mnt/e_disk/rm_yolo_project/rm_dataset/images/train/images"
label_dir = "/mnt/e_disk/rm_yolo_project/rm_dataset/labels/train"
# =======================================

create_empty_labels(image_dir, label_dir)
