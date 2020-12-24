/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef MOUSEDATA_H
#define MOUSEDATA_H

#include <WinSock2.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <process.h>
#include "Socket.h"
#include "PacketData.h"
#include "Command.h"

/* �}�E�X�֘A�����i�[����\���� */
typedef struct MouseData {
	int x; // ���݂̃}�E�X�|�C���^�ʒu��x���W
	int y; // ���݂̃}�E�X�|�C���^�ʒu��y���W
	bool lbutton_down; // ���{�^����������Ă����true�C�����łȂ����false
	bool dragged; // �h���b�O���ł����true, �����łȂ����false
	int color_idx; // Command.h�Q��
} MouseData;

/***** �p�u���b�N(�v���O��������Ăяo���\)�֐�********************************************/
void MouseData_make(MouseData* mouse_data, int x, int y, bool lbutton_down, bool dragged, int color_id);

void MouseData2PacketData(MouseData* src, PacketData* dst);
void PacketData2MouseData(PacketData* src, MouseData* dst);

void MouseData_clear(MouseData* mouse_data);
/* �}�E�X�f�[�^�̃R�s�[ */
/* ��1����(src)���2����(dst)�ɃR�s�[ */
void MouseData_copy(MouseData* src, MouseData* dst);

/* �}�E�X�f�[�^�̃e�L�X�g���\�� */
void MouseData_print(MouseData* mouse_data);

/***** �v���C�x�[�g(�O������̌Ăяo����z�肵�Ȃ�)*******************************************/

// �V���A���C�Y�֐�
void MouseData_serialize(MouseData* mouse_data, PacketData* base_data);
// �f�V���A���C�Y�֐�
void MouseData_deserialize(PacketData* base_data, MouseData* mouse_data);

#endif /* MOUSEDATA_H */