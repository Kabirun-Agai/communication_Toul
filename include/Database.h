/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef DATABASE_H
#define DATABASE_H

#include <Winsock2.h>
#include <Windows.h>
#include <conio.h>
#include <wincon.h>
#include <process.h>
#include <stdio.h>

#include "Socket.h"
#include "UserData.h"

#define MAX_USERNUM 10 // �ڑ��\�ő僆�[�U�[��
#define MAX_NUM_IN_ROW 10 // Database_print�֐��ɂ�����1�s������̍ő�\����

// ���[�U�Ǘ��f�[�^�x�[�X�\����
typedef struct Database {
	HANDLE hMutex; // ���ݔr���p
	SOCKET listen_sock; 
	bool registered; // 1���ȏ�o�^�҂������true
	bool running; // ���s���ł��邩�ǂ����������t���O
	UserData entry[MAX_USERNUM]; // �i�[����郆�[�U�f�[�^�̔z��
} Database;

// ���[�U�f�[�^�x�[�X���̃G���g���󋵂��e�L�X�g�\��
int Database_print(Database* database);

// ���[�U�f�[�^�x�[�X���ŋ󂢂Ă���G���g���̃C���f�b�N�X������
// �󂫃G���g��������Ƃ��F�C���f�b�N�X(0 <-> (MAX_USER - 1))��Ԃ�
// �󂫃G���g�����Ȃ��Ƃ��F-1��Ԃ�
int Database_search_empty(Database* database);

// ���[�U�[�Ǘ��f�[�^�x�[�X���������i�S�ẴG���g����o�^�Ȃ��ɂ���j
void Database_init(Database* database);

// ���[�U�Ǘ��f�[�^�x�[�X�̃��\�[�X�̉��
void Database_release(Database* database);

// ���[�U�[�f�[�^�����[�U�Ǘ��f�[�^�x�[�X�ɓo�^
int Database_regist(Database* database, UserData* user_data);

// ���[�U�[�f�[�^�����[�U�Ǘ��f�[�^�x�[�X����폜
int Database_unregist(Database* database, UserData* user_data);

// ���[�U�Ǘ��f�[�^�x�[�X�ɂ��L�[���͑҂��󂯃X���b�h�p�֐�
unsigned __stdcall Database_keyinput_thread(void* p);

#endif /* DATABASE_H */