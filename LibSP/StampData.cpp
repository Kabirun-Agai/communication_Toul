/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "StampData.h"

void StampData_make(StampData* data, int x, int y, int stamp_id) {
	data->x = x;
	data->y = y;
	data->stamp_id = stamp_id;
}

void StampData2PacketData(StampData* src, PacketData* dst) {
	dst->cmd = DAT_STAMPDATA;
	dst->size = sizeof(int) * 3;
	StampData_serialize(src, dst);

}

void PacketData2StampData(PacketData* src, StampData* dst) {
	StampData_deserialize(src, dst);
}

void StampData_clear(StampData* stamp_data) {
	stamp_data->x = -1;
	stamp_data->y = -1;
	stamp_data->stamp_id = 0;
}

void StampData_copy(StampData* src, StampData* dst) {
	dst->x = src->x;
	dst->y = src->y;
	dst->stamp_id = src->stamp_id;
}

void StampData_print(StampData* stamp_data) {
	printf("StampData_print: (%d, %d), stamp_id = %d\n", stamp_data->x, stamp_data->y, stamp_data->stamp_id);
}

void StampData_serialize(StampData* stamp_data, PacketData* packet_data){
	// スタンプデータのデータ部のシリアライズ

	memcpy(&packet_data->binary[0], &stamp_data->x, sizeof(int));
	memcpy(&packet_data->binary[sizeof(int)], &stamp_data->y, sizeof(int));
	memcpy(&packet_data->binary[sizeof(int) * 2], &stamp_data->stamp_id, sizeof(int));

}

void StampData_deserialize(PacketData* packet_data, StampData* stamp_data){
	// スタンプデータのデシリアライズ

	memcpy(&stamp_data->x, &packet_data->binary[0], sizeof(int));
	memcpy(&stamp_data->y, &packet_data->binary[sizeof(int)], sizeof(int));
	memcpy(&stamp_data->stamp_id, &packet_data->binary[sizeof(int) * 2], sizeof(int));

}
