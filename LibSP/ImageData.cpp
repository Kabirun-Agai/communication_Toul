/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "ImageData.h"

void ImageData_init(ImageData* image_data) {
	image_data->image.create(cv::Size(IMAGE_W, IMAGE_H), CV_8UC3);
}

void ImageData_release(ImageData* image_data) {
	image_data->image.release();
}

void ImageData_make(ImageData* image_data, cv::Mat image) {
	image.copyTo(image_data->image);
}

void ImageData2PacketData(ImageData* src, PacketData* dst) {
	dst->cmd = DAT_IMAGEDATA;
	dst->size = -1;
	ImageData_serialize(src, dst);
}

void PacketData2ImageData(PacketData* src, ImageData* dst) {
	ImageData_deserialize(src, dst);
}

void ImageData_clear(ImageData* image_data) {
	// ブルー画像
	image_data->image = cv::Scalar(255, 0, 0); 
}

void ImageData_copy(ImageData* src, ImageData* dst) {
	src->image.copyTo(dst->image);
}

void ImageData_print(ImageData* image_data) {
	fprintf(stderr, "ImageData_print: (w, h) = (%d, %d)\n", image_data->image.cols, image_data->image.rows);
}

/* シリアライズ関数 */
void ImageData_serialize(ImageData* image_data, PacketData* packet_data) {
	std::vector<uchar> buff;
	std::vector<int> param = std::vector<int>(2);
	param[0] = cv::IMWRITE_JPEG_QUALITY;
	param[1] = 95; //default(95) 0-100
	
	cv::imencode(".jpg", image_data->image, buff, param);
	memcpy(packet_data->binary, &buff[0], buff.size());
	packet_data->size = (int)buff.size();
}

/* デシリアライズ関数 */
void ImageData_deserialize(PacketData* packet_data, ImageData* image_data) {
	std::vector<uchar> buff;
	buff.assign(packet_data->binary, packet_data->binary + packet_data->size);
	cv::Mat decoded_image = cv::imdecode(cv::Mat(buff), cv::IMREAD_COLOR);
	decoded_image.copyTo(image_data->image);
}

