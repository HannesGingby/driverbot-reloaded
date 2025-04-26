import os
import cv2
import numpy as np
from PIL import Image
import pillow_heif

root_path = "./data"
output_root = "./data/preprocessed"

# root_path = "./data/testing"
# output_root = "./data/testing"

def preprocess(img, output_path):
    # Convert to HSV for better color segmentation
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # Define red color range (red wraps around in HSV, so we need two ranges)
    lower_red1 = np.array([0, 70, 50])
    upper_red1 = np.array([10, 255, 255])

    lower_red2 = np.array([170, 70, 50])
    upper_red2 = np.array([180, 255, 255])

    # Create masks for the red ranges
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)

    # Combine masks
    red_mask = cv2.bitwise_or(mask1, mask2)

    # Optional: Resize for consistency
    red_mask = cv2.resize(red_mask, (320, 240))

    # Invert to make red lines WHITE and everything else BLACK
    bw_output = cv2.threshold(red_mask, 1, 255, cv2.THRESH_BINARY)[1]

    # Save the result
    cv2.imwrite(output_path, bw_output)


def convert_heic_to_jpg(heic_path):
    heif_file = pillow_heif.read_heif(heic_path)
    image = Image.frombytes(
        heif_file.mode,
        heif_file.size,
        heif_file.data
    )
    return cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)

def load_jpg(jpg_path):
    return cv2.imread(jpg_path)

if __name__ == "__main__":
  print("Preprocessing images...")

  for dirpath, dirnames, filenames in os.walk(root_path):
      for filename in filenames:
          if filename.lower().endswith(".jpg"):
              file_path = os.path.join(dirpath, filename)

              relative_path = os.path.relpath(dirpath, root_path)
              output_dir = os.path.join(output_root, relative_path)
              os.makedirs(output_dir, exist_ok=True)

              output_path = os.path.join(output_dir, filename.replace(".jpg", ".jpg"))

              try:
                  #img = convert_heic_to_jpg(file_path)
                  img = load_jpg(file_path)
                  preprocess(img, output_path)
                  print(f"Processed and saved: {output_path}")
              except Exception as e:
                  print(f"Failed to process {file_path}: {e}")
