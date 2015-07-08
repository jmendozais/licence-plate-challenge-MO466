#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include "mocv_lpr.h"

using namespace std;
using namespace cv;

char recognize_char(Mat char_img) {
	Mat img_gray, rz_img;

    OCR ocr("OCR.xml");    
    ocr.saveSegments=true;
    ocr.DEBUG=false;
    ocr.filename="ocr_filenamewithoutext";
	//cvtColor(char_img, img_gray, CV_BGR2GRAY);
	img_gray = 255 - char_img;

    return ocr.run_on_char(img_gray);
}

string recognize_plate(Mat plate_img) {
	Mat plate_gray;
	Plate plate;

    OCR ocr("OCR.xml");    
    ocr.saveSegments=true;
    ocr.DEBUG=false;
    ocr.filename="ocr_filenamewithoutext";
	cvtColor(plate_img, plate_gray, CV_BGR2GRAY);
	plate_gray.convertTo(plate_gray, CV_8UC1);
	imshow("converted", plate_gray);
	waitKey();

	plate.plateImg = plate_gray;
    return ocr.run(&plate);
}

void read_set(char list_path[], vector<string> &img_paths, Mat &responses) {
	string line, ann_path, ann;
	ifstream ftrain(list_path);
	Mat img;

	while (ftrain >> line) {
		cout << "img: " << line << endl;
		img = imread(line, 1);

		// filtering imgs with area 0
		if (img.size().width == 0 || img.size().height == 0)
			continue;

		ann_path = line.substr(0, line.size() - 3);
		ann_path += "txt";
		ifstream fin(ann_path.c_str());
		fin >> ann;
		if (ann.size() == 0) 
			continue;

		img_paths.push_back(line);
		responses.push_back((int) ann[0]);
	}
}

Mat normalize(Mat img) {
	Mat gray, resized, centered, roi;
	Moments m;
	Point cmass, offset;
	Size size;
	double factor;

	if (img.channels() == 3)
		cvtColor(img, gray, CV_BGR2GRAY);
	else
		gray = img;
	gray.convertTo(gray, CV_8UC1);
	gray = 255 - gray;

	size = gray.size();
	factor = max(size.width, size.height) * 1.0 / 20;
	resize(gray, resized, Size(0,0), 1.0/factor, 1.0/factor, INTER_CUBIC);

	m = moments(resized, false);
	cmass = Point(round(m.m10/m.m00), round(m.m01/m.m00));	

	centered = Mat(50,50,CV_8UC1, Scalar(0));
	offset = Point(25 - cmass.x, 25 - cmass.y);
	roi = centered(Rect(offset, resized.size()));
	resized.copyTo(roi);
	
	return centered(Rect(11, 11, 28, 28));
}

Mat features(Mat img) {
	Mat result, resultf;
	
	result = img.clone();
	result = result.reshape(0, 1);
	result.convertTo(resultf, CV_32FC1);

	return resultf;
}

Mat features_from_paths(vector<string> img_paths) {
	Mat img, normalized;
	Mat x, float_x;

	for (int i = 0; i < img_paths.size(); ++ i) {
		img = imread(img_paths[i], 1);
		normalized = normalize(img);	
		/*
		Mat output;
		resize(255 - normalized, output,  Size(100, 100), 0, 0, INTER_CUBIC);
		imwrite(img_paths[i] + ".jpg", output);
		*/
		x.push_back(features(normalized));	
	}
	
	x.convertTo(float_x, CV_32F);
	return float_x;
}

KNearest read_knn(string fname, int &k) {
	Mat x, y;

	FileStorage fin(fname, FileStorage::READ);
	fin["x"] >> x;
	fin["y"] >> y;
	fin["k"] >> k;
	
	KNearest knn(x, y, cv::Mat(), false, k);	
	return knn;
}

void write_knn(string fname, Mat x, Mat y, int k) {
	FileStorage fout(fname, FileStorage::WRITE);
	fout << "x" << x;
	fout << "y" << y;
	fout << "k" << k;
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
