#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;

int input(int argc, char *argv[], string &data_dir, string &out_dir, string &in_list, string &out_list, int &major_side, bool &isPositive) {
	if (argc < 6) {
		cout << "Usage: ./scale data_dir out_dir input_list output_list major_side is_positive" << endl;
		return -1;
	}
	data_dir = argv[1];
	out_dir = argv[2];
	in_list = argv[3];
	out_list = argv[4];
	major_side = atoi(argv[5]);
	isPositive = atoi(argv[6]) != 0;
	return 0;
}

bool debug = false;
int main(int argc, char* argv[]) {
	string img_list, out_list, parent_dir, data_dir, out_dir;
	int major_side;
	bool isPositive;
	double margin = 0.2;

	if (input(argc, argv, data_dir, out_dir, img_list, out_list, major_side, isPositive) == -1)
		return 0;

	ifstream fin(img_list.c_str());
	ofstream fout(out_list.c_str());
	string line, img_name, img_path, scaled_img_path, ann_name, ann_path, scaled_ann_path;
	vector<int> pts(8);
	double factor = 1;

	while(getline(fin, line)) {

		stringstream ss; ss << line; ss >> img_name;
		img_path = data_dir + "/" + img_name;
		scaled_img_path = out_dir + "/" + img_name;
		ann_name = img_name.substr(0, img_name.size() - 3) + "txt";
		ann_path = img_path.substr(0, img_path.size() - 3) + "txt";
		scaled_ann_path = scaled_img_path.substr(0, scaled_img_path.size() - 3) + "txt";

		// Resize the image
		Mat img = imread(img_path, 1);
		Size dsize = img.size();
		factor = max(dsize.width, dsize.height) * 1.0 / major_side;
		dsize.width = round(dsize.width/factor);
		dsize.height = round(dsize.width/factor);

		// Write the image
		Mat rimg(dsize.width, dsize.height, CV_64F);
		resize(img, rimg, dsize);
		imwrite(scaled_img_path, rimg);

		if (debug) {
			Mat tmp = rimg.clone();
			if (isPositive)
				rectangle(tmp, Point(pts[0], pts[1]), Point(pts[0]+pts[2], pts[1]+pts[3]), Scalar(0, 0, 255), 3);
			namedWindow("LPR");
			imshow("LPR", tmp);
			waitKey();
		}
		
		// Write annotation
		cout << "ann_path " << ann_path << endl;
		ifstream fin1(ann_path);
		ofstream fout1(scaled_ann_path);
		int coord;

		if (isPositive) {
			fin1 >> line;
			replace(line.begin(), line.end(), ',', ' ');
			stringstream ss(line);
			istream_iterator<string> begin(ss);
			istream_iterator<string> end;
			vector<string> toks(begin, end);
			assert(toks.size() == 9);
			for (int i = 0; i < 8; ++ i) {
				coord = atoi(toks[i].c_str());
				fout1 << round(coord / factor) << ",";
			}
			fout1 << toks[8];
		} else {
			fout1 << "None" << endl;
		}

		fin1.close();
		fout1.close();

		// Write list
		fout << img_name;
		if (isPositive) {
			fout << " 1";
			ss >> pts[0];
			for(int i = 0; i < 4; ++ i) {
				ss >> pts[i];
				pts[i] = round(pts[i]/factor);
			}
			cout << pts[0] << " " << pts[1] << " " << pts[2] << " " << pts[3] << endl;
			pts[0] -= (pts[3] * margin / 2.0);
			pts[1] -= (pts[3] * margin / 2.0);
			pts[2] += (pts[3] * margin);
			pts[3] += (pts[3] * margin);
			for(int i = 0; i < 4; ++ i) {
				fout << " " << pts[i];
				cout << " " << pts[i];
			}
		}
		fout << endl;
		cout << endl;
	}
	return 0;
}
