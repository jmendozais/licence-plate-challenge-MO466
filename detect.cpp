#include "detect.h"

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
    resize(img, scaled_img, Size(0, 0), factor, factor);

    return scaled_img;
}

vector<Rect> detect(Mat img) {
    CascadeClassifier plate_cascade;
    String plate_cascade_name = "model.xml";
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
    plate_cascade.detectMultiScale( img_gray, plates, 1.1, 2, 0|CASCADE_SCALE_IMAGE );
    size = plates.size();
    for (int i = 0; i < size; ++ i)
        plates.push_back(Rect(plates[i]));
    // groupRectangles(plates, 1, 0.2);
    groupRectangles(plates, 1, 0.5);

    return plates;
}

void write_plates_to_file(Mat img, vector<Rect> plates, char name[]) {
    Mat roi;
    char s[1234];

    for (int i = 0; i < plates.size(); i++) {
        sprintf(s, "%s_%d.jpg", name, i);
        roi = img(plates[i]);
        imwrite(s, roi);
    }
}

void show_detect_result(Mat img, vector<Rect> plates) {
    for (int i = 0; i < plates.size(); i++) {
        Scalar color = Scalar(0, 0, 255);
        rectangle(img, plates[i].tl(), plates[i].br(), color, 2, 8, 0);
    }
    imshow("detect", img);
}

int main(int argc, char* argv[])  {
    Mat img, scaled_img, plate_roi;
    vector<Rect> plates;

    if (argc < 2) 
        cout << "Usage: ./detect img_path" << endl;
    img = imread(argv[1], 1),

    scaled_img = scale(img);

    plates = detect(scaled_img);

    write_plates_to_file(scaled_img, plates, "test1");

    show_detect_result(scaled_img, plates);

    while (true) {
        int c;
        c = waitKey(20);
        if ((char)c == 'q' || (char)c == 'Q')
            break;
    }

    return 0;
}
