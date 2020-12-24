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
#define IMAGE_C 3    // �`�����l��(�J���[�摜�̏ꍇ��3�`�����l���C�O���[�摜�̏ꍇ��1�`�����l���j

#define MAX_FILE_PATH 2048

/* �摜�֘A�����i�[����\���� */
typedef struct ImageData {
	cv::Mat image; // [IMAGE_W * IMAGE_H * IMAGE_C]; // jpeg���k�摜�̃o�C�i���f�[�^���i�[�D����M�ɗ��p�D
} ImageData;

/***** �p�u���b�N(�v���O��������Ăяo���\)�֐�********************************************/

void ImageData_init(ImageData* image_data);
void ImageData_release(ImageData* image_data);

void ImageData_make(ImageData* image_data, cv::Mat image);

void ImageData2PacketData(ImageData* src, PacketData* dst);
void PacketData2ImageData(PacketData* src, ImageData* dst);

// �u���[�摜�ɂ���
void ImageData_clear(ImageData* image_data);
void ImageData_copy(ImageData* src, ImageData* dst);

// �摜�f�[�^�̏��i�e�L�X�g�j�\��
void ImageData_print(ImageData* image_data);

/***** �v���C�x�[�g(�O������̌Ăяo����z�肵�Ȃ�)*******************************************/

void ImageData_serialize(ImageData* image_data, PacketData* packet_data);
void ImageData_deserialize(PacketData* packet_data, ImageData* image_data);

#endif /* IMAGEDATA_H */