// g++  main.cpp -o robocoffee `pkg-config --cflags opencv`   `pkg-config --libs opencv`

const int DESIRED_CAMERA_WIDTH = 640;
const int DESIRED_CAMERA_HEIGHT = 480;
const char *windowName = "RoboCoffee";   // Name shown in the GUI window.
const int BORDER = 8;  // Border between GUI elements to the edge of the image.

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
#include "opencv2/opencv.hpp"
#include<arpa/inet.h>   
#include<unistd.h>
#include<fstream>
#include<errno.h>

using namespace cv;
using namespace std;

Rect m_rcBtnAdd;
//Rect m_rcBtnDel;

int KAPA = 0;




int send_image(int socket){
	
	FILE *picture;
	int size, read_size, stat, packet_index;
	char send_buffer[10240], read_buffer[256];
	packet_index = 1;
	
	picture = fopen("capture.jpg", "r");
	printf("Getting Picture Size\n");   
	
	if(picture == NULL) {
        printf("Error Opening Image File"); } 
	
	fseek(picture, 0, SEEK_END);
	size = ftell(picture);
	fseek(picture, 0, SEEK_SET);
	printf("Total Picture size: %i\n",size);
	
	//Send Picture Size
	printf("Sending Picture Size\n");
	write(socket, (void *)&size, sizeof(int));
	
	//Send Picture as Byte Array
	printf("Sending Picture as Byte Array\n");
	
	do { //Read while we get errors that are due to signals.
		stat=read(socket, &read_buffer , 255);
		printf("Bytes read: %i\n",stat);
	} while (stat < 0);
	
	printf("Received data in socket\n");
	printf("Socket data: %c\n", read_buffer);
	
	while(!feof(picture)) {
		//while(packet_index = 1){
		//Read from the file into our send buffer
		read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);
		
		//Send data through our socket 
		do{
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
}








bool isPointInRect(const Point pt, const Rect rc)
{
    if (pt.x >= rc.x && pt.x <= (rc.x + rc.width - 1))
        if (pt.y >= rc.y && pt.y <= (rc.y + rc.height - 1))
            return true;
	
    return false;
}


// Mouse event handler. Called automatically by OpenCV when the user clicks in the GUI window.
void onMouse(int event, int x, int y, int, void*)
{
    
	//printf("\n\nonMouse\n\n");
	
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



Rect drawString(Mat img, string text, Point coord, Scalar color, float fontScale = 0.6f, int thickness = 1, int fontFace = FONT_HERSHEY_COMPLEX)
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
    Point textCoord = Point(coord.x + B, coord.y + B);
    // Get the bounding box around the text.
    Rect rcText = drawString(img, text, textCoord, CV_RGB(0,0,0));
    // Draw a filled rectangle around the text.
    Rect rcButton = Rect(rcText.x - B, rcText.y - B, rcText.width + 2*B, rcText.height + 2*B);
    // Set a minimum button width.
    if (rcButton.width < minWidth)
        rcButton.width = minWidth;
    // Make a semi-transparent white rectangle.
    Mat matButton = img(rcButton);
    matButton += CV_RGB(90, 90, 90);
    // Draw a non-transparent white border.
    rectangle(img, rcButton, CV_RGB(200,200,200), 1, CV_AA);
	
    // Draw the actual text that will be displayed, using anti-aliasing.
    drawString(img, text, textCoord, CV_RGB(10,55,20));
	
    return rcButton;
}




int main(int argc, char *argv[])
{
	
	
		
	namedWindow(windowName);
	setMouseCallback(windowName, onMouse, 0);
	
	
	Mat image;
    	image = imread("images/1.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Get OpenCV to automatically call my "onMouse()" function when the user clicks in the GUI window.
    //setMouseCallback(windowName, onMouse, 0);
	// Create a GUI window for display on the screen.
    	imshow( windowName, image );                   // Show our image inside it.
	waitKey(3000);  // Wait for a keystroke in the window
	
	image = imread("images/2.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
    imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	

	image = imread("images/3.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
    imshow( windowName, image );                   // Show our image inside it.
	waitKey(3000);  // Wait for a keystroke in the window
	
	int h = 50;
	Scalar eyeColor = CV_RGB(0,255,255);
	float txtSize = 0.45;
	string help;
	help = "Try to align your face relative to the eye-circles and the big rectangle";
	
	VideoCapture videoCapture;
	
	// Allow the user to specify a camera number, since not all computers will be the same camera number.
    int cameraNumber = 0;   // Change this if you want to use a different camera device.
    //if (argc > 1) {
    //    cameraNumber = atoi(argv[1]);
    //}
	
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
	
	while (1) {
		
		

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
		
		drawString(displayedFrame, help, Point(20,450), eyeColor, txtSize);  // Black shadow.
		
		// Draw light-blue anti-aliased circles for the 2 eyes.
		circle(displayedFrame, Point(320, 120), 6, eyeColor, 1, CV_AA);
		circle(displayedFrame, Point(380,120), 6, eyeColor, 1, CV_AA);
		// big rectangle
		rectangle(displayedFrame,cvPoint(280,60),cvPoint(410,200),eyeColor,2,CV_AA);
	
		m_rcBtnAdd = drawButton(displayedFrame, "Start", Point(BORDER, BORDER));
		//m_rcBtnDel = drawButton(displayedFrame, "Stop", Point(m_rcBtnAdd.x, m_rcBtnAdd.y + m_rcBtnAdd.height), m_rcBtnAdd.width);
		// Show the camera frame on the screen.
        	imshow(windowName, displayedFrame);
		
		k = cvWaitKey(20);
		if(k=='c') break;
		
		if (KAPA==1) {
			// TO DO
			//printf("KAPA==1");
			flag_exit++;
			printf("%d\n",flag_exit);
			help = "Please do not move your body for 5 seconds :)";

			// save image
			imwrite("capture.jpg",displayedFrame);

			////////////////////////////////////////
			// CLIENT SENT THE PHOTOS

			int socket_desc , new_socket , c, read_size,buffer = 0;
			struct sockaddr_in server , client;
			char *readin;

			//Create socket
			socket_desc = socket(AF_INET , SOCK_STREAM , 0);
			if (socket_desc == -1)
			{
				printf("Could not create socket");
			}

			//Prepare the sockaddr_in structure
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons( 1234 );

			//Bind
			if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
			{
				puts("bind failed");
				return 1;
			}

			puts("bind done");

			//Listen
			listen(socket_desc , 3);
		
			//Accept and incoming connection
			puts("Waiting for incoming connections...");
			c = sizeof(struct sockaddr_in);

			if((new_socket = accept(socket_desc, (struct sockaddr *)&client,(socklen_t*)&c))){
				puts("Connection accepted");
			}

			fflush(stdout);

			if (new_socket<0)
			{
				perror("Accept Failed");
				return 1;
			}

			send_image(new_socket);


			close(socket_desc);
			fflush(stdout);			

			



			////////////////////////////////////////

		}
		
		if (flag_exit > 1) {
			break;
		}
		
	}
	
	videoCapture.release();
	
	
	//printf("\n\nServer opens\n\n");
	image = imread("images/4.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	

	//////////////////////////////////////////////////////////
	// SERVER RECIEVE MESSAGE

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) printf("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	      printf("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) printf("ERROR on accept");
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) printf("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) printf("ERROR writing to socket");
	close(newsockfd);
	close(sockfd);



	//////////////////////////////////////////////////////////
		
	// connection close
	
	image = imread("images/5.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	    imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	
	image = imread("images/6.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	    imshow( windowName, image );                   // Show our image inside it.
	waitKey(20000);  // Wait for a keystroke in the window
	
	image = imread("images/7.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	    imshow( windowName, image );                   // Show our image inside it.
	waitKey(5000);  // Wait for a keystroke in the window
	
	image = imread("images/8.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
	// Create a GUI window for display on the screen.
	    imshow( windowName, image );                   // Show our image inside it.
	waitKey(0);  // Wait for a keystroke in the window
	
	
	
    return 0;

}
