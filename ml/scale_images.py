import os
import cv2
from PIL import Image
import numpy as np

# Method 1: Using OpenCV (recommended for batch processing)
def resize_images_opencv(input_dir, output_dir, target_width=160, target_height=120):
    """
    Resize images using OpenCV
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Process all subdirectories (for class folders)
    for root, dirs, files in os.walk(input_dir):
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp')):
                # Create corresponding output directory structure
                rel_path = os.path.relpath(root, input_dir)
                output_subdir = os.path.join(output_dir, rel_path)
                if not os.path.exists(output_subdir):
                    os.makedirs(output_subdir)

                # Read and resize image
                input_path = os.path.join(root, file)
                img = cv2.imread(input_path)

                if img is not None:
                    # Resize image
                    resized_img = cv2.resize(img, (target_width, target_height),
                                           interpolation=cv2.INTER_LINEAR)

                    # Save resized image
                    output_path = os.path.join(output_subdir, file)
                    cv2.imwrite(output_path, resized_img)
                    print(f"Resized: {input_path} -> {output_path}")

# Method 2: Using PIL (good for quality control)
def resize_images_pil(input_dir, output_dir, target_width=120, target_height=160):
    """
    Resize images using PIL with high-quality resampling
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for root, dirs, files in os.walk(input_dir):
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp')):
                # Create corresponding output directory structure
                rel_path = os.path.relpath(root, input_dir)
                output_subdir = os.path.join(output_dir, rel_path)
                if not os.path.exists(output_subdir):
                    os.makedirs(output_subdir)

                # Read and resize image
                input_path = os.path.join(root, file)
                try:
                    with Image.open(input_path) as img:
                        # Resize using high-quality resampling
                        resized_img = img.resize((target_width, target_height),
                                               Image.Resampling.LANCZOS)

                        # Save resized image
                        output_path = os.path.join(output_subdir, file)
                        resized_img.save(output_path)
                        print(f"Resized: {input_path} -> {output_path}")
                except Exception as e:
                    print(f"Error processing {input_path}: {e}")

# Method 3: Using numpy/cv2 with batch processing for speed
def resize_images_batch(input_dir, output_dir, target_width=120, target_height=160):
    """
    Fast batch resize using OpenCV with optimizations
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    total_files = 0
    processed_files = 0

    # Count total files first
    for root, dirs, files in os.walk(input_dir):
        total_files += len([f for f in files if f.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp'))])

    print(f"Processing {total_files} images...")

    for root, dirs, files in os.walk(input_dir):
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp')):
                # Create output directory structure
                rel_path = os.path.relpath(root, input_dir)
                output_subdir = os.path.join(output_dir, rel_path)
                if not os.path.exists(output_subdir):
                    os.makedirs(output_subdir)

                input_path = os.path.join(root, file)
                output_path = os.path.join(output_subdir, file)

                # Read, resize, and save
                img = cv2.imread(input_path)
                if img is not None:
                    resized_img = cv2.resize(img, (target_width, target_height))
                    cv2.imwrite(output_path, resized_img)
                    processed_files += 1

                    if processed_files % 100 == 0:
                        print(f"Progress: {processed_files}/{total_files} ({processed_files/total_files*100:.1f}%)")

# Usage example
if __name__ == "__main__":
    input_directory = "./data/preprocessedv2"  # Your 240x320 images
    output_directory = "./data/preprocessedv3"  # New 120x160 images

    print("Choose resize method:")
    print("1. OpenCV (fast, good quality)")
    print("2. PIL (high quality, slower)")
    print("3. Batch OpenCV (fastest)")

    choice = input("Enter choice (1-3): ")

    if choice == "1":
        resize_images_opencv(input_directory, output_directory)
    elif choice == "2":
        resize_images_pil(input_directory, output_directory)
    elif choice == "3":
        resize_images_batch(input_directory, output_directory)
    else:
        print("Invalid choice, using OpenCV method...")
        resize_images_opencv(input_directory, output_directory)

    print("Resize complete!")

# Alternative: In-place resize (be careful - this overwrites original files!)
def resize_images_inplace(directory, target_width=120, target_height=160):
    """
    WARNING: This overwrites your original images!
    """
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp')):
                file_path = os.path.join(root, file)
                img = cv2.imread(file_path)
                if img is not None:
                    resized_img = cv2.resize(img, (target_width, target_height))
                    cv2.imwrite(file_path, resized_img)
                    print(f"Resized in-place: {file_path}")

# Quick verification function
def verify_resize(directory):
    """
    Check if all images are the correct size
    """
    sizes = {}
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp')):
                file_path = os.path.join(root, file)
                img = cv2.imread(file_path)
                if img is not None:
                    size = (img.shape[1], img.shape[0])  # width, height
                    sizes[size] = sizes.get(size, 0) + 1

    print("Image sizes found:")
    for size, count in sizes.items():
        print(f"  {size[0]}x{size[1]}: {count} images")
