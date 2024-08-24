import cv2
import os
from pathlib import Path

def create_video_from_images(image_dir, output_video_path, frame_rate=30):
    # Get a list of image file paths
    image_files = sorted(Path(image_dir).glob("*.jpg"))

    # Check if there are images
    if not image_files:
        print("No images found in the directory.")
        return

    # Read the first image to get the width and height
    first_image = cv2.imread(str(image_files[0]))
    height, width, _ = first_image.shape

    # Create a VideoWriter object
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # Codec for .mp4 files
    video_writer = cv2.VideoWriter(output_video_path, fourcc, frame_rate, (width, height))

    for image_file in image_files:
        # Read each image
        image = cv2.imread(str(image_file))
        if image is None:
            print(f"Warning: Could not read image {image_file}. Skipping.")
            continue

        # Write the image to the video
        video_writer.write(image)

    # Release the VideoWriter object
    video_writer.release()

    print(f"Video saved to {output_video_path}")

# Usage
image_directory = "/home/gilberto/Downloads/VisDrone2019-VID-val/sequences/uav0000137_00458_v"
output_video_file = "./assets/vid2.mp4"
create_video_from_images(image_directory, output_video_file)
