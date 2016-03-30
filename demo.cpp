// Copyright 2014-2015 Isis Innovation Limited and the authors of gSLICr

#include <time.h>
#include <stdio.h>
#include <algorithm>
#include "gSLICr_Lib/gSLICr.h"
#include "NVTimer.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int global_no_sprpxls;

void load_image(const Mat& inimg, gSLICr::UChar4Image* outimg)
{
	gSLICr::Vector4u* outimg_ptr = outimg->GetData(MEMORYDEVICE_CPU);
	for (int y = 0; y < outimg->noDims.y;y++)
		for (int x = 0; x < outimg->noDims.x; x++)
		{
			int idx = x + y * outimg->noDims.x;
			outimg_ptr[idx].b = inimg.at<Vec3b>(y, x)[0];
			outimg_ptr[idx].g = inimg.at<Vec3b>(y, x)[1];
			outimg_ptr[idx].r = inimg.at<Vec3b>(y, x)[2];
		}
}

void load_image(const gSLICr::UChar4Image* inimg, Mat& outimg)
{
	const gSLICr::Vector4u* inimg_ptr = inimg->GetData(MEMORYDEVICE_CPU);
	for (int y = 0; y < inimg->noDims.y; y++)
		for (int x = 0; x < inimg->noDims.x; x++)
		{
			int idx = x + y * inimg->noDims.x;
			outimg.at<Vec3b>(y, x)[0] = inimg_ptr[idx].b;
			outimg.at<Vec3b>(y, x)[1] = inimg_ptr[idx].g;
			outimg.at<Vec3b>(y, x)[2] = inimg_ptr[idx].r;
		}
}

struct bgr {
	float b;
	float g;
	float r;
	bgr(){
		b=0;
		g=0;
		r=0;
	}
};

struct apple_info_t {
	/*TODO: Instead of a static array make it a dynamic one with the number of superpixels being calculated at the run time*/
	/*int spxl_len[768];*/
	int* spxl_len; 
	/*bgr bgr_arr[768];*/
	bgr* bgr_arr;
	apple_info_t(){

		
		spxl_len = (int*)malloc(sizeof(int)*global_no_sprpxls);
		bgr_arr = (bgr*)malloc(sizeof(bgr)*global_no_sprpxls);
		for(int i = 0; i < global_no_sprpxls; i++) {
			spxl_len[i]=0;
			bgr_arr[i].b=bgr_arr[i].g=bgr_arr[i].r=0;
		}
	}
};

/*TODO: Feed Pravakar's code here*/
vector<int> getRedApples_sprpxls(apple_info_t* apple_info) {
	vector<int> qualified_sprpxls;
	/*TODO For all the super pixel find those that qualifies for the job*/
	for(int i = 0; i < global_no_sprpxls; i++) {
		
		int red = apple_info->bgr_arr[i].r;
		int green = apple_info->bgr_arr[i].g;
		int blue = apple_info->bgr_arr[i].b;
		
		int min = 2000;
		int max = -1;	
		
		if(green >= red && green >=blue)
			max = green;
		else if(blue>=red)
			max = blue;
		else
			max = red;

		if(red <= blue && blue <= green)
			min = red;
		else if(blue<=green)
			min = blue;
		else
			min = green;


		if(max==red) {

			/* For the dark red apples */
		
			//bool cond1 = ((red - blue < 35 || red - green < 35) && red > 200) || red - blue < 20 || red - green < 20 || red + green + blue > 680;
			//bool cond1 = ((red - blue < 35 || red - green < 35) && red > 100) || red - blue < 20 || red - green < 20 || red + green + blue > 680;
			//if(!(((red - blue < 35 || red - green < 35 ) && red > 200) || red - blue < 20|| red - green < 20 || red + green + blue > 680))
	 		//	if(!((green + blue > 270|| (green - blue < 40 && red - green < 50 && red > 79) ) && !cond1))
			
	
			if(red-blue>15 && red-green>15)
				qualified_sprpxls.push_back(i);
			
		
		
		
			/* For the mixed green apples */
			//if(!(red -green >35||green <blue||red -blue<20|| green -blue<20|| red+green+blue>680 ))
			//	qualified_sprpxls.push_back(i);/*TODO: only push if it is not there*/

	    }	
		
		/* For the mixed green apples */
		
		//if(max==green) {
		//	if(!(green-red>15|| red<blue|| red+green +blue<270||red-blue<25||green -blue<25 || red+green+blue>680))
		//		qualified_sprpxls.push_back(i);
		//}
	}
	return qualified_sprpxls;
}



int main(int argc, char **argv)
{
	// VideoCapture cap(argv[1]);
	//VideoCapture cap("../frames/frames_compressed/plots/p2/1%03d.jpg");
	//VideoCapture cap("../frames/seg0.jpg");
	//VideoCapture cap("../frames/frames_compressed/VIRB0072.MP4");
	VideoCapture cap("../frames/apple_video_frames_100/600.jpg");


	if (!cap.isOpened())
	{
		cerr << "unable to open camera!\n";
		return -1;
	}
	// gSLICr settings
	gSLICr::objects::settings my_settings;
	my_settings.img_size.x = 800;
	my_settings.img_size.y = 600;
	my_settings.no_segs = 500;
	my_settings.spixel_size = 20 ;
	my_settings.coh_weight = 0.6f;
	my_settings.no_iters = 5;
	my_settings.color_space = gSLICr::XYZ; // gSLICr::CIELAB for Lab, or gSLICr::RGB for RGB
	my_settings.seg_method = gSLICr::GIVEN_SIZE; // or gSLICr::GIVEN_NUM for given number
	my_settings.do_enforce_connectivity = true; // wheter or not run the enforce connectivity step


	// Find the value of the global_no_superpixles
	global_no_sprpxls = (my_settings.img_size.x * my_settings.img_size.y)/(my_settings.spixel_size * my_settings.spixel_size);
	cout<<"\n Total number of Superpixels: "<<global_no_sprpxls<<endl;

	// Instantiate a core_engine with the settings specified by us above.
	gSLICr::engines::core_engine* gSLICr_engine = new gSLICr::engines::core_engine(my_settings);

	// gSLICr takes gSLICr::UChar4Image as input and output
	gSLICr::UChar4Image* in_img = new gSLICr::UChar4Image(my_settings.img_size, true, true);
	gSLICr::UChar4Image* out_img = new gSLICr::UChar4Image(my_settings.img_size, true, true);

	Size s(my_settings.img_size.x, my_settings.img_size.y);
	Mat oldFrame, frame;
	Mat boundry_draw_frame; boundry_draw_frame.create(s, CV_8UC3);
	Mat apple_seg_mat_image; apple_seg_mat_image.create(s, CV_8UC3);
	StopWatchInterface *my_timer; sdkCreateTimer(&my_timer);

	int key; int save_count = 0;
	int frame_number = -1;
	float sum = 0;

	while (cap.read(oldFrame))
	{
		frame_number++;
		//if(frame_number<1200)
		//	continue;
	
		apple_info_t apple_info;

		resize(oldFrame, frame, s);
		load_image(frame, in_img);

	        sdkResetTimer(&my_timer);
		sdkStartTimer(&my_timer);

		/* This process takes the whole time. Main processing code.
		   in_image is a pointer to UChar4Image */
		gSLICr_engine->Process_Frame(in_img, frame_number);
		sdkStopTimer(&my_timer);
        	
		//cout<<"\rsegmentation in:["<<sdkGetTimerValue(&my_timer)<<"]ms"<<flush;
		sum+=sdkGetTimerValue(&my_timer);
		
		//gSLICr_engine->slic_seg_engine->spixel_map->UpdateHostFromDevice();
		//gSLICr::objects::spixel_info *aha = gSLICr_engine->slic_seg_engine->spixel_map->GetData(MEMORYDEVICE_CPU) ;
		//cout<<(aha+1)->center<<endl;	

		gSLICr_engine->Draw_Segmentation_Result(out_img);

		load_image(out_img, boundry_draw_frame);
		imshow("superpixels", boundry_draw_frame);
		//waitKey(0);
		
		gSLICr::IntImage* myIntImage_ptr = gSLICr_engine->slic_seg_engine->idx_img;
		myIntImage_ptr->UpdateHostFromDevice();
		int* int_ptr = myIntImage_ptr->GetData(MEMORYDEVICE_CPU);	
		
		int nodim_x = myIntImage_ptr->noDims.x;//width is x
		int nodim_y = myIntImage_ptr->noDims.y;//height is y	

		vector<int> int_vector(int_ptr, int_ptr+nodim_x*nodim_y-1);
		//cout<<"max element: "<< *max_element(int_vector.begin(), int_vector.end())  <<endl;
		
			
		
		const gSLICr::Vector4u* in_img_ptr = in_img->GetData(MEMORYDEVICE_CPU);
			

		for(int i = 0; i < nodim_y*nodim_x; ++i){	
			apple_info.bgr_arr[int_ptr[i]].b += in_img_ptr[i].b;
			apple_info.bgr_arr[int_ptr[i]].g += in_img_ptr[i].g;
			apple_info.bgr_arr[int_ptr[i]].r += in_img_ptr[i].r;
			apple_info.spxl_len[int_ptr[i]]++;
		}	
	
		for(int i = 0; i < global_no_sprpxls; i++){
			apple_info.bgr_arr[i].b /= apple_info.spxl_len[i];
			apple_info.bgr_arr[i].g /= apple_info.spxl_len[i];
			apple_info.bgr_arr[i].r /= apple_info.spxl_len[i];	
			/* spxl[i]=(r, g, b)  */
			//cout<<"\n spxl["<<i<<"]=("<<apple_info.bgr_arr[i].r<<", "<<apple_info.bgr_arr[i].g<<", "<<apple_info.bgr_arr[i].b<<")"<<endl;
		}

		

		/* ids of the superpixels that qualifies for the job*/
		vector<int> qualified_sprpxls = getRedApples_sprpxls(&apple_info); 
		/*
		for(int i = 0; i < qualified_sprpxls.size(); ++i) {
			cout<<qualified_sprpxls[i]<<endl;
		}
		*/
		/*create one clone of in_image and draw the required superpixel. basically blacken out everything else*/
		gSLICr::UChar4Image* appleseg_img = new gSLICr::UChar4Image(my_settings.img_size, true, true);
		load_image(frame, appleseg_img);	

		gSLICr::Vector4u* appleseg_img_ptr = appleseg_img->GetData(MEMORYDEVICE_CPU);
			
		for(int i = 0; i < nodim_x*nodim_y; ++i) {
			if(find(qualified_sprpxls.begin(), qualified_sprpxls.end(), int_ptr[i]) != qualified_sprpxls.end()) {
				/*This pixel is qualified to be an apple. Do nothing for now */
			}
			else {
				//This pixel is not qualified to be an apple. We can make it complete dark
				appleseg_img_ptr[i].b = 0;
				appleseg_img_ptr[i].g = 0;
				appleseg_img_ptr[i].r = 0;
			}
		}

		load_image(appleseg_img, apple_seg_mat_image);
		imshow("apple  segmentation", apple_seg_mat_image);
		waitKey(0);

		cout<<"\n noDims: "<<myIntImage_ptr->noDims<<endl;
		cout<<"\n int_ptr: "<<int_ptr[2000]<<endl;
		key = waitKey(1);
		if (key == 27) break;
		else if (key == 's')
		{
			char out_name[100];
			sprintf(out_name, "seg_%04i.pgm", save_count);
			gSLICr_engine->Write_Seg_Res_To_PGM(out_name);
			sprintf(out_name, "edge_%04i.png", save_count);
			imwrite(out_name, boundry_draw_frame);
			sprintf(out_name, "img_%04i.png", save_count);
			imwrite(out_name, frame);
			printf("\nsaved segmentation %04i\n", save_count);
			save_count++;
		}

		//frame_number++;
		cout<<"\t Frame Number: "<<frame_number<<"\n"<<endl;
		free(apple_info.spxl_len);
		free(apple_info.bgr_arr);
			
	}
	cout<<"\t Average Per Frame: "<<sum/frame_number<<"\n"<<endl;
	//destroyAllWindows();
    return 0;
}
