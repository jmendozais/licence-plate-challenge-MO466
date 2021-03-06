#include "detect.h"

#include "opencv2/video/video.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "mocv_lpr.h"

using namespace std;
using namespace cv;

Mat scale(Mat img, double* factor) {
    Mat scaled_img;
    Size dsize;
    int major_side = 640;

    // Scale image such as max(width, height) == major_side
	dsize = img.size();
    (*factor) = max(dsize.width, dsize.height) * 1.0 / major_side;
    resize(img, scaled_img, Size(0,0), 1.0/(*factor), 1.0/(*factor), CV_INTER_CUBIC);

    return scaled_img;
}

Rect fix_rect(const Mat &img, Rect rect) {
	Size size = img.size();

	rect.x = max(0, rect.x);
    rect.y = max(0, rect.y);
    rect.width = min(rect.width, size.width - rect.x);
    rect.height = min(rect.height, size.height - rect.y);

	return rect;
}

vector<Rect> get_bounding_rects(Mat img, vector<RotatedRect> rot_rects) {
	vector<Rect> rects;
	Rect rect;

	for (int i = 0; i < rot_rects.size(); ++ i) {
		rect = rot_rects[i].boundingRect();
		rects.push_back(fix_rect(img, rect));
	}

	return rects;
}

vector<RotatedRect> extract(Mat img, vector<Rect> plates) {
    double margin = 0.3;
    Point tl, br;
    vector<RotatedRect> extracted_plates;
    vector<Plate> extractions;
    Rect roi;
	RotatedRect rot_rect;
	Point2f center;
	Size2f size;
    Mat plate_region;
    DetectRegions detectRegions;
    detectRegions.setFilename("filenamewithoutext");
    detectRegions.saveRegions = false;
    detectRegions.showSteps = false;

    for (int i = 0; i < plates.size(); ++ i) {
        // Add margin to the detected plate region
        tl.x = max(0, (int)(plates[i].x - plates[i].width * margin));
        tl.y = max(0, (int)(plates[i].y - plates[i].height * margin));
        br.x = min(img.size().width, (int)(plates[i].x + plates[i].width * \
		(1 + margin)));
        br.y = min(img.size().height, (int)(plates[i].y + plates[i].height * \
		(1 + margin)));

        // Crop plate region
        plate_region = img(Rect(tl, br));

        // Extract plate(s)
        extractions  = detectRegions.run(plate_region);
        for(int j = 0; j < extractions.size(); ++j) {
			// Translate to the image coordinates
			rot_rect = extractions[j].rotRect;
			rot_rect.center.x = rot_rect.center.x + max(0.0, plates[i].x - \
			margin * plates[i].width);
			rot_rect.center.y = rot_rect.center.y + max(0.0, plates[i].y - \
			margin * plates[i].height);
            extracted_plates.push_back(rot_rect);
        }
    }

    return extracted_plates;
}

vector<RotatedRect> detect(Mat img, String models_path) {
    vector<CascadeClassifier> models;
    Mat img_gray;
    vector<Rect> plates;
	int size;
	string line;

	// Get detectors path
	ifstream fin(models_path.c_str());

	// Load models
	while(fin >> line) {
		CascadeClassifier model;
    	if (!model.load(line))
        	cerr << "error loading model" << endl;
    	else
			models.push_back(model);
	}

    // Preprocess image
	// TODO: to hsv, hist eq
    cvtColor(img, img_gray, COLOR_BGR2GRAY);

    // Detect plates
	for (int i = 0; i < models.size(); ++ i) {
		vector<Rect> tmp;
    	models[i].detectMultiScale(img_gray, tmp, 1.1, 2, 0|CASCADE_SCALE_IMAGE);
		plates.insert(plates.end(), tmp.begin(), tmp.end());
	}

	if (plates.size() == 0)
		return vector<RotatedRect>();

    // Group plates
	size = plates.size();
    for (int i = 0; i < size; ++ i)
        plates.push_back(Rect(plates[i]));
    groupRectangles(plates, 1, 0.3);

    // Extraction
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
	Rect rect;
	Scalar color;
    for (int i = 0; i < plates.size(); i++) {
        color = Scalar(0, 0, 255);
		rect = plates[i];
        rectangle(img, rect.tl(), rect.br(), color, 2, 8, 0);
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
