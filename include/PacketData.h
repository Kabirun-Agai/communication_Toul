/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#ifndef PACKETDATA_H
#define PACKETDATA_H

#include <Winsock2.h>
#include <Windows.h>
#include "Command.h"

#define DATA_W 640
#define DATA_H 480
#define DATA_C 3

typedef struct PacketData {
	unsigned char cmd;
	int size; // データサイズ（バイト）
	unsigned char* binary; // [IMAGE_W * IMAGE_H * IMAGE_C]; // jpeg圧縮画像のバイナリデータを格納．送受信に利用．
} PacketData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/

void PacketData_init(PacketData* packet_data);
void PacketData_release(PacketData* packet_data);

void PacketData_clear(PacketData* packet_data);
void PacketData_copy(PacketData* src, PacketData* dst);

int PacketData_recv(SOCKET sock, PacketData* packet_data);
int PacketData_send(SOCKET sock, PacketData* packet_data);

void PacketData_print(PacketData* packet_data);
void PacketData_print_binary(PacketData* packet_data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

void PacketData_serialize(PacketData* packet_data, unsigned char* serialized_data);
void PacketData_deserialize(unsigned char* serialized_data, PacketData* packet_data);

#endif /* PACKETDATA_H */