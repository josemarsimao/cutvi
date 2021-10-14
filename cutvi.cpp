// --------------------------------
//
// Cutvi - Cut video
//
// Author: Josemar Simao
// email:  josemars@ifes.edu.br
//
// --------------------------------




#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#define MAX_STR_SIZE                1024
//#include <opencv2/features2d/features2d.hpp>
//#include <opencv2/xfeatures2d/nonfree.hpp>
//#include <opencv2/xfeatures2d.hpp>
//#include <opencv2/flann.hpp>
//#include <linux/videodev2.h>
//#include <opencv2/core/core.hpp>
//#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

    VideoCapture cap;
    char TrackbarName01[] = "Start Frame";    // Name of trackbar one
    char TrackbarName02[] = "End Frame";    // Name of trackbar two
    int totalframes;
    Mat frame;

static void on_trackbar( int, void* ){
    int tbpos01;
    int tbpos02;

    tbpos01 = getTrackbarPos(TrackbarName01, "Cutvi");
    tbpos02 = getTrackbarPos(TrackbarName02, "Cutvi");

    if(tbpos01 > tbpos02){
        setTrackbarPos(TrackbarName02, "Cutvi", tbpos01);
    }

    if(tbpos02 < tbpos01){
        setTrackbarPos(TrackbarName01, "Cutvi", tbpos02);
    }

    cap.set(CAP_PROP_POS_FRAMES,tbpos01);

    cap >> frame;

}

static void onMouse(int evt, int x, int y, int flags, void* param) {
    if(evt == CV_EVENT_LBUTTONDOWN) {
        cv::Point* ptPtr = (cv::Point*)param;
        ptPtr->x = x;
        ptPtr->y = y;
    }
}

static void chg_ext_file(char* name, char* ext){
    int sz = strlen(name);
    char* p = name + sz; /// it will start in '\n' character
    char* q = ext;
    while(*q != '.' && *q != 0){q++;}
    while(*p != '.'){
        p--;
    }
    if( (int(p-name) + 1 + strlen(ext)) > MAX_STR_SIZE){
        printf("Size file name is very big\n");
        exit(0);
    }
    if(*q == 0){
        p++;
        strcpy(p,ext);
    }else{
        *p++ = '_';
        strcpy(p,ext);
    }

}

int main(){

    Point2i pt(-1,-1);//assume initial point
    double fps;                 // Frame Rate
    char fn[MAX_STR_SIZE];      // File name

    int startframe;
    int endframe;

    FILE *f = popen("zenity --file-selection", "r"); // popen starts a new process. zenity is a process that creates a dialog box
    fgets(fn, 1024, f);
    /// There is a '/n' character in the end of string
    int sz = strlen(fn);
    if(sz == 0){
        return 0;
    }
    fn[sz-1] = 0;
    cap.open(fn);
    fps = cap.get(CAP_PROP_FPS);
    totalframes = cap.get(CAP_PROP_FRAME_COUNT);
    startframe = totalframes/10;
    endframe = (totalframes * 90)/100;


    if(!cap.open(fn)){
        return 0;
    }

    namedWindow("Cutvi", CV_WINDOW_AUTOSIZE); // Create a window

    startWindowThread();

    setMouseCallback("Cutvi", onMouse, (void*)&pt);

    createTrackbar( TrackbarName01, "Cutvi", &startframe, totalframes, on_trackbar );
    createTrackbar( TrackbarName02, "Cutvi", &endframe, totalframes, on_trackbar );



    /// Select ROI from first image
    cap.set(CAP_PROP_POS_FRAMES,startframe);
    cap >> frame;
    if(frame.empty()){
        return 0;
    }
    Rect2d r = selectROI("Cutvi",frame);

    /// Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file.
    chg_ext_file(fn,(char*)"out.avi");
    VideoWriter video(fn,VideoWriter::fourcc('M','J','P','G'),fps, Size(r.width,r.height));

    /// Set start frame to write
    cap.set(CAP_PROP_POS_FRAMES,startframe);

    while(!frame.empty() &&  cap.get(CAP_PROP_POS_FRAMES) < endframe) {

        /// Crop image
        Mat imCrop = frame(r);

        /// salve image in new file
        video.write(imCrop);

        /// Capture a new image
        cap >> frame;

    }

    cap.release();
    video.release();
    destroyWindow("Cutvi");
    ///destroyAllWindows();

    return 0;
}
