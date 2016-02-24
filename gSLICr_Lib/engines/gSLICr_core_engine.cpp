// Copyright 2014-2015 Isis Innovation Limited and the authors of gSLICr

#pragma once
#include "gSLICr_core_engine.h"
#include <fstream>
#include <iostream>

using namespace gSLICr;
using namespace std;

/*
	The core engine class is nothing but a wrapper to a slic_seg_engine
	variable(kept as a private member).
	The main function of this wrapper class is via the Process_Frame fucntion
	which initiates the Perform_Segmentation() function for the slic_seg_engine
	private variable.
	seg_engine* slic_seg_engine = new seg_engine_GPU();
*/
gSLICr::engines::core_engine::core_engine(const objects::settings& in_settings)
{
	slic_seg_engine = new seg_engine_GPU(in_settings);
}

gSLICr::engines::core_engine::~core_engine()
{
		delete slic_seg_engine;
}

void gSLICr::engines::core_engine::Process_Frame(UChar4Image* in_img, int frame_number)
{
	/* The private variable that we intialized at the time of constructor
	calling is now used to perform segmentation on the in_image which is
	provided as the input.
	*/
	slic_seg_engine->Perform_Segmentation(in_img, frame_number);
}

const IntImage * gSLICr::engines::core_engine::Get_Seg_Res()
{
	return slic_seg_engine->Get_Seg_Mask();
}

void gSLICr::engines::core_engine::Draw_Segmentation_Result(UChar4Image* out_img)
{
	slic_seg_engine->Draw_Segmentation_Result(out_img);
}

void gSLICr::engines::core_engine::Write_Seg_Res_To_PGM(const char* fileName)
{
	const IntImage* idx_img = slic_seg_engine->Get_Seg_Mask();
	int width = idx_img->noDims.x;
	int height = idx_img->noDims.y;
	const int* data_ptr = idx_img->GetData(MEMORYDEVICE_CPU);

	ofstream f(fileName, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	f << "P5\n" << width << " " << height << "\n65535\n";
	for (int i = 0; i < height * width; ++i)
	{
		ushort lable = (ushort)data_ptr[i];
		ushort lable_buffer = (lable << 8 | lable >> 8);
		f.write((const char*)&lable_buffer, sizeof(ushort));
	}
	f.close();
}
