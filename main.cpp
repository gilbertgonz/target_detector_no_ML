#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

/* Overview:
    1. Load target image
    2. Extract features
    3. Initial find target in image by identifying matching keypoints from the reference image
    4. Average of pixels is center (remove outliers)
    5. That image then becomes your reference (skipping this as its not stable) 
    6. Reduce ROI for feature detection once you find target in image
    7. Repeat 3-6 
*/  

void manual_filter(std::vector<cv::DMatch>& matches) {
    // Get all match distances
    std::vector<float> distances;
    for (const auto& match : matches) {
        distances.push_back(match.distance);
    }

    // Find median descriptor distances
    std::sort(distances.begin(), distances.end());
    size_t n = distances.size();
    float median_dist;
    if (n % 2 == 0) {
        median_dist = ((distances[n / 2 - 1] + distances[n / 2 + 1]) / 2);
    } else {
        median_dist = (distances[n / 2]);
    }

    // Filter outliers
    float max_dist = median_dist / 3.0; // manual tuning
    matches.erase(std::remove_if(matches.begin(), matches.end(), [max_dist](const cv::DMatch& m) {
        return m.distance > max_dist;
    }), matches.end());
}

void ransac_filter(std::vector<cv::DMatch>& matches, std::vector<cv::KeyPoint>& keypoints1, std::vector<cv::KeyPoint>& keypoints2) {
    // Get points from matches
    std::vector<cv::Point2f> points1, points2;
    for (const auto& match : matches) {
        points1.push_back(keypoints1[match.queryIdx].pt);
        points2.push_back(keypoints2[match.trainIdx].pt);
    }

    if (matches.size() < 4) {
        return;  // need at least 4 points for homography
    }

    // Use RANSAC to find homography and filter matches
    std::vector<unsigned char> inliersMask(matches.size());
    cv::Mat homography = cv::findHomography(points1, points2, cv::RANSAC, 3.0, inliersMask);

    // Filter matches based on inliersMask
    std::vector<cv::DMatch> inlier_matches;
    for (size_t i = 0; i < matches.size(); ++i) {
        if (inliersMask[i]) {
            inlier_matches.push_back(matches[i]); // Keep only inliers
        }
    }

    // Update original matches vector with inliers only
    matches = inlier_matches;
}

int main() {  
    // Load initial target reference
    cv::Mat img1 = cv::imread("/assets/target1.png", cv::IMREAD_GRAYSCALE);

    // File
    std::string filename = "assets/vid1.mp4";
    cv::VideoCapture cap(filename);

    // Vars
    cv::Mat img2;
    cv::Mat img2_roi;
    int rect_offset = 50;
    int prev_x_avg;
    int prev_y_avg;
    int delay = 1;
    int frame_number = 0;
    while (true) {
        // Read
        cap.read(img2);
        if (img2.empty()) break;

        // Delay using fps
        float fps = cap.get(cv::CAP_PROP_FPS);
        delay = static_cast<int>(1000.0 / fps);

        // Create SIFT detector
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

        // Detect SIFT keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints1, keypoints2;
        cv::Mat descriptors1, descriptors2;
        detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);

        if (!img2_roi.empty()) {
            detector->detectAndCompute(img2_roi, cv::noArray(), keypoints2, descriptors2);
        } else {
            detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);   
        }
        
        // BFMatcher
        cv::BFMatcher matcher(cv::NORM_L2); // cv::NORM_HAMMING for ORB
        std::vector<cv::DMatch> matches;
        matcher.match(descriptors1, descriptors2, matches);

        // Filter
        manual_filter(matches);
        // ransac_filter(matches, keypoints1, keypoints2); // work in progress

        // Skip if no matches after filtering
        if (matches.empty()) {
            std::cout << "No matches, continuing.." << "\n";
            img2_roi = cv::Mat(); // no roi if no matches (idea for future: implement KF tracking to estimate roi even if no detections)
            cv::imshow("img2", img2);
            cv::waitKey(delay);
            continue;
        }

        // Find average coord of matches
        float x_avg = 0.0;
        float y_avg = 0.0;
        for (const auto& match : matches) {
            const cv::KeyPoint& keypoint = keypoints2[match.trainIdx];
            x_avg += keypoint.pt.x;
            y_avg += keypoint.pt.y;
        }
        x_avg /= matches.size();
        y_avg /= matches.size();
        
        // Adjust x_avg and y_avg relative to the center of img2_roi
        if (!img2_roi.empty()) {
            float diff_x = x_avg - img2_roi.cols/2;
            float diff_y = y_avg - img2_roi.rows/2;

            x_avg = prev_x_avg + diff_x;
            y_avg = prev_y_avg + diff_y;
        }

        // Compute bbox
        int x1 = std::max(static_cast<int>(x_avg - rect_offset), 0);
        int y1 = std::max(static_cast<int>(y_avg - rect_offset), 0);
        int x2 = std::min(static_cast<int>(x_avg + rect_offset), img2.cols);
        int y2 = std::min(static_cast<int>(y_avg + rect_offset), img2.rows);
        
        // New reference image (since reference can change over time) not stable
        // img1 = img2(cv::Range(y1, y2), cv::Range(x1, x2)); 

        // Update detection ROI
        img2_roi = img2(cv::Range(std::max(y1 - rect_offset, 0), std::min(y2 + rect_offset, img2.rows)), 
                        cv::Range(std::max(x1 - rect_offset, 0), std::min(x2 + rect_offset, img2.cols)));
                        
        // Draw bbox
        cv::Rect rect(x_avg - rect_offset, y_avg - rect_offset, rect_offset * 2, rect_offset * 2);
        cv::rectangle(img2, rect, cv::Scalar(0, 255, 0));

        // Show image
        cv::imshow("img2", img2);
        
        std::string frame_filename = "/imgs/frame_" + std::to_string(frame_number) + ".jpg";
        cv::imwrite(frame_filename, img2);
        frame_number++;
        
        // cv::imshow("img1", img1); // for debugging
        if (!img2_roi.empty()) cv::imshow("img2_roi", img2_roi); // for debugging

        cv::waitKey(delay);

        prev_x_avg = x_avg;
        prev_y_avg = y_avg;        
    }

    cap.release();
    
    return 0;
}