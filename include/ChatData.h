/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef CHATDATA_H
#define CHATDATA_H

#include <process.h>
#include "Socket.h"
#include "Command.h"
#include "UserData.h"
#include "PacketData.h"

#define MAX_MESSAGE_LEN 128 // bytes

/* ChatData�\���� */
typedef struct ChatData {
	int color_idx;                    // �F�C���f�b�N�X(Command.h�Q��)
	char user_name[MAX_USERNAME_LEN]; // ���[�U��
	char message[MAX_MESSAGE_LEN];    // ���b�Z�[�W
} ChatData;

/***** �p�u���b�N(�v���O��������Ăяo���\)�֐�********************************************/

// �FIDX, ���[�U��, ���b�Z�[�W����ChatData���\�z����
void ChatData_make(ChatData* data, int color_idx, char* user_name, char* message);

// PacketData�ւ̕ϊ��֐�
void ChatData2PacketData(ChatData* src, PacketData* dst);

// PacketData����̕ϊ��֐�
void PacketData2ChatData(PacketData* src, ChatData* dst);

void ChatData_clear(ChatData* chat_data);
void ChatData_copy(ChatData* src, ChatData* dst);

// ChatData�̏�����ʂɕ\��
void ChatData_print(ChatData* chat_data);

/***** �v���C�x�[�g(�O������̌Ăяo����z�肵�Ȃ�)*******************************************/

// ChatData���V���A���C�Y
void ChatData_serialize(ChatData* chat_data, PacketData* packet_data);

// ChatData���f�V���A���C�Y
void ChatData_deserialize(PacketData* packet_data, ChatData* chat_data);

#endif /* CHATDATA_H */