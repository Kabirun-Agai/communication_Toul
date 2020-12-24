/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef STAMPDATA_H
#define STAMPDATA_H

#include <WinSock2.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <process.h>
#include "Socket.h"
#include "Command.h"
#include "PacketData.h"

#define NUM_OF_STAMP 8

/* �X�^���v�֘A�����i�[����\���� */
typedef struct StampData {
	int x; // �X�^���v��x���W
	int y; // �X�^���v��y���W
	int stamp_id; // �X�^���v��ID
} StampData;

/***** �p�u���b�N(�v���O��������Ăяo���\)�֐�********************************************/

void StampData_make(StampData* data, int x, int y, int stamp_id);

void StampData2PacketData(StampData* src, PacketData* dst);
void PacketData2StampData(PacketData* src, StampData* dst);

void StampData_clear(StampData* stamp_data);
void StampData_copy(StampData* src, StampData* dst);

// �X�^���v�f�[�^�̃e�L�X�g���\��
void StampData_print(StampData* stamp_data);

/***** �v���C�x�[�g(�O������̌Ăяo����z�肵�Ȃ�)*******************************************/

// �V���A���C�Y�֐�
void StampData_serialize(StampData* stamp_data, PacketData* packet_data);

// �f�V���A���C�Y�֐�
void StampData_deserialize(PacketData* packet_data, StampData* stamp_data);

#endif /* STAMPDATA_H */