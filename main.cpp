// g++  main.cpp -o robocoffee `pkg-config --cflags opencv`   `pkg-config --libs opencv`

const int DESIRED_CAMERA_WIDTH = 1366;
const int DESIRED_CAMERA_HEIGHT = 768;
const char *windowName = "RoboCoffee";   // Name shown in the GUI window.
const int BORDER = 30;  // Border between GUI elements to the edge of the image.

#include <stdio.h>
#include <vector>
#include <time.h>
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<errno.h>
#include<arpa/inet.h>   
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

Rect m_rcBtnAdd;
//Rect m_rcBtnDel;

int KAPA = 0;
int evaluate = 0;


bool isPointInRect(const Point pt, const Rect rc)
{
    if (pt.x >= rc.x && pt.x <= (rc.x + rc.width - 1))
        if (pt.y >= rc.y && pt.y <= (rc.y + rc.height - 1))
            return true;
	
    return false;
}


int send_image(int socket, int sequence)
{
    cout<<"ENTERING SEND"<<endl;
    std::string path = "capture";
	std::string jpg =".jpg";
    stringstream sstream ;
    sstream << path << sequence << jpg;
    std::string full_path = sstream.str();
    
    FILE *picture;
	int size, read_size, stat, packet_index;
	char send_buffer[10240], read_buffer[256];
	packet_index = 1;
    cout << full_path.c_str()<<endl;
	picture = fopen(full_path.c_str(), "r");
	printf("Getting Picture Size\n");   

	if(picture == NULL) 
	{
        perror(full_path.c_str());
		printf("Error %d \n", errno);
    }
    cout << "1" << endl;
	fseek(picture, 0, SEEK_END);
    cout << "2" << endl;
	size = ftell(picture);

    cout << "3" << endl;
	fseek(picture, 0, SEEK_SET);
	printf("Total Picture size: %i\n",size);

	//Send Picture Size
	printf("Sending Picture Size\n");
	write(socket, (void *)&size, sizeof(int));

	//Send Picture as Byte Array
	printf("Sending Picture as Byte Array\n");

	do 
	{ //Read while we get errors that are due to signals.
        cout<<"Reading"<< endl;
		stat=read(socket, &read_buffer , 255);
		printf("Bytes read: %i\n",stat);
	} while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %c\n", read_buffer);

    while(!feof(picture)) 
    {
        //while(packet_index = 1){
        //Read from the file into our send buffer
        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

        //Send data through our socket 
        do
        {
            stat = write(socket, send_buffer, read_size);  
        }while (stat < 0);

        printf("Packet Number: %i\n",packet_index);
        printf("Packet Size Sent: %i\n",read_size);     
        printf(" \n");
        printf(" \n");

        packet_index++;  

        //Zero out our send buffer
        bzero(send_buffer, sizeof(send_buffer));
    }
    //fflush(stdout);
    cout<<"EXITING"<<endl;
    return 0;
}





 
// Mouse event handler. Called automatically by OpenCV when the user clicks in the GUI window.
void onMouse(int event, int x, int y, int, void*)
{
	// We only care about left-mouse clicks, not right-mouse clicks or mouse movement.
	if (event != CV_EVENT_LBUTTONDOWN)
	return;

	printf("x:%d\ty:%d\n",x,y);

	// Check if the user clicked on one of our GUI buttons.
	Point pt = Point(x,y);
	if (isPointInRect(pt, m_rcBtnAdd)) {
	printf("Start\n\n");
		KAPA = 1;
	}

	if (y > 630 && y < 720)
	{
		if (x > 330 && x < 420){
		// TO DO
			printf("pointer = [%d %d]",x,y);
			evaluate = 5;
			KAPA = 2;

		}
		if (x > 488 && x < 585){
		// TO DO
			printf("pointer = [%d %d]",x,y);
			evaluate = 4;
			KAPA = 2;
		}
		if (x > 651 && x < 735){
		// TO DO
			printf("pointer = [%d %d]",x,y);		
			evaluate = 3;
			KAPA = 2;
		}
		if (x > 806 && x < 900){
		// TO DO
			printf("pointer = [%d %d]",x,y);		
			evaluate = 2;
			KAPA = 2;
		}
		if (x > 965 && x < 1055){
		// TO DO	
			printf("pointer = [%d %d]",x,y);	
			evaluate = 1;
			KAPA = 2;
		}	
	}
}


// Get access to the webcam.
void initWebcam(VideoCapture &videoCapture, int cameraNumber)
{
    // Get access to the default camera.
    try {   // Surround the OpenCV call by a try/catch block so we can give a useful error message!
        videoCapture.open(cameraNumber);
    } catch (cv::Exception &e) {}
    if ( !videoCapture.isOpened() ) {
        cerr << "ERROR: Could not access the camera!" << endl;
        exit(1);
    }
    cout << "Loaded camera " << cameraNumber << "." << endl;
}



Rect drawString(Mat img, string text, Point coord, Scalar color, float fontScale = 0.8f, int thickness = 1, int fontFace = FONT_HERSHEY_COMPLEX)
{
    // Get the text size & baseline.
    int baseline=0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;
	
    // Adjust the coords for left/right-justified or top/bottom-justified.
    if (coord.y >= 0) {
        // Coordinates are for the top-left corner of the text from the top-left of the image, so move down by one row.
        coord.y += textSize.height;
    }
    else {
        // Coordinates are for the bottom-left corner of the text from the bottom-left of the image, so come up from the bottom.
        coord.y += img.rows - baseline + 1;
    }
    // Become right-justified if desired.
    if (coord.x < 0) {
        coord.x += img.cols - textSize.width + 1;
    }
	
    // Get the bounding box around the text.
    Rect boundingRect = Rect(coord.x, coord.y - textSize.height, textSize.width, baseline + textSize.height);
	
    // Draw anti-aliased text.
    putText(img, text, coord, fontFace, fontScale, color, thickness, CV_AA);
	
    // Let the user know how big their text is, in case they want to arrange things.
    return boundingRect;
}


