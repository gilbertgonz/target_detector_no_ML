#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

float find_median(std::vector<float>& distances) {
    std::sort(distances.begin(), distances.end());
    size_t n = distances.size();

    if (n % 2 == 0) {
        return ((distances[n / 2 - 1] + distances[n / 2 + 1]) / 2);
    } else {
        return (distances[n / 2]);
    }
}

int main() {
    /* Concept:
  X 1. Load target image
  X 2. Extract features
  X 3. Initial find target in image by identifying matching keypoints from the reference image
  X 4. Average of pixels is center (remove outliers)
  X 5. That image then becomes your reference
    6. Reduce ROI for feature detection once you find target in image?
  X 7. Repeat 3-5
    
    # Video stablization?
    # tidy up code
    
    */    

    // Load initial target reference
    cv::Mat img1 = cv::imread("/assets/target1.png", cv::IMREAD_GRAYSCALE);

    // File
    std::string filename = "assets/vid1.mp4";
    cv::VideoCapture cap(filename);

    cv::Mat img2;
    int count = 0;
    int rect_box = 50;
    while (true) {
        cap.read(img2);
        if (img2.empty()) break;

        // Create SIFT detector
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

        // Detect SIFT keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints1, keypoints2;
        cv::Mat descriptors1, descriptors2;
        detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
        detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);

        // BFMatcher
        cv::BFMatcher matcher(cv::NORM_L2); // cv::NORM_HAMMING for ORB
        std::vector<cv::DMatch> matches;
        matcher.match(descriptors1, descriptors2, matches);

        // Find median descriptor distances
        std::vector<float> distances;
        for (const auto& match : matches) {
            distances.push_back(match.distance);
        }
        float median_dist = find_median(distances);

        // Filter outliers
        float maxDist = median_dist / 3.0; // tune
        matches.erase(std::remove_if(matches.begin(), matches.end(), [maxDist](const cv::DMatch& m) {
            return m.distance > maxDist;
        }), matches.end());

        // Skip frame if no matches after filtering
        if (matches.size() == 0) {
            std::cout << "no matches, continuing.." << "\n";
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

        
        if (count > 20) { // testing
            // Update img1 with new reference image
            int x1 = std::max(static_cast<int>(x_avg - rect_box), 0);
            int y1 = std::max(static_cast<int>(y_avg - rect_box), 0);
            int x2 = std::min(static_cast<int>(x_avg + rect_box), img2.cols);
            int y2 = std::min(static_cast<int>(y_avg + rect_box), img2.rows);
            img1 = img2(cv::Range(y1, y2), cv::Range(x1, x2));            
        }

        // Draw estimated bbox
        cv::Rect rect(x_avg - rect_box, y_avg - rect_box, rect_box * 2, rect_box * 2);
        cv::rectangle(img2, rect, cv::Scalar(0, 255, 0));
        
        // Show
        cv::Mat img_matches;
        cv::drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches);
        cv::imshow("matches", img_matches);
        // cv::imshow("img2", img2);
        cv::waitKey(1);

        count += 1;
    }

    cap.release();
    
    return 0;
}