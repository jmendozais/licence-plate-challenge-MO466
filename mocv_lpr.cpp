#include "mocv_lpr.h"

using namespace std;
using namespace cv;

/* Mastering OpenCV impl */
Plate::Plate(){
}

/*
Plate::Plate(Mat img, Rect pos){
    plateImg=img;
    position=pos;
} */

Plate::Plate(Mat img, RotatedRect rot){
    plateImg=img;
	rotRect=rot;
	position=rot.boundingRect();
	position.x = max(position.x, 0);
	position.y = max(position.y, 0);
}

string Plate::str(){
    string result="";
    //Order numbers
    vector<int> orderIndex;
    vector<int> xpositions;
    for(int i=0; i< charsPos.size(); i++){
        orderIndex.push_back(i);
        xpositions.push_back(charsPos[i].x);
    }
    float min=xpositions[0];
    int minIdx=0;
    for(int i=0; i< xpositions.size(); i++){
        min=xpositions[i];
        minIdx=i;
        for(int j=i; j<xpositions.size(); j++){
            if(xpositions[j]<min){
                min=xpositions[j];
                minIdx=j;
            }
        }
        int aux_i=orderIndex[i];
        int aux_min=orderIndex[minIdx];
        orderIndex[i]=aux_min;
        orderIndex[minIdx]=aux_i;
        
        float aux_xi=xpositions[i];
        float aux_xmin=xpositions[minIdx];
        xpositions[i]=aux_xmin;
        xpositions[minIdx]=aux_xi;
    }
    for(int i=0; i<orderIndex.size(); i++){
        result=result+chars[orderIndex[i]];
    }
    return result;
}

void DetectRegions::setFilename(string s) {
    filename=s;
};

DetectRegions::DetectRegions(){
    showSteps=false;
    saveRegions=false;
}

bool DetectRegions::verifySizes(RotatedRect mr){
    
    float error=0.4;
    //Spain car plate size: 52x11 aspect 4,7272
    float aspect=4.7272;
    //Set a min and max area. All other patchs are discarded
    int min= 15*aspect*15; // minimum area
    int max= 125*aspect*125; // maximum area
    //Get only patchs that match to a respect ratio.
    float rmin= aspect-aspect*error;
    float rmax= aspect+aspect*error;
    
    int area= mr.size.height * mr.size.width;
    float r= (float)mr.size.width / (float)mr.size.height;
    if(r<1)
        r= (float)mr.size.height / (float)mr.size.width;
    
    if(( area < min || area > max ) || ( r < rmin || r > rmax )){
        return false;
    }else{
        return true;
    }
    
}

Mat DetectRegions::histeq(Mat in)
{
    Mat out(in.size(), in.type());
    if(in.channels()==3){
        Mat hsv;
        vector<Mat> hsvSplit;
        cvtColor(in, hsv, CV_BGR2HSV);
        split(hsv, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, hsv);
        cvtColor(hsv, out, CV_HSV2BGR);
    }else if(in.channels()==1){
        equalizeHist(in, out);
    }
    
    return out;
    
}

vector<Plate> DetectRegions::segment(Mat input){
    vector<Plate> output;
    
    //convert image to gray
    Mat img_gray;
    cvtColor(input, img_gray, CV_BGR2GRAY);
    blur(img_gray, img_gray, Size(5,5));
    
    //Finde vertical lines. Car plates have high density of vertical lines
    Mat img_sobel;
    Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    if(showSteps)
        imshow("Sobel", img_sobel);
    
    //threshold image
    Mat img_threshold;
    threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    if(showSteps)
        imshow("Threshold", img_threshold);
    
    //Morphplogic operation close
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3) );
    morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
    if(showSteps)
        imshow("Close", img_threshold);
    
    //Find contours of possibles plates
    vector< vector< Point> > contours;
    findContours(img_threshold,
                 contours, // a vector of contours
                 CV_RETR_EXTERNAL, // retrieve the external contours
                 CV_CHAIN_APPROX_NONE); // all pixels of each contours
    
    //Start to iterate to each contour founded
    vector<vector<Point> >::iterator itc= contours.begin();
    vector<RotatedRect> rects;
    
    //Remove patch that are no inside limits of aspect ratio and area.
    while (itc!=contours.end()) {
        //Create bounding rect of object
        RotatedRect mr= minAreaRect(Mat(*itc));
        if( !verifySizes(mr)){
            itc = contours.erase(itc);
        }else{
            ++itc;
            rects.push_back(mr);
        }
    }
    
    // Draw blue contours on a white image
    cv::Mat result;
    input.copyTo(result);
    cv::drawContours(result,contours,
                     -1, // draw all contours
                     cv::Scalar(255,0,0), // in blue
                     1); // with a thickness of 1
    
    for(int i=0; i< rects.size(); i++){
        
        //For better rect cropping for each posible box
        //Make floodfill algorithm because the plate has white background
        //And then we can retrieve more clearly the contour box
        circle(result, rects[i].center, 3, Scalar(0,255,0), -1);
        //get the min size between width and height
        float minSize=(rects[i].size.width < rects[i].size.height)?rects[i].size.width:rects[i].size.height;
        minSize=minSize-minSize*0.5;
        //initialize rand and get 5 points around center for floodfill algorithm
        srand ( time(NULL) );
        //Initialize floodfill parameters and variables
        Mat mask;
        mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
        mask= Scalar::all(0);
        int loDiff = 30;
        int upDiff = 30;
        int connectivity = 4;
        int newMaskVal = 255;
        int NumSeeds = 10;
        Rect ccomp;
        int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
        for(int j=0; j<NumSeeds; j++){
            Point seed;
            seed.x=rects[i].center.x+rand()%(int)minSize-(minSize/2);
            seed.y=rects[i].center.y+rand()%(int)minSize-(minSize/2);
			seed.x = max(seed.x, 0);
			seed.y = max(seed.y, 0);
			//if(seed.x < 0 || seed.x >= input.rows || seed.y < 0 || seed.y >= input.cols ) continue;
			if(seed.x < seed.y) 
            circle(result, seed, 1, Scalar(0,255,255), -1);
            int area = floodFill(input, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
        }
        if(showSteps)
            imshow("MASK", mask);
        //cvWaitKey(0);
        
        //Check new floodfill mask match for a correct patch.
        //Get all points detected for get Minimal rotated Rect
        vector<Point> pointsInterest;
        Mat_<uchar>::iterator itMask= mask.begin<uchar>();
        Mat_<uchar>::iterator end= mask.end<uchar>();
        for( ; itMask!=end; ++itMask)
            if(*itMask==255)
                pointsInterest.push_back(itMask.pos());
        
        RotatedRect minRect = minAreaRect(pointsInterest);
        if(verifySizes(minRect)){
            // rotated rectangle drawing
            Point2f rect_points[4]; minRect.points( rect_points );
            for( int j = 0; j < 4; j++ )
                line( result, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,255), 1, 8 );
            
            //Get rotation matrix
            float r= (float)minRect.size.width / (float)minRect.size.height;
            float angle=minRect.angle;
            if(r<1)
                angle=90+angle;
            Mat rotmat= getRotationMatrix2D(minRect.center, angle,1);
            
            //Create and rotate image
            Mat img_rotated;
            warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);
            
            //Crop image
            Size rect_size=minRect.size;
            if(r < 1)
                swap(rect_size.width, rect_size.height);
            Mat img_crop;
            getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);
            
            Mat resultResized;
            resultResized.create(33,144, CV_8UC3);
            resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
            //Equalize croped image
            Mat grayResult;
            cvtColor(resultResized, grayResult, CV_BGR2GRAY);
            blur(grayResult, grayResult, Size(3,3));
            grayResult=histeq(grayResult);
            if(saveRegions){
                stringstream ss(stringstream::in | stringstream::out);
                ss << "tmp/" << filename << "_" << i << ".jpg";
                imwrite(ss.str(), grayResult);
            }
            output.push_back(Plate(grayResult,minRect));
        }
    }
    if(showSteps)
        imshow("Contours", result);
    
    return output;
}

vector<Plate> DetectRegions::run(Mat input){
    
    //Segment image by white 
    vector<Plate> tmp=segment(input);
   	 
    //return detected and posibles regions
    return tmp;
}

const char OCR::strCharacters[] = {'0','1','2','3','4','5','6','7','8','9','B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z'};
const int OCR::numCharacters=30;

CharSegment::CharSegment(){}
CharSegment::CharSegment(Mat i, Rect p){
    img=i;
    pos=p;
}

OCR::OCR(){
    DEBUG=false;
    trained=false;
    saveSegments=false;
    charSize=20;
}
OCR::OCR(string trainFile){
    DEBUG=false;
    trained=false;
    saveSegments=false;
    charSize=20;

    //Read file storage.
    FileStorage fs;
    fs.open("OCR.xml", FileStorage::READ);
    Mat TrainingData;
    Mat Classes;
    fs["TrainingDataF15"] >> TrainingData;
    fs["classes"] >> Classes;

    train(TrainingData, Classes, 10);

}

Mat OCR::preprocessChar(Mat in){
    //Remap image
    int h=in.rows;
    int w=in.cols;
    Mat transformMat=Mat::eye(2,3,CV_32F);
    int m=max(w,h);
    transformMat.at<float>(0,2)=m/2 - w/2;
    transformMat.at<float>(1,2)=m/2 - h/2;

    Mat warpImage(m,m, in.type());
    warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0) );

    Mat out;
    resize(warpImage, out, Size(charSize, charSize) ); 

    return out;
}

bool OCR::verifySizes(Mat r){
    //Char sizes 45x77
    float aspect=45.0f/77.0f;
    float charAspect= (float)r.cols/(float)r.rows;
    float error=0.35;
    float minHeight=15;
    float maxHeight=28;
    //We have a different aspect ratio for number 1, and it can be ~0.2
    float minAspect=0.2;
    float maxAspect=aspect+aspect*error;
    //area of pixels
    float area=countNonZero(r);
    //bb area
    float bbArea=r.cols*r.rows;
    //% of pixel in area
    float percPixels=area/bbArea;

    if(DEBUG)
        cout << "Aspect: "<< aspect << " ["<< minAspect << "," << maxAspect << "] "  << "Area "<< percPixels <<" Char aspect " << charAspect  << " Height char "<< r.rows << "\n";
    if(percPixels < 0.8 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeight && r.rows < maxHeight)
        return true;
    else
        return false;

}

vector<CharSegment> OCR::segment(Plate plate){
    Mat input=plate.plateImg;
    vector<CharSegment> output;
    //Threshold input image
    Mat img_threshold;
    threshold(input, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
    if(DEBUG)
        imshow("Threshold plate", img_threshold);
    Mat img_contours;
    img_threshold.copyTo(img_contours);
    //Find contours of possibles characters
    vector< vector< Point> > contours;
    findContours(img_contours,
            contours, // a vector of contours
            CV_RETR_EXTERNAL, // retrieve the external contours
            CV_CHAIN_APPROX_NONE); // all pixels of each contours
    
    // Draw blue contours on a white image
    cv::Mat result;
    img_threshold.copyTo(result);
    cvtColor(result, result, CV_GRAY2RGB);
    cv::drawContours(result,contours,
            -1, // draw all contours
            cv::Scalar(255,0,0), // in blue
            1); // with a thickness of 1

    //Start to iterate to each contour founded
    vector<vector<Point> >::iterator itc= contours.begin();
    
    //Remove patch that are no inside limits of aspect ratio and area.    
    while (itc!=contours.end()) {
        
        //Create bounding rect of object
        Rect mr= boundingRect(Mat(*itc));
        rectangle(result, mr, Scalar(0,255,0));
        //Crop image
        Mat auxRoi(img_threshold, mr);
        if(verifySizes(auxRoi)){
            auxRoi=preprocessChar(auxRoi);
            output.push_back(CharSegment(auxRoi, mr));
            rectangle(result, mr, Scalar(0,125,255));
        }
        ++itc;
    }
    if(DEBUG)
        cout << "Num chars: " << output.size() << "\n";
    


    if(DEBUG)
        imshow("SEgmented Chars", result);
    return output;
}

Mat OCR::ProjectedHistogram(Mat img, int t)
{
    int sz=(t)?img.rows:img.cols;
    Mat mhist=Mat::zeros(1,sz,CV_32F);

    for(int j=0; j<sz; j++){
        Mat data=(t)?img.row(j):img.col(j);
        mhist.at<float>(j)=countNonZero(data);
    }

    //Normalize histogram
    double min, max;
    minMaxLoc(mhist, &min, &max);
    
    if(max>0)
        mhist.convertTo(mhist,-1 , 1.0f/max, 0);

    return mhist;
}

Mat OCR::getVisualHistogram(Mat *hist, int type)
{

    int size=100;
    Mat imHist;


    if(type==HORIZONTAL){
        imHist.create(Size(size,hist->cols), CV_8UC3);
    }else{
        imHist.create(Size(hist->cols, size), CV_8UC3);
    }

    imHist=Scalar(55,55,55);

    for(int i=0;i<hist->cols;i++){
        float value=hist->at<float>(i);
        int maxval=(int)(value*size);

        Point pt1;
        Point pt2, pt3, pt4;

        if(type==HORIZONTAL){
            pt1.x=pt3.x=0;
            pt2.x=pt4.x=maxval;
            pt1.y=pt2.y=i;
            pt3.y=pt4.y=i+1;

            line(imHist, pt1, pt2, CV_RGB(220,220,220),1,8,0);
            line(imHist, pt3, pt4, CV_RGB(34,34,34),1,8,0);

            pt3.y=pt4.y=i+2;
            line(imHist, pt3, pt4, CV_RGB(44,44,44),1,8,0);
            pt3.y=pt4.y=i+3;
            line(imHist, pt3, pt4, CV_RGB(50,50,50),1,8,0);
        }else{

                        pt1.x=pt2.x=i;
                        pt3.x=pt4.x=i+1;
                        pt1.y=pt3.y=100;
                        pt2.y=pt4.y=100-maxval;


            line(imHist, pt1, pt2, CV_RGB(220,220,220),1,8,0);
            line(imHist, pt3, pt4, CV_RGB(34,34,34),1,8,0);

            pt3.x=pt4.x=i+2;
            line(imHist, pt3, pt4, CV_RGB(44,44,44),1,8,0);
            pt3.x=pt4.x=i+3;
            line(imHist, pt3, pt4, CV_RGB(50,50,50),1,8,0);

        }




    }

    return imHist ;
}

void OCR::drawVisualFeatures(Mat character, Mat hhist, Mat vhist, Mat lowData){
    Mat img(121, 121, CV_8UC3, Scalar(0,0,0));
    Mat ch;
    Mat ld;
    
    cvtColor(character, ch, CV_GRAY2RGB);

    resize(lowData, ld, Size(100, 100), 0, 0, INTER_NEAREST );
    cvtColor(ld,ld,CV_GRAY2RGB);

    Mat hh=getVisualHistogram(&hhist, HORIZONTAL);
    Mat hv=getVisualHistogram(&vhist, VERTICAL);

    Mat subImg=img(Rect(0,101,20,20));
    ch.copyTo(subImg);

    subImg=img(Rect(21,101,100,20));
    hh.copyTo(subImg);

    subImg=img(Rect(0,0,20,100));
    hv.copyTo(subImg);

    subImg=img(Rect(21,0,100,100));
    ld.copyTo(subImg);

    line(img, Point(0,100), Point(121,100), Scalar(0,0,255));
    line(img, Point(20,0), Point(20,121), Scalar(0,0,255));

    imshow("Visual Features", img);

    cvWaitKey(0);
}

Mat OCR::features(Mat in, int sizeData){
    //Histogram features
    Mat vhist=ProjectedHistogram(in,VERTICAL);
    Mat hhist=ProjectedHistogram(in,HORIZONTAL);
    
    //Low data feature
    Mat lowData;
    resize(in, lowData, Size(sizeData, sizeData) );

    if(DEBUG)
        drawVisualFeatures(in, hhist, vhist, lowData);
    

    
    //Last 10 is the number of moments components
    int numCols=vhist.cols+hhist.cols+lowData.cols*lowData.cols;
    
    Mat out=Mat::zeros(1,numCols,CV_32F);
    //Asign values to feature
    int j=0;
    for(int i=0; i<vhist.cols; i++)
    {
        out.at<float>(j)=vhist.at<float>(i);
        j++;
    }
    for(int i=0; i<hhist.cols; i++)
    {
        out.at<float>(j)=hhist.at<float>(i);
        j++;
    }
    for(int x=0; x<lowData.cols; x++)
    {
        for(int y=0; y<lowData.rows; y++){
            out.at<float>(j)=(float)lowData.at<unsigned char>(x,y);
            j++;
        }
    }
    if(DEBUG)
        cout << out << "\n===========================================\n";
    return out;
}

void OCR::train(Mat TrainData, Mat classes, int nlayers){
    Mat layers(1,3,CV_32SC1);
    layers.at<int>(0)= TrainData.cols;
    layers.at<int>(1)= nlayers;
    layers.at<int>(2)= numCharacters;
    ann.create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);

    //Prepare trainClases
    //Create a mat with n trained data by m classes
    Mat trainClasses;
    trainClasses.create( TrainData.rows, numCharacters, CV_32FC1 );
    for( int i = 0; i <  trainClasses.rows; i++ )
    {
        for( int k = 0; k < trainClasses.cols; k++ )
        {
            //If class of data i is same than a k class
            if( k == classes.at<int>(i) )
                trainClasses.at<float>(i,k) = 1;
            else
                trainClasses.at<float>(i,k) = 0;
        }
    }
    Mat weights( 1, TrainData.rows, CV_32FC1, Scalar::all(1) );
    
    //Learn classifier
    ann.train( TrainData, trainClasses, weights );
    trained=true;
}

