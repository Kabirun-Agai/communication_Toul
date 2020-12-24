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

// クライアントからサーバに送信するコマンド（１バイト）
typedef enum {
	CMD_USERDATA_REGIST = 0x00,   // ユーザ登録要求コマンド
	CMD_USERDATA_UNREGIST = 0x01, // ユーザ登録解除要求コマンド
	DAT_CHATDATA = 0x02, // メッセージデータ
	DAT_MOUSEDATA = 0x03,   // マウスポインタデータ
	DAT_STAMPDATA = 0x04,   // スタンプデータ
	DAT_IMAGEDATA = 0x05,   // jpeg画像データ
	CMD_ERROR = 0xff,
	// 送受信するデータを追加するには，ここに新しいコマンドを追加する．
} COMMAND;

#endif /* COMMAND_H */