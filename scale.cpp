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

bool debug = false;
int main(int argc, char* argv[]) {
  double margin = 0.2;
  ios_base::sync_with_stdio(false);
  cin.tie(NULL);
  string img_list = argv[1];
  string out_list = argv[2];
  int major_side = atoi(argv[3]);
  bool isPositive = atoi(argv[4]) != 0;
  ifstream fin(img_list.c_str());
  ofstream fout(out_list.c_str());

  string line, path;
  vector<int> pts(8);
  double factor = 1;
  while(getline(fin, line)) {
      stringstream ss; ss << line; ss >> path;
	  path = "scaled/" + path;
      cout << path << endl;
      Mat img = imread(path, 1);
      Size dsize = img.size();
      factor = max(dsize.width, dsize.height) * 1.0 / major_side;
      dsize.width /= factor;
      dsize.height /= factor;

      Mat rimg(dsize.width, dsize.height, CV_64F);
      fout << path;
      if (isPositive) {
          fout << " ";
          fout << 1;
          ss >> pts[0];
          for(int i = 0; i < 4; ++ i) {
              ss >> pts[i];
              pts[i] /= factor;
          }
          cout << pts[0] << " " << pts[1] << " " << pts[2] << " " << pts[3] << endl;
          pts[0] -= (pts[3] * margin / 2.0);
          pts[1] -= (pts[3] * margin / 2.0);
          pts[2] += (pts[3] * margin);
          pts[3] += (pts[3] * margin);
          for(int i = 0; i < 4; ++ i) {
              fout << " ";
              fout << pts[i];
              cout << " ";
              cout << pts[i];
          }
      }
      fout << endl;
      cout << endl;

      resize(img, rimg, dsize);
      if (debug) {
          Mat tmp = rimg.clone();
          if (isPositive)
              rectangle(tmp, Point(pts[0], pts[1]), Point(pts[0]+pts[2], pts[1]+pts[3]), Scalar(0, 0, 255), 3);
          namedWindow("LPR");
          imshow("LPR", tmp);
          waitKey();
      }
      imwrite(path, rimg);
  }
  return 0;
}