int OCR::classify(Mat f){
    int result=-1;
    Mat output(1, numCharacters, CV_32FC1);
    ann.predict(f, output);
    Point maxLoc;
    double maxVal;
    minMaxLoc(output, 0, &maxVal, 0, &maxLoc);
    //We need know where in output is the max val, the x (cols) is the class.

    return maxLoc.x;
}

int OCR::classifyKnn(Mat f){
    int response = (int)knnClassifier.find_nearest( f, K );
    return response;
}
void OCR::trainKnn(Mat trainSamples, Mat trainClasses, int k){
    K=k;
    // learn classifier
    knnClassifier.train( trainSamples, trainClasses, Mat(), false, K );
}

string OCR::run(Plate *input){
	string result;
    
    //Segment chars of plate
    vector<CharSegment> segments=segment(*input);

    for(int i=0; i<segments.size(); i++){
        //Preprocess each char for all images have same sizes
        Mat ch=preprocessChar(segments[i].img);
        if(saveSegments){
            stringstream ss(stringstream::in | stringstream::out);
            ss << "tmpChars/" << filename << "_" << i << ".jpg";
            imwrite(ss.str(),ch);
        }
        //For each segment Extract Features
        Mat f=features(ch,15);
        //For each segment feature Classify
        int character=classify(f);
		result += strCharacters[character];
        input->chars.push_back(strCharacters[character]);
        input->charsPos.push_back(segments[i].pos);
    }
    return "-";//input->str();
}

char OCR::run_on_char(Mat ch){
	Mat preproc, feat;
    //Preprocess char
    preproc = preprocessChar(ch);
    //For each segment Extrchact Features
    feat = features(preproc,15);
    //For each segment feature Classify
    return strCharacters[classify(feat)];
}

