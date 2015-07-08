#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include "recognize.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
	vector<string> train_paths;
	vector<string> test_paths;
	string line, ann_path, ann;
	Mat img, normalized, x, y, tr_x, tr_y, te_x, te_y, pred;

    if (argc < 3) 
        cout << "Usage: ./ocr_training train_list test_list" << endl;

	read_set(argv[1], train_paths, tr_y);
	read_set(argv[2], test_paths, te_y);

	tr_x = features_from_paths(train_paths);	
	te_x = features_from_paths(test_paths);	
	
	// grid search
	int ks[8] = {1,3,5,7,11,15,21,29};
	int best_k;
	double best_acc = -1, acc;
	for (int i = 0; i < 8; ++ i) {
		KNearest knn(tr_x, tr_y, cv::Mat(), false, ks[i]);	
		knn.find_nearest(te_x, ks[i], &pred);

		acc = evaluate(te_y, pred);
		cout << "k = " << ks[i] << " acc: " << acc << endl;
		if (best_acc < acc) {
			best_acc = acc;
			best_k = ks[i];
		}
	}
	cout << "best k = " << best_k << ", best acc = " << best_acc << endl;
	// 3 is between bests 1 and 5
	best_k = 3;
	x = tr_x;	
	x.push_back(te_x);
	y = tr_y;	
	y.push_back(te_y);

	write_knn("ocr.xml", x, y, best_k);
		
	return 0;
}
