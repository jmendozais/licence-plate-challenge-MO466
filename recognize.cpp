#include "recognize.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include "mocv_lpr.h"

using namespace std;
using namespace cv;

void read_set(char list_path[], vector<string> *img_paths, Mat *responses) {
	string line, ann_path, ann;
	ifstream ftrain(list_path);
	Mat img;

	while (ftrain >> line) {
		img = imread(line, 1);

		// filter imgs with area 0
		if (img.size().width == 0 || img.size().height == 0)
			continue;

		// read annotation
		ann_path = line.substr(0, line.size() - 3);
		ann_path += "txt";
		ifstream fin(ann_path.c_str());
		fin >> ann;
		if (ann.size() == 0)
			continue;

		(*img_paths).push_back(line);
		(*responses).push_back((int) ann[0]);
	}
}

Mat normalize(Mat img) {
	Mat gray, resized, centered, roi;
	Moments m;
	Point cmass, offset;
	Size size;
	double factor;

	// convert to gray scale
	if (img.channels() == 3)
		cvtColor(img, gray, CV_BGR2GRAY);
	else
		gray = img;
	gray.convertTo(gray, CV_8UC1);
	gray = 255 - gray;

	// normalize by translating the image to its center of mass
	size = gray.size();
	factor = max(size.width, size.height) * 1.0 / 20;
	resize(gray, resized, Size(0, 0), 1.0/factor, 1.0/factor, INTER_CUBIC);
	m = moments(resized, false);
	cmass = Point(round(m.m10/m.m00), round(m.m01/m.m00));

	// crop
	centered = Mat(50, 50, CV_8UC1, Scalar(0));
	offset = Point(25 - cmass.x, 25 - cmass.y);
	roi = centered(Rect(offset, resized.size()));
	resized.copyTo(roi);

	return centered(Rect(11, 11, 28, 28));
}

Mat deskew(Mat img) {
	Mat skewed;
	Moments m;
	float skew;

	// calculate skew
	m = moments(img, false);
	if (abs(m.mu02) < 1e-2)
		return img;
	skew = m.mu11 * 1.0 / m.mu02;

	// create affine transform matrix
	float aff_[2][3] = {{1, skew, -0.5 * img.rows * skew}, {0, 1, 0}};
	Mat aff(2, 3, CV_32FC1);
	for (int i = 0; i < 2; ++ i)
		for (int j = 0; j < 3; ++ j)
			aff.at<float>(i, j) = aff_[i][j];

	// apply affine transform
	warpAffine(img, skewed, aff, img.size(), WARP_INVERSE_MAP | INTER_CUBIC);

	return skewed;
}

Mat features(Mat img) {
	Mat result, resultf, normalized, deskewed;

	// normalize and deskew char by its principal axis
	normalized = normalize(img);
	deskewed = deskew(normalized);

	// convert result to 1D array
	result = deskewed.clone();
	result = result.reshape(0, 1);
	result.convertTo(resultf, CV_32FC1);

	return resultf;
}

Mat features_from_paths(vector<string> img_paths) {
	Mat img;
	Mat x;

	for (int i = 0; i < img_paths.size(); ++ i) {
		img = imread(img_paths[i], 1);
		/*
		Mat output;
		resize(255 - normalized, output,  Size(100, 100), 0, 0, INTER_CUBIC);
		imwrite(img_paths[i] + ".jpg", output);
		*/
		x.push_back(features(img));
	}

	return x;
}

KNearest read_knn(string fname, int *k) {
	Mat x, y;

	FileStorage fin(fname, FileStorage::READ);
	fin["x"] >> x;
	fin["y"] >> y;
	fin["k"] >> *k;

	KNearest knn(x, y, cv::Mat(), false, *k);

	return knn;
}

void write_knn(string fname, Mat x, Mat y, int k) {
	FileStorage fout(fname, FileStorage::WRITE);
	fout << "x" << x;
	fout << "y" << y;
	fout << "k" << k;
}

string recognize_with_knn(vector<Rect> rois, Mat plate) {
	int k;
    KNearest knn = read_knn("ocr.xml", &k);
    Mat character, feats;
	string result;

    for (int i = 0; i < rois.size(); ++ i) {
        // rois with side < 4 cause segfault
        if (rois[i].width < 4 || rois[i].height < 4 )
            continue;

        character = plate(rois[i]);

        feats = features(character);

		// predict character
        result += (char)knn.find_nearest(feats, k);
    }

	return result;
}

string recognize(vector<Rect> rois, Mat plate) {
	CvSVM svm;
    Mat character, feats;
	string result;

	svm.load("ocr_svm.xml");

    for (int i = 0; i < rois.size(); ++ i) {
        // rois with side < 4 cause segfault
        if (rois[i].width < 4 || rois[i].height < 4 )
            continue;

		// extract character region
        character = plate(rois[i]);

        feats = features(character);

		// scale features to -1, 1 for SVM
		feats /= 128;
		feats -= 1;

        result += (char)svm.predict(feats);
    }

	return result;
}

double evaluate(Mat real, Mat pred) {
	assert(real.rows == pred.rows);

	int *ireal;
	float *fpred;
	double acc;

	ireal = real.ptr<int>(0);
	fpred = pred.ptr<float>(0);
	acc = 0;
	for(int i = 0; i < real.rows; ++ i) {
		acc += (ireal[i] == (int)fpred[i]);
	}

	return acc/real.rows;
}

/*
int main(int argc, char* argv[])  {
    Mat img;
    if (argc < 2) 
        cout << "Usage: ./recognize img_path" << endl;

    img = imread(argv[1], 1);
	
    char c = recognize_char(img);
	//string results = recognize_plate(img);
    cout << "Result: " << c << endl;

    return 0;
}
*/
