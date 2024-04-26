#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
using namespace std;
using namespace cv;

int main(){
	ofstream image;
	image.open("image.jpg");

	if(image.is_open()){
		//
		image << "P3" << endl;
		image << "256 256" << endl;
		image << "255" << endl;

		for(int y=0; y<256; y++){
			for(int x=0; x<256; x++){
				image << rand() % 255 << " " << rand() % 255 << " " << rand() % 255 << endl;
			}
		}
	}
	image.close();

	// Reading the image file
 	Mat image = imread(“C:/users/downloads/Untitled design.jpg”, IMREAD_grayscale);
	//Error Handling
	if(image.empty()){
		cout << "Could not open or find the image" << endl;
		return -1;
	} else {
		cout << "Image loaded successfully" << endl;
	}

	// Show Image inside a window with 
	// the name provided 
	imshow("Window Name", image); 

	// Wait for any keystroke
	waitKey(0); 
	return 0;
}
