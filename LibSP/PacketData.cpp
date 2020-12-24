/* ========================================================================= */
/* Copyright (C) 2020 Nishiguchi-lab. & Fukushima-lab. All Rights Reserved.  */
/* ========================================================================= */
#include "PacketData.h"

HANDLE sock_mutex = NULL;

void PacketData_init(PacketData* packet_data) {
	if (sock_mutex == NULL) {
		sock_mutex = CreateMutex(NULL, FALSE, NULL);
	}
	packet_data->binary = new unsigned char[DATA_W * DATA_H * DATA_C];
}

void PacketData_release(PacketData* packet_data) {
	if (sock_mutex != NULL) {
		CloseHandle(sock_mutex);
	}
	sock_mutex = NULL;
	delete[] packet_data->binary;
}

void PacketData_clear(PacketData* packet_data) {
	packet_data->cmd = CMD_ERROR;
	packet_data->size = 0;
	memset(packet_data->binary, '\0', DATA_W * DATA_H * DATA_C);
}

void PacketData_copy(PacketData* src, PacketData* dst) {
	dst->cmd = src->cmd;
	dst->size = src->size;
	memcpy(dst->binary, src->binary, dst->size);
}

int PacketData_recv(SOCKET sock, PacketData* packet_data) {
	int ret;

	const int header_length = sizeof(unsigned char) + sizeof(int);
	unsigned char header[header_length];

	PacketData_clear(packet_data);

	WaitForSingleObject(sock_mutex, INFINITE);
	// コマンド（１バイト）+ サイズ(int型)を受信
	ret = Socket_recvn(sock, &header[0], sizeof(unsigned char) + sizeof(int));
	memcpy(&packet_data->cmd, &header[0], sizeof(unsigned char));
	memcpy(&packet_data->size, &header[sizeof(unsigned char)], sizeof(int));
	//fprintf(stderr, "PacketData_recv: ret = %d\n", ret);
	if (ret > 0 && packet_data->size > 0) {
		memset(packet_data->binary, '\0', DATA_W * DATA_H * DATA_C);
		ret = Socket_recvn(sock, &packet_data->binary[0], packet_data->size);
	}
	ReleaseMutex(sock_mutex);

	Sleep(1);

#if 0
	if (ret > 0) {
		fprintf(stderr, "===== RECV: PacketData_recv: ret = %d ==\n", ret);
		// PacketData_print(packet_data);
		// PacketData_print_binary(packet_data);
		fprintf(stderr, "----------------------------------\n");
	}
#endif

	return ret;
}

int PacketData_send(SOCKET sock, PacketData* packet_data) {
	int ret;
	
	const int packet_size = sizeof(char) + sizeof(int) + DATA_W * DATA_H * DATA_C;
	unsigned char* serialized_data = new unsigned char [packet_size];

	PacketData_serialize(packet_data, serialized_data);

	WaitForSingleObject(sock_mutex, INFINITE);
	ret = Socket_send(sock, serialized_data, sizeof(char) + sizeof(int) + packet_data->size);
	ReleaseMutex(sock_mutex);

	delete[] serialized_data;

#if 0
	if (ret > 0) {
		fprintf(stderr, "===== SEND: PacketData_send: =====\n");
		PacketData_print(packet_data);
		// PacketData_print_binary(packet_data);
		fprintf(stderr, "----------------------------------\n");
	}
#endif

	return ret;
}

void PacketData_print(PacketData* packet_data) {
	fprintf(stderr, "PacketData_print: cmd = 0x%02x, size = %d\n", packet_data->cmd, packet_data->size);

}

void PacketData_print_binary(PacketData* packet_data) {
	fprintf(stderr, "PacketData_print_binary:\n");
	for (int i = 0; i < packet_data->size; i++) {
		fprintf(stderr, "%02x", (unsigned char)packet_data->binary[i]);
		if ((i + 1) % 16 == 0) {
			fprintf(stderr, "\n");
		}
		else {
			fprintf(stderr, " ");
		}
	}
	fprintf(stderr, "\n");
}

void PacketData_serialize(PacketData* packet_data, unsigned char* serialized_data) {

	memcpy(&serialized_data[0], &packet_data->cmd, sizeof(unsigned char));
	memcpy(&serialized_data[sizeof(unsigned char)], &packet_data->size, sizeof(int));
	memcpy(&serialized_data[sizeof(unsigned char) + sizeof(int)], &packet_data->binary[0], packet_data->size);

}

void PacketData_deserialize(unsigned char* serialized_data, PacketData* packet_data) {

	memcpy(&packet_data->cmd, &serialized_data[0], sizeof(unsigned char)); // 送信コマンド
	memcpy(&packet_data->size, &serialized_data[sizeof(unsigned char)], sizeof(int)); // 送信バイト数
	memcpy(&packet_data->binary[0], &serialized_data[sizeof(unsigned char) + sizeof(int)], packet_data->size); // 実際のデータ

}