// Copyright 2014-2015 Isis Innovation Limited and the authors of gSLICr

#pragma once
#include "gSLICr_seg_engine.h"
#include <iostream>

using namespace std;
using namespace gSLICr;
using namespace gSLICr::objects;
using namespace gSLICr::engines;


seg_engine::seg_engine(const objects::settings& in_settings)
{
	gSLICr_settings = in_settings;
}

seg_engine::~seg_engine()
{
	if (source_img != NULL) delete source_img;
	if (cvt_img != NULL) delete cvt_img;
	if (idx_img != NULL) delete idx_img;
	if (spixel_map != NULL) delete spixel_map;
}

void seg_engine::Perform_Segmentation(UChar4Image* in_img, int frame_number)
{
	source_img->SetFrom(in_img, ORUtils::MemoryBlock<Vector4u>::CPU_TO_CUDA);
	Cvt_Img_Space(source_img, cvt_img, gSLICr_settings.color_space);

	if(frame_number == 0)
		Init_Cluster_Centers();
	Find_Center_Association();


		spixel_map->UpdateHostFromDevice();
		spixel_info *aha = spixel_map->GetData(MEMORYDEVICE_CPU) ;
		
			for(int j=0; j<768; j++)
			cout<<"\t spixel "<<j<<"--> "<<(aha+j)->center<<endl;
			cout<<"--------------------------------------------------"<<endl;		


	for (int i = 0; i < gSLICr_settings.no_iters; i++)
	{
		
		Update_Cluster_Center();
		Find_Center_Association();
		spixel_map->UpdateHostFromDevice();
		aha = spixel_map->GetData(MEMORYDEVICE_CPU) ;
		for(int j=0; j<768; j++)
			cout<<"\t spixel "<<j<<"--> "<<(aha+j)->center<<endl;
		cout<<"--------------------------------------------------"<<endl;	
	}

	if(gSLICr_settings.do_enforce_connectivity) Enforce_Connectivity();
	cudaThreadSynchronize();
}
