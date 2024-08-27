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
    cv::Mat img2_tmp;
    int count = 0;
    int rect_box = 50;
    int prev_x_avg = 0;
    int prev_y_avg = 0;
    float upd_x_avg;
    float upd_y_avg;
    while (true) {
        cap.read(img2);
        if (img2.empty()) break;

        // Create SIFT detector
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

        // Detect SIFT keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints1, keypoints2;
        cv::Mat descriptors1, descriptors2;
        detector->detectAndCompute(img1, cv::noArray(), keypoints1, descriptors1);
        if (!img2_tmp.empty()) {
            detector->detectAndCompute(img2_tmp, cv::noArray(), keypoints2, descriptors2);
            std::cout << "USING IMG2_TEMP" << "\n"; 
        } else {
            detector->detectAndCompute(img2, cv::noArray(), keypoints2, descriptors2);   
        }
        
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
        float maxDist = median_dist / 3.0; // manual tuning
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
        std::cout << "x_avg: " << x_avg << " y_avg: " << y_avg << "\n";
        
        if (count > 20) { // testing
            // Update img1 with new reference image
            int x1 = std::max(static_cast<int>(x_avg - rect_box), 0);
            int y1 = std::max(static_cast<int>(y_avg - rect_box), 0);
            int x2 = std::min(static_cast<int>(x_avg + rect_box), img2.cols);
            int y2 = std::min(static_cast<int>(y_avg + rect_box), img2.rows);
            // img1 = img2(cv::Range(y1, y2), cv::Range(x1, x2));  

            img2_tmp = img2(cv::Range(std::max(y1 - rect_box, 0), std::min(y2 + rect_box, img2.rows)), 
                            cv::Range(std::max(x1 - rect_box, 0), std::min(x2 + rect_box, img2.cols)));
        
            /* fix logic below, issue is that teh avg pts are scaled down due to new 
            img2_tmp being used for detection that has different resolution than img2;
            thus, "avg" points produce wrong ROI
            */
           
            float mid_pt_x = (x1+x2)/2;
            float mid_pt_y = (y1+y2)/2;

            float diff_x = x_avg - mid_pt_x;
            float diff_y = y_avg - mid_pt_y;

            upd_x_avg = prev_x_avg + diff_x;
            upd_y_avg = prev_y_avg + diff_y;
            std::cout << "upd_x_avg: " << upd_x_avg << " upd_y_avg: " << upd_y_avg << "\n";
        
        }        
        
        // Show        
        if (!img2_tmp.empty()) {
            // Draw estimated bbox
            cv::Rect rect(x_avg - rect_box, y_avg - rect_box, rect_box * 2, rect_box * 2);
            cv::rectangle(img2, rect, cv::Scalar(0, 255, 0));

            cv::Mat img_matches;
            // cv::drawMatches(img1, keypoints1, img2_tmp, keypoints2, matches, img_matches);
            cv::imshow("img2", img2);

            cv::imshow("img2_tmp", img2_tmp);
            cv::waitKey(0);

            prev_x_avg = upd_x_avg;
            prev_y_avg = upd_y_avg;
        }
        
        count += 1;
        
    }

    cap.release();
    
    return 0;
}