/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "MouseData.h"

void MouseData_make(MouseData* data, int x, int y, bool lbutton_down, bool dragged, int color_idx) {
	data->x = x;
	data->y = y;
	data->lbutton_down = lbutton_down;
	data->dragged = dragged;
	data->color_idx = color_idx;
}

void MouseData2PacketData(MouseData* src, PacketData* dst) {
	dst->cmd = DAT_MOUSEDATA;
	dst->size = sizeof(int) * 3 + sizeof(bool) * 2;
	MouseData_serialize(src, dst);

}

void PacketData2MouseData(PacketData* src, MouseData* dst) {
	MouseData_deserialize(src, dst);
}

void MouseData_clear(MouseData* mouse_data) {
	mouse_data->x = -1;
	mouse_data->y = -1;
	mouse_data->lbutton_down = false;
	mouse_data->dragged = false;
	mouse_data->color_idx = 12; // black
}

void MouseData_copy(MouseData* src, MouseData* dst) {
	dst->x = src->x;
	dst->y = src->y;
	dst->lbutton_down = src->lbutton_down;
	dst->dragged = src->dragged;
	dst->color_idx = src->color_idx;
}

void MouseData_print(MouseData* data) {
	printf("MouseData_print: (%d, %d), lbutton_down: %d, dragged: %d, color_idx = %d\n",
		data->x, data->y, data->lbutton_down, data->dragged, data->color_idx);
}

void MouseData_serialize(MouseData* mouse_data, PacketData* packet_data){

	// マウスポインタデータのシリアライズ
	memcpy(&packet_data->binary[0], &mouse_data->x, sizeof(int));
	memcpy(&packet_data->binary[sizeof(int)], &mouse_data->y, sizeof(int));
	memcpy(&packet_data->binary[sizeof(int) * 2], &mouse_data->lbutton_down, sizeof(bool));
	memcpy(&packet_data->binary[sizeof(int) * 2 + sizeof(bool) * 1], &mouse_data->dragged, sizeof(bool));
	memcpy(&packet_data->binary[sizeof(int) * 2 + sizeof(bool) * 2], &mouse_data->color_idx, sizeof(int));
}

void MouseData_deserialize(PacketData* packet_data, MouseData* mouse_data){

	// マウスポインタデータのデシリアライズ
	memcpy(&mouse_data->x, &packet_data->binary[0], sizeof(int));
	memcpy(&mouse_data->y, &packet_data->binary[sizeof(int)], sizeof(int));
	memcpy(&mouse_data->lbutton_down, &packet_data->binary[sizeof(int) * 2], sizeof(bool));
	memcpy(&mouse_data->dragged, &packet_data->binary[sizeof(int) * 2 + sizeof(bool) * 1], sizeof(bool));
	memcpy(&mouse_data->color_idx, &packet_data->binary[sizeof(int) * 2 + sizeof(bool) * 2], sizeof(int));

}

