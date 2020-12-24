/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <WinSock2.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <process.h>
#include "Socket.h"
#include "Command.h"
#include "PacketData.h"

#define IMAGE_W 640
#define IMAGE_H 480
#define IMAGE_C 3    // チャンネル(カラー画像の場合は3チャンネル，グレー画像の場合は1チャンネル）

#define MAX_FILE_PATH 2048

/* 画像関連情報を格納する構造体 */
typedef struct ImageData {
	cv::Mat image; // [IMAGE_W * IMAGE_H * IMAGE_C]; // jpeg圧縮画像のバイナリデータを格納．送受信に利用．
} ImageData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/

void ImageData_init(ImageData* image_data);
void ImageData_release(ImageData* image_data);

void ImageData_make(ImageData* image_data, cv::Mat image);

void ImageData2PacketData(ImageData* src, PacketData* dst);
void PacketData2ImageData(PacketData* src, ImageData* dst);

// ブルー画像にする
void ImageData_clear(ImageData* image_data);
void ImageData_copy(ImageData* src, ImageData* dst);

// 画像データの情報（テキスト）表示
void ImageData_print(ImageData* image_data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

void ImageData_serialize(ImageData* image_data, PacketData* packet_data);
void ImageData_deserialize(PacketData* packet_data, ImageData* image_data);

#endif /* IMAGEDATA_H */