/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef COMMAND_H
#define COMMAND_H

#include <Winsock2.h>
#include <Windows.h>
#include <opencv2/highgui/highgui_c.h> 
#include <opencv2/opencv.hpp>
#include "cvPutTextJP_Win2.h"
#include "Socket.h"

/* color_idx */
// 0 white
// 1 olive
// 2 yellow
// 3 fuchsia
// 4 silver
// 5 aqua
// 6 lime
// 7 red
// 8 gray
// 9 blue
// 10 green
// 11 purple
// 12 black
// 13 navy
// 14 teal
// 15 maroon

#define NUM_OF_COLOR 16

extern cv::Scalar color[NUM_OF_COLOR];

// �N���C�A���g����T�[�o�ɑ��M����R�}���h�i�P�o�C�g�j
typedef enum {
	CMD_USERDATA_REGIST = 0x00,   // ���[�U�o�^�v���R�}���h
	CMD_USERDATA_UNREGIST = 0x01, // ���[�U�o�^�����v���R�}���h
	DAT_CHATDATA = 0x02, // ���b�Z�[�W�f�[�^
	DAT_MOUSEDATA = 0x03,   // �}�E�X�|�C���^�f�[�^
	DAT_STAMPDATA = 0x04,   // �X�^���v�f�[�^
	DAT_IMAGEDATA = 0x05,   // jpeg�摜�f�[�^
	CMD_ERROR = 0xff,
	// ����M����f�[�^��ǉ�����ɂ́C�����ɐV�����R�}���h��ǉ�����D
} COMMAND;

#endif /* COMMAND_H */