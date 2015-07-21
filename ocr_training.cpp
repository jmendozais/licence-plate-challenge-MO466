#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include "recognize.h"

using namespace std;
using namespace cv;

void train_knn(Mat tr_x, Mat tr_y, Mat te_x, Mat te_y) {
	Mat x, y, pred;

	// Grid search
	int ks[8] = {1, 3, 5, 7, 11, 15, 21, 29};
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
}

void train_svm(Mat tr_x, Mat tr_y, Mat te_x, Mat te_y) {
	Mat pred, x, y;
	int exps[8] = {-4, -3, -2, -1, 0, 1, 2, 3};
	double C, gamma, best_C, best_gamma, best_acc, acc;
	for (int i = 1; i < 12; ++ i) {
		for (int j = 0; j < 8; ++ j) {
			C = i;
			gamma = pow(10, exps[j]);
			pred = Mat();

			// Set up SVM's parameters
    		CvSVMParams params;
    		params.svm_type    = CvSVM::C_SVC;
    		params.kernel_type = CvSVM::RBF;
			params.gamma = gamma;
			params.C = C;

    		// Train the SVM
    		CvSVM SVM;
    		SVM.train(tr_x, tr_y, Mat(), Mat(), params);
			for (int i = 0; i < te_x.rows; ++ i) {
				pred.push_back(SVM.predict(te_x.rowRange(i, i+1)));
			}

			// Update best hyperparameters
			assert(pred.rows == te_y.rows);
			acc = evaluate(te_y, pred);
			cout << "C = " << C << " gamma = " << gamma << " acc: " << acc << endl;
			if (best_acc < acc) {
				best_acc = acc;
				best_gamma = gamma;
				best_C = C;
			}
		}
	}

	cout << "best C = " << best_C << " best gamma = " << best_gamma << ", best acc = " << best_acc << endl;

	x = tr_x;
	x.push_back(te_x);
	y = tr_y;
	y.push_back(te_y);

	// Create best SVM
	CvSVMParams params;
    params.svm_type = CvSVM::C_SVC;
    params.kernel_type = CvSVM::RBF;
	params.gamma = best_gamma;
	params.C = best_C;

    // Train best SVM
    CvSVM SVM;
    SVM.train(tr_x, tr_y, Mat(), Mat(), params);
	for (int i = 0; i < te_x.rows; ++ i) {
		pred.push_back(SVM.predict(te_x.rowRange(i, i+1)));
	}
	SVM.save("ocr_svm.xml");

	// Test svm loading
    CvSVM SVM2;
	SVM2.load("ocr_svm.xml");

	for (int i = 0; i < te_x.rows; ++ i) {
		cout << SVM2.predict(te_x.rowRange(i, i+1)) << endl;
	}
}

int main(int argc, char *argv[]) {
	vector<string> train_paths;
	vector<string> test_paths;
	string line, ann_path, ann;
	Mat img, normalized, x, y, tr_x, tr_y, te_x, te_y, pred;

    if (argc < 3) {
        cout << "Usage: ./ocr_training train_list test_list" << endl;
		return -1;
	}

	read_set(argv[1], &train_paths, &tr_y);
	read_set(argv[2], &test_paths, &te_y);

	// Extract features
	tr_x = features_from_paths(train_paths);
	te_x = features_from_paths(test_paths);

	// Scale features [-1, 1]
	tr_x /= 128;
	tr_x -= 1;
	te_x /= 128;
	te_x -= 1;

	// train_knn(tr_x, tr_y, te_x, te_y);
	train_svm(tr_x, tr_y, te_x, te_y);

	return 0;
}