Rect drawButton(Mat img, string text, Point coord, int minWidth = 0)
{
    int B = BORDER;
    Point textCoord = Point(coord.x + 2*B, coord.y + 2*B);
    // Get the bounding box around the text.
    Rect rcText = drawString(img, text, textCoord, CV_RGB(0,0,0));
    // Draw a filled rectangle around the text.
    Rect rcButton = Rect(rcText.x - 1.8*B, rcText.y - B, rcText.width + 3.2*B, rcText.height + 1.6*B);
    // Set a minimum button width.
    if (rcButton.width < minWidth)
        rcButton.width = minWidth;
    // Make a semi-transparent white rectangle.
    Mat matButton = img(rcButton);
    matButton += CV_RGB(255, 255, 255);
    // Draw a non-transparent white border.
    rectangle(img, rcButton, CV_RGB(200,200,200), 2, CV_AA);
	
    // Draw the actual text that will be displayed, using anti-aliasing.
    drawString(img, text, textCoord, CV_RGB(0,0,0));
	
    return rcButton;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	
	char name[100];
	printf("Please give your nickname:");
	scanf("%s",name);

	FILE *fp;
	fp = fopen("Evaluation_1.txt","a");

	//namedWindow(windowName,WINDOW_NORMAL);
	namedWindow(windowName, CV_WINDOW_NORMAL);
	setWindowProperty(windowName, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setMouseCallback(windowName, onMouse, 0);

	Mat image;
    image = imread("images/1.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Get OpenCV to automatically call my "onMouse()" function when the user clicks in the GUI window.
    	//setMouseCallback(windowName, onMouse, 0);
	// Create a GUI window for display on the screen.
    	imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	
	image = imread("images/2.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
    	imshow( windowName, image );                   // Show our image inside it.
	waitKey(8000);  // Wait for a keystroke in the window
	

	image = imread("images/3.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
    	imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	
	int h = 50;
	Scalar eyeColor = CV_RGB(0,255,255);
	Scalar helpColor = CV_RGB(255,0,0);
	float txtSize = 0.85;
	string help;
	help = "Please align your eyes with the circles.";
	
	VideoCapture videoCapture;
	
	// Allow the user to specify a camera number, since not all computers will be the same camera number.
    int cameraNumber = 0;   // Change this if you want to use a different camera device.
    
	// Get access to the webcam.
    initWebcam(videoCapture, cameraNumber);
	
	videoCapture.set(CV_CAP_PROP_FRAME_WIDTH, DESIRED_CAMERA_WIDTH);
    	videoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, DESIRED_CAMERA_HEIGHT);
	

	
	srand(time(NULL));
	char k, str[100], c[100];
	int i=rand();
	cout << "Put your face in the bounds of the rectangle:\n";
	cout << "Press 'c' to capture your face \n";
	
	int flag_exit = 0;
	char image_name[100];
	char name_int[100];
	
	int lolo = 20;

	while (1) 
	{
		// Grab the next camera frame. Note that you can't modify camera frames.
		Mat cameraFrame;
		videoCapture >> cameraFrame;
		if( cameraFrame.empty() ) {
		    cerr << "ERROR: Couldn't grab the next camera frame." << endl;
		    exit(1);
		}
		
		// Get a copy of the camera frame that we can draw onto.
        	Mat displayedFrame;
		cameraFrame.copyTo(displayedFrame);
		
		flip(displayedFrame,displayedFrame,1);
		
		Rect rcButton = Rect(350, 535,620 ,50);
		Mat matButton = displayedFrame(rcButton);
    		matButton += CV_RGB(255, 255, 255);
		//rectangle(displayedFrame, rcButton, CV_RGB(200,200,200), 2, CV_AA);
		drawString(displayedFrame, help, Point(370,550), helpColor, txtSize);  // Black shadow.
				

		// Draw light-blue anti-aliased circles for the 2 eyes.
		circle(displayedFrame, Point(590, 175), 12, CV_RGB(255,255,255), 2, CV_AA);
		circle(displayedFrame, Point(680,175), 12, CV_RGB(255,255,255), 2, CV_AA);
		// big rectangle
		rectangle(displayedFrame,cvPoint(510,60),cvPoint(750,350),CV_RGB(255,255,255),4,CV_AA);
	
		m_rcBtnAdd = drawButton(displayedFrame, "Start", Point(540, 600));
		//m_rcBtnDel = drawButton(displayedFrame, "Stop", Point(m_rcBtnAdd.x, m_rcBtnAdd.y + m_rcBtnAdd.height), m_rcBtnAdd.width);
		// Show the camera frame on the screen.
        	imshow(windowName, displayedFrame);
		
		k = cvWaitKey(lolo);
		if(k=='c') break;
		
		if (KAPA==1) 
		{
			// TO DO
			//printf("KAPA==1");
			flag_exit++;
			printf("%d\n",flag_exit);
			//help = "Please do not move your body for 5 seconds :)";
			lolo = 500;
			strcpy(image_name,"capture");
			sprintf(name_int,"%d",flag_exit);
			strcat(image_name,name_int);
			strcat(image_name,".jpg");
			// save image
			imwrite(image_name,cameraFrame);

		}
		
		if (flag_exit > 10) {
			break;
		}
		
	}
	
	videoCapture.release();

	//printf("\n\nServer opens\n\n");
	image = imread("images/4.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	imshow( windowName, image );                   // Show our image inside it.
	
	waitKey(5000);  // Wait for a keystroke in the window
	
	image = imread("images/5.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	imshow( windowName, image ); 
	// Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
        	
	image = imread("images/6.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	
	//////////////////////////////////////////////////
	//TODO client sent an message that the coffee reached to the initial position
	
	image = imread("images/7.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	imshow( windowName, image );                   // Show our image inside it.
	waitKey(20);
	
	int lole = 0;
	int seconds = 30;
	while(1)
	{
		printf("click\n");
		if(KAPA == 2){
			fprintf(fp,"%s\t",name);
			fprintf(fp,"%d\n",evaluate);	
			printf("evaluate = %d\n",evaluate);		
			break;
		}
		k = cvWaitKey(200);
		lole = lole + 200;
		printf("lole = %d\n",lole);
		if (lole > 200*5*seconds){
			fprintf(fp,"%s\t",name);	
			fprintf(fp,"%d\n",evaluate);
			break;
		}

	}
	
	// Wait for a keystroke in the window
	image = imread("images/8.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	    imshow( windowName, image );                   // Show our image inside it.
	waitKey(30000);  // Wait for a keystroke in the window
	
	
	fclose(fp);	
    return 0;

}
