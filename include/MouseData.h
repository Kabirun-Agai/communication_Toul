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

/* マウス関連情報を格納する構造体 */
typedef struct MouseData {
	int x; // 現在のマウスポインタ位置のx座標
	int y; // 現在のマウスポインタ位置のy座標
	bool lbutton_down; // 左ボタンが押されていればtrue，そうでなければfalse
	bool dragged; // ドラッグ中であればtrue, そうでなければfalse
	int color_idx; // Command.h参照
} MouseData;

/***** パブリック(プログラムから呼び出し可能)関数********************************************/
void MouseData_make(MouseData* mouse_data, int x, int y, bool lbutton_down, bool dragged, int color_id);

void MouseData2PacketData(MouseData* src, PacketData* dst);
void PacketData2MouseData(PacketData* src, MouseData* dst);

void MouseData_clear(MouseData* mouse_data);
/* マウスデータのコピー */
/* 第1引数(src)を第2引数(dst)にコピー */
void MouseData_copy(MouseData* src, MouseData* dst);

/* マウスデータのテキスト情報表示 */
void MouseData_print(MouseData* mouse_data);

/***** プライベート(外部からの呼び出しを想定しない)*******************************************/

// シリアライズ関数
void MouseData_serialize(MouseData* mouse_data, PacketData* base_data);
// デシリアライズ関数
void MouseData_deserialize(PacketData* base_data, MouseData* mouse_data);

#endif /* MOUSEDATA_H */