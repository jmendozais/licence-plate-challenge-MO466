#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "segment.h"

using namespace std;
using namespace cv;

bool cmp_area(const Rect & a, const Rect & b) {
    return a.width*a.height > b.width*b.height;
}

bool cmp_x(const Rect & a, const Rect & b) {
    return a.x < b.x;
}

vector<Mat> get_digit_vector(vector<Rect> & chars, Mat preproc){
    vector<Mat> croppedImages;

    for(int i = 0; i < 7; ++i)
        croppedImages[i].push_back(preproc(Rect(chars[i].x, chars[i].y, chars[i].width,
                                                chars[i].height)));

    return croppedImages;
}

void write_digits_to_file(vector<Rect> & chars, Mat preproc, char name[]){
    Mat croppedImage;
    char s[1234];

    for(int i = 0; i < chars.size(); ++i){
        sprintf(s, "%s%d.jpg", name, i);
        croppedImage = preproc(Rect(chars[i].x, chars[i].y, chars[i].width, chars[i].height));
        imwrite(s, croppedImage);
    }
}

Mat preprocess(Mat src){
    Mat src_gray, preproc, dst, SE_element, distance;
    int threshold_value = 150;
    int threshold_type = THRESH_BINARY | THRESH_OTSU;
    int const max_value = 255;
    int const max_type = 4;
    int const max_BINARY_value = 255;
    int morph_size = 1;
    RNG rng(12345);
    
    /// Convert the image to Gray
    cvtColor( src, src_gray, CV_RGB2GRAY );

    //imshow("input", src);

    // Blur
    medianBlur(src_gray, src_gray, 5);
    
    //imshow("blur", src_gray);

    // Threshhold
    //threshold( src_gray, preproc, threshold_value, max_BINARY_value,threshold_type );
    adaptiveThreshold( src_gray, preproc, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 91, 0);
    
    //imshow("Binary", preproc);
    
    // Opening
    morph_size = 1;
    SE_element = getStructuringElement(MORPH_ELLIPSE,
                                       Size(2*morph_size+1, 2*morph_size+1),
                                       Point( morph_size, morph_size ) );
    morphologyEx(preproc, preproc, MORPH_OPEN, SE_element);

    //imshow("Opened", preproc);

    // Closing
    morph_size = 1;
    SE_element = getStructuringElement(MORPH_ELLIPSE,
                                       Size(2*morph_size+1, 2*morph_size+1),
                                       Point( morph_size, morph_size ) );
    morphologyEx(preproc, preproc, MORPH_CLOSE, SE_element);

    //imshow("Closed", preproc);

    // Dilate
    morph_size = 1;
    dilate(preproc, preproc, SE_element);//, Point(-1,-1), 1, 1, 1);

    //preproc = 255 - preproc;
    return preproc;
}

bool _too_white(Rect r, Mat img){
    Point p = r.tl();
    int w = 0, b = 0;
    for(int i = p.y; i < p.y+r.height; ++i){
        for(int j = p.x; j < p.x+r.width; ++j){
            Vec3b pxl = img.at<Vec3b>(j, i);
            if(pxl[1] == 255) ++w;
            else ++b;

        }
    }
    if((double)b/(double)w < 0.3) return 1;
    return 0;
}

vector<Rect> segment(Mat preproc){

    // Find Contours
    vector<vector<Point> > contours;
    vector<Point> approx;
    vector<Vec4i> hierarchy;
    vector<Rect> characters;
    Mat dst = 255-preproc;

    findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE,Point(0, 0));

    /// Find the rectangles for bigger contours
    for( int i = 0; i < contours.size(); i++ ){

        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
        double area = fabs(contourArea(Mat(approx)));
        //if( area > 500 && area < 5000){
        //Get rectangle containing character from image

        Rect rect = boundingRect(Mat(approx));

        char str[1234];
        sprintf(str, "contours_%d.jpg", i);
        //imshow(str, preproc);

        //If taller than it is long, save possible character
        if (rect.width < rect.height){
            characters.push_back(rect);
        }
        //}
    }

    // Remove smallest rectangles and those with wrong height
    sort(characters.begin(), characters.end(), cmp_area);

    // If the difference to one's height compared to the 7 first is too much, discard
    double minh = 1234567;
    int idx, idx2;
    for(int i = 0; i < 7; ++i){
        if(characters[i].height < minh){
            minh = characters[i].height;
            idx = i;
        }
        else idx2 = i;
    }
    double sum = 0, avg_diff;
    for(int i = 1; i < 7; ++i){
        if(i==idx || i-1==idx) continue;
        sum += abs(characters[i].height - characters[i-1].height);
    }
    avg_diff = sum/4;
    if(abs(characters[idx2].height-characters[idx].height) > characters[idx2].height/4)
        characters.erase(characters.begin() + idx);

    // Remove rectangles whose interior is almost all white
    /*for(int i = 0; i < 7; ++i){
        if(_too_white(characters[i], preproc)){
            characters.erase(characters.begin() + i);
            i--;
        }
        }*/

    // Remove rectangles that contain others
    

    vector<Rect> digits;
    if(characters.size() >=7)
        digits = vector<Rect>(characters.begin(), characters.begin()+7);
    else digits = characters;
    sort(digits.begin(), digits.end(), cmp_x);

    return digits;
}

void show_result(Mat orig, vector<Rect> digits){
	int num_digits = min(7, (int)digits.size());
	
    for( int i = 0; i < num_digits; i++ ){
        Scalar color = Scalar(0, 255, 0);
        rectangle(orig, digits[i].tl(), digits[i].br(), color, 2, 8, 0);
    }

    imshow("Contours", orig);
}


int main( int argc, char** argv ){
    // Load an license-plate image
    Mat orig = imread( argv[1], 1 );

    // Preprocess
    cout << "preprocess" << endl;
    Mat preproc = preprocess(orig);

    // Segment
    cout << "segment" << endl;
    vector<Rect> digits = segment(preproc);

    cout << "write" << endl;
    // Write to file
    write_digits_to_file(digits, preproc, "digit");

    // Show results
    imshow("preprocessed", preproc);
    show_result(orig, digits);

    // Wait until user finishes program
    while(true){
        int c;
        c = waitKey( 20 );
        if( (char)c == 'q' || (char)c == 'Q' )
            break;
    }
}

