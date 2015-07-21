#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

#include "detect.h"
#include "segment.h"
#include "recognize.h"

using namespace std;
using namespace cv;

void show_result(Mat img, Point2f pts[], string str) {
	cout << "Str: " << str << endl;
	for (int i = 0; i < 4; i++)
    	line(img, pts[i], pts[(i+1)%4], Scalar(0, 255, 0));
	imshow("Location", img);
	waitKey();
}

void output(Mat img, Point2f pts[], string str) {
	int offset = 0;
	int mins = 12345;
	Size size = img.size();

	// look for tl point
	for (int i = 0; i < 4; ++ i)
		if(mins > (pts[i].x + pts[i].y)) {
			mins = pts[i].x + pts[i].y;
			offset = i;
		}

	// fix if invalid coordinate
	for (int i = 0; i < 4; ++ i) {
		pts[i].x = max((int)pts[i].x, 0);
		pts[i].y = max((int)pts[i].y, 0);
		pts[i].x = min((int)pts[i].x, size.width);
		pts[i].y = min((int)pts[i].y, size.height);
	}

	// show results
	for (int i = 0; i < 4; ++ i)
		cout << pts[(i+offset)%4].x << "," << pts[(i+offset)%4].y << ",";
	cout << str << endl;
}

// Fix licence characters
char num2alp[1234];
char alp2num[1234];
char alp[1234] = "OIZBASGZBB";
char num[1234] = "0123456789";
char alp2[1234] = "ABDEFGHIJKLMNOPQRSTUVWXYZ";
char num2[1234] = "4808868178000080857000772";

string fix_license(string license) {
	if (license.size() == 7) {
		for (int i = 0; i < 3; ++ i)
			license[i] = num2alp[license[i]];
		for (int i = 3; i < 7; ++ i)
			license[i] = alp2num[license[i]];
	}
	return license;
}

// Sorting results
int score_license(const pair<RotatedRect, string> &a) {
	int score =  7 - a.second.size();
	score *= 10;
	string license = a.second;
	string fixed = fix_license(license);
	for(int i = 0; i < fixed.size(); ++ i)
		score += license[i] != fixed[i];
	return score;
}

int cmp_license(const pair<RotatedRect, string> &a,  \
	const pair<RotatedRect, string> &b) {
	return score_license(a) < score_license(b);
}

int init(int argc, char* argv[], string *img_path, string *model_path) {
	// Init char maps
	for (int i = 0; i < 256; ++ i)
		num2alp[i] = alp2num[i] = (char)i;
	for (int i = 0; i < 10; ++ i)
		num2alp[num[i]] = alp[i];
	for (int i = 0; i < 25; ++ i)
		alp2num[alp2[i]] = num2[i];

    if (argc < 2) {
        cout << "Usage: ./lpdetect img_path [model_list]" << endl;

		return -1;
	}

	*img_path = argv[1];

	if (argc > 2)
		*model_path = argv[2];
	else
		*model_path = "models/latest.txt";

	return 0;
}

int main(int argc, char* argv[]) {
    Mat img, scaled_img, roi, resized_roi;
    vector<RotatedRect> plates;
	vector<Rect> right_plates;
	Point2f pts[4];
	double factor_img, factor_plate;
	string result, model_path, img_path;

	init(argc, argv, &img_path, &model_path);

    img = imread(img_path, 1),

    scaled_img = scale(img, &factor_img);

    plates = detect(scaled_img, model_path);

	// No plates detected
	if (plates.size() == 0) {
		cout << "None" << endl;
		return 0;
	}

	vector<pair<RotatedRect, string> > results;
	// cerr << " # plates: " << plates.size() << endl;

	// Perform recognition for first 5 detections
	int num_plates = min((int)plates.size(), 10);

	for (int i = 0; i < num_plates; ++ i) {
		// Get plate bounding box
		right_plates = get_bounding_rects(scaled_img, plates);

		// Scale plate to be compatible with segmenting algorithm
		roi = scaled_img(right_plates[i]);
		factor_plate = max(roi.size().width, roi.size().height)/500.0;
		resize(roi, resized_roi, Size(0, 0), 1/factor_plate, 1/factor_plate, \
		CV_INTER_CUBIC);

    	// Preprocess
    	Mat preproc = preprocess(resized_roi);

    	// Segment
    	vector<Rect> digits = segment(preproc);

    	//write_digits_to_file(digits, preproc, "test_segment");

		// Recognize
		result = recognize(digits, preproc);
		results.push_back(make_pair(plates[i], result));
	}

	// Sort results by number of chars
	sort(results.begin(), results.end(), cmp_license);

	results[0].first.points(pts);

	// scale to the input size
	for (int i = 0; i < 4; ++ i) {
		pts[i].x *= factor_img;
		pts[i].y *= factor_img;
	}
	if (result.size() == 0)
		result = "UNKNOWN";

	// show_result(img, pts, result);

	output(img, pts, fix_license(result));

    return 0;
}
