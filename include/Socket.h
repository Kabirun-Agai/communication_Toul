/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef SOCKET_H
#define SOCKET_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

// �����F�\�P�b�g�ʐM�p�V�X�e��������������D�T�[�o�\�E�N���C�A���g���p�D
// �����F�Ȃ�
// �߂�l�F�G���[�̂Ƃ� -1�C�����̂Ƃ� 0
int Socket_init();

// �����F�\�P�b�g�ʐM�̗��p���I�����C�v���O�������I������ۂɌĂяo���D�T�[�o�[�E�N���C�A���g���p�D
// �����F�Ȃ�
// �߂�l�F�Ȃ�
void Socket_release();

// �����F�T�[�o�[�p�҂��󂯃\�P�b�g���쐬����D
// �����Fport �҂��󂯗p�|�[�g�ԍ��i�����j
// �߂�l�F�T�[�o�[���̑҂��󂯗p�\�P�b�g
SOCKET Socket_server(u_short port);

// �����F�T�[�o�[�p�҂��󂯃\�P�b�g�ɑ΂���ڑ��v����҂��C�u���b�N����D
// �����Flosten_sock �҂��󂯗p�\�P�b�g
// �߂�l�F�T�[�o�[���̒ʐM�p�\�P�b�g
SOCKET Socket_accept(SOCKET listen_sock);

// �����F�N���C�A���g�p�ʐM�\�P�b�g���쐬����D
// �����Fserver �z�X�g�����邢��IP�A�h���X�i������)
// �����Fport �T�[�o�[�̃|�[�g�ԍ�(����)
// �߂�l�F�N���C�A���g���̒ʐM�p�\�P�b�g
SOCKET Socket_client(char* server, u_short port);

// �����F�\�P�b�g�����i�T�[�o�[�E�N���C�A���g���p�C�҂��󂯁E�ʐM���p�j
// �����Fsock ����\�P�b�g
// �߂�l�F�Ȃ�
void Socket_close(SOCKET sock);

// �����F�\�P�b�g����w�肳�ꂽ�o�C�g���̃f�[�^�𐳊m�ɓǂݏo��
// �����Fsock �ʐM�p�\�P�b�g
// �����Fbp �ǂݏo�����char�^�z��ւ̃|�C���^
// �����Flen �ǂݏo���o�C�g��
// �߂�l�F����I�����@�ǂݏo�����o�C�g���i����len�Ɠ������j
// �߂�l�F�\�P�b�g������ꂽ�Ƃ��@0
// �߂�l�F�ǂݏo���Ȃ������Ƃ� -1
int Socket_recvn(SOCKET sock, unsigned char *bp, int len);

// �����F�\�P�b�g�Ɏw�肳�ꂽ�o�C�g���̃f�[�^�𑗐M����
// �����Fsock �ʐM�p�\�P�b�g
// �����Fbp ���M����f�[�^������char�^�z��ւ̃|�C���^
// �����Flen ���M����o�C�g��
// �߂�l�F����I�����@���M�����o�C�g��
// �߂�l�F���M�ł��Ȃ������Ƃ� -1
int Socket_send(SOCKET sock, unsigned char *bp, int len);

// �����F�\�P�b�g����int�^�f�[�^������M����
// �����Fsock �ʐM�p�\�P�b�g
// �����Fdata ���M����int�^�f�[�^
// �߂�l�F���M�����f�[�^�̃o�C�g�� sizeof(int)
int Socket_sendIntData(SOCKET sock, int data);

int Socket_recvIntData(SOCKET sock, int* data);

// �����F�\�P�b�g���u���b�L���O���[�h�ɂ���
// �����Fsock ��u���b�L���O���[�h�ɂ���\�P�b�g
// �߂�l�F�Ȃ�
void Socket_setNonBlockingMode(SOCKET sock);

// �����F�\�P�b�g���u���b�L���O���[�h�ɂ���
// �����Fsock �u���b�L���O���[�h�ɂ���\�P�b�g
// �߂�l�F�Ȃ�
void Socket_setBlockingMode(SOCKET sock);

#endif /* SOCEKT_H */