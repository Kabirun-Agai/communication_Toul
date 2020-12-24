/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef USERDATA_H
#define USERDATA_H

#include "Socket.h"
#include "PacketData.h"
#include "Command.h"

#define MAX_USERNAME_LEN 64 // bytes

// ���[�U�[�f�[�^
typedef struct UserData {
	SOCKET sock;            // �T�[�o�[���邢�̓N���C�A���g�ƒʐM���邽�߂̃\�P�b�g
	char user_name[MAX_USERNAME_LEN]; // ���[�U�[��
} UserData;

/***** �p�u���b�N(�v���O��������Ăяo���\)�֐�********************************************/

/* ���[�U������UserData�𐶐����� */
// idx = -1, sock = -1�ɃZ�b�g�����
void UserData_make(char* user_name, UserData* user_data); 

// cmd = CMD_USERDATA_REGIST or CMD_USERDATA_UNREGIST
void UserData2PacketData(unsigned char cmd, UserData* src, PacketData* dst);
void PacketData2UserData(PacketData* src, UserData* dst);

void UserData_clear(UserData* user_data);
void UserData_copy(UserData* src, UserData* dst);

// ���[�U�[�f�[�^�̃e�L�X�g���\��
void UserData_print(UserData* data);

/***** �v���C�x�[�g(�O������̌Ăяo����z�肵�Ȃ�)*******************************************/

// UserData�\���̂���z��ւ̃V���A���C�Y�֐�
void UserData_serialize(UserData* user_data, PacketData* packet_data);

// �z�񂩂�User�\���̂ւ̃f�V���A���C�Y�֐�
void UserData_deserialize(PacketData* packet_data, UserData* user_data);




#endif /* USERDATA_H */