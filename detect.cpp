#include "detect.h"
#include "mocv_lpr.h"

using namespace std;
using namespace cv;

Mat scale(Mat img) {
    Mat scaled_img;
    Size dsize;
    double factor;
    int major_side = 640;

    // Scale image such as max(width, height) = major_side
    dsize = img.size();
    factor = max(dsize.width, dsize.height) * 1.0 / major_side; 
    resize(img, scaled_img, Size(0, 0), factor, factor, CV_INTER_CUBIC);

    return scaled_img;
}

vector<Rect> extract(Mat img, vector<Rect> plates) {
    double margin = 0.3;
    Point tl, br;
    vector<Rect> extracted_plates;
    vector<Plate> extractions;
    Rect roi;
	Point2f center; 
	Size2f size;
    Mat plate_region;
    DetectRegions detectRegions;
    detectRegions.setFilename("filenamewithoutext");
    detectRegions.saveRegions=false;
    detectRegions.showSteps=false;

    for (int i = 0; i < plates.size(); ++ i) {

        // Add margin to the detected plate region
        tl.x = max(0, (int)(plates[i].x - plates[i].width * margin));
        tl.y = max(0, (int)(plates[i].y - plates[i].height * margin));
        br.x = min(img.size().width, (int)(plates[i].x + plates[i].width *(1 + margin)));
        br.y = min(img.size().height, (int)(plates[i].y + plates[i].height * (1 + margin)));

        // Crop plate region
        plate_region = img(Rect(tl, br));

        // Extract plate(s)
        extractions  = detectRegions.run(plate_region);
        for(int j = 0; j < extractions.size(); ++j) {
            roi = extractions[j].position;
            // Save the first extraction
            if(j == 0 && i == 0) 
                imwrite("extraction_0_0.jpg", extractions[j].plateImg);
            extracted_plates.push_back(Rect(plates[i].x + roi.x - margin * plates[i].width, plates[i].y + roi.y - margin * plates[i].height, roi.br().x - roi.tl().x, roi.br().y - roi.tl().y));
			/*
			RotatedRect rotRect = extractions[i].rotRect;
			rotRect.center.x += plates[i].x - margin * plates[i].width;
			rotRect.center.y += plates[i].y - margin * plates[i].height;
            extracted_plates.push_back(rotRect);
			*/
        }
    }
    return extracted_plates;
}

vector<Rect> detect(Mat img) {
    CascadeClassifier plate_cascade;
    String plate_cascade_name = "models/besthaar/cascade.xml";
    Mat img_gray;
    vector<Rect> plates;
    int size;
    
    // Load detector
    if (!plate_cascade.load(plate_cascade_name)) {
        cout << "error loading model" << endl;
    }

    // Preprocess image
    cvtColor(img, img_gray, COLOR_BGR2GRAY);
    //equalizeHist( img_gray, img_gray );

    // Detect plates
    plate_cascade.detectMultiScale(img_gray, plates, 1.1, 2, 0|CASCADE_SCALE_IMAGE );

    // Group plates
    size = plates.size();
    for (int i = 0; i < size; ++ i)
        plates.push_back(Rect(plates[i]));

    // groupRectangles(plates, 1, 0.2);
    groupRectangles(plates, 1, 0.3);

    // Mastering OpenCV plate extraction 
    return extract(img, plates); 
}

void write_plates_to_file(Mat img, vector<Rect> plates, char name[]) {
    Mat roi, resized_roi;
	double factor;	
    char s[1234];
	
    for (int i = 0; i < plates.size(); i++) {
        sprintf(s, "%s_%d.jpg", name, i);
        roi = img(plates[i]);
		factor = max(roi.size().width, roi.size().height)/300.0;
		resize(roi, resized_roi, Size(0, 0), 1/factor, 1/factor, CV_INTER_CUBIC);
        imwrite(s, resized_roi);
    }
}

void show_detect_result(Mat img, vector<Rect> plates) {
    for (int i = 0; i < plates.size(); i++) {
        Scalar color = Scalar(0, 0, 255);
        rectangle(img, plates[i].tl(), plates[i].br(), color, 2, 8, 0);
    }
    imshow("detect", img);
}

/*
int main(int argc, char* argv[])  {
    Mat img, scaled_img, plate_roi;
    vector<Rect> plates;

    if (argc < 2) 
        cout << "Usage: ./detect img_path" << endl;

    img = imread(argv[1], 1),

    scaled_img = scale(img);

    plates = detect(scaled_img);

    write_plates_to_file(scaled_img, plates, "detect");

    show_detect_result(scaled_img, plates);

    while (true) {
        int c;
        c = waitKey(20);
        if ((char)c == 'q' || (char)c == 'Q')
            break;
    }

    return 0;
}
*/
