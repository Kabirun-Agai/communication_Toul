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

/* スタンプ関連情報を格納する構造体 */
typedef struct StampData {
	int x; // スタンプのx座標
	int y; // スタンプのy座標
	int stamp_id; // スタンプのID
} StampData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/

void StampData_make(StampData* data, int x, int y, int stamp_id);

void StampData2PacketData(StampData* src, PacketData* dst);
void PacketData2StampData(PacketData* src, StampData* dst);

void StampData_clear(StampData* stamp_data);
void StampData_copy(StampData* src, StampData* dst);

// スタンプデータのテキスト情報表示
void StampData_print(StampData* stamp_data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

// シリアライズ関数
void StampData_serialize(StampData* stamp_data, PacketData* packet_data);

// デシリアライズ関数
void StampData_deserialize(PacketData* packet_data, StampData* stamp_data);

#endif /* STAMPDATA_H */