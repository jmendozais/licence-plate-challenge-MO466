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
    int threshold_type = THRESH_BINARY;
    int const max_value = 255;
    int const max_type = 4;
    int const max_BINARY_value = 255;
    int morph_size = 1;
    RNG rng(12345);
    
    /// Convert the image to Gray
    cvtColor( src, src_gray, CV_RGB2GRAY );

    // Blur
    medianBlur(src_gray, src_gray, 5);
    
    // Threshhold
    threshold( src_gray, preproc, threshold_value, max_BINARY_value,threshold_type );
    
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
          
        //If taller than it is long, save possible character
        if (rect.width < rect.height){
            characters.push_back(rect);
        }
        //}
    }

    // Remove smallest rectangles
    sort(characters.begin(), characters.end(), cmp_area);
    vector<Rect> digits(characters.begin(), characters.begin()+7);

    //sort by x value
    sort(digits.begin(), digits.end(), cmp_x);

    return digits;
}

void show_result(Mat orig, vector<Rect> digits){
    for( int i = 0; i < 7; i++ ){
        Scalar color = Scalar(0, 255, 0);
        rectangle(orig, digits[i].tl(), digits[i].br(), color, 2, 8, 0);
    }

    imshow("Contours", orig);
}

int main( int argc, char** argv ){
    // Load an license-plate image
    Mat orig = imread( argv[1], 1 );

    // Preprocess
    Mat preproc = preprocess(orig);

    // Segment
    vector<Rect> digits = segment(preproc);

    // Write to file
    write_digits_to_file(digits, preproc, "test2");

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


