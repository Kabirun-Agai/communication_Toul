#include "Command.h"
#include "UserData.h"
#include "ChatData.h"

#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Comdlg32.lib")

#if _DEBUG
#pragma comment(lib, "opencv_world430d.lib")
#pragma comment(lib, "../x64/Debug/LibSP.lib")
#else
#pragma comment(lib, "opencv_world430.lib")
#pragma comment(lib, "../x64/Release/LibSP.lib")
#endif

// �z���C�g�{�[�h�C�X�^���v�C�摜�`��p�̉摜�T�C�Y
#define DRAW_AREA_W 640
#define DRAW_AREA_H 480

// �`���b�g�\���p�̉摜�T�C�Y
#define CHAT_AREA_W 640
#define CHAT_AREA_H 240

// �S�̗p�̉摜�T�C�Y
#define CANVAS_W (DRAW_AREA_W)
#define CANVAS_H (DRAW_AREA_H + CHAT_AREA_H)

#define FONT_SIZE 16 // points

cv::Mat canvas;
cv::Mat draw_area;
cv::Mat chat_area;

// false�̂Ƃ��X���b�h���I��
bool connected = true;

// ���[�U�f�[�^(�o�^�C�폜�p)
UserData user_data;

void update_draw_area(cv::Mat& canvas, cv::Mat draw_area) {
	cv::Rect drawrect(0, 0, DRAW_AREA_W, DRAW_AREA_H);
	cv::Mat draw_area_on_canvas = canvas(drawrect);
	draw_area.copyTo(draw_area_on_canvas);
}

void update_chat_area(cv::Mat& canvas, cv::Mat chat_area) {
	cv::Rect chatrect(0, DRAW_AREA_H, CHAT_AREA_W, CHAT_AREA_H);
	cv::Mat chat_area_on_canvas = canvas(chatrect);
	chat_area.copyTo(chat_area_on_canvas);
}

// �N�����I�v�V�����̕\��
int usage(char* program_name) {
	printf("%s server_ip server_port user_name\n", program_name);
	return -1;
}
/****************************************************************/

char* winname = "Chat Client"; // �E�B���h�E��

// �N���C�A���g�X���b�h
unsigned __stdcall client_thread(void* p) {

	// �ʐM�\�P�b�g���󂯎��
	SOCKET sock = *(SOCKET*)p;

	// �\�P�b�g���u���b�L���O���[�h�ɂ���
	Socket_setNonBlockingMode(sock);

	PacketData packet_data;
	PacketData_init(&packet_data);

	ChatData chat_data;

	/***** �h���[�G���A�p�摜�̏��� *****/
	cv::Mat draw_area;
	// ���������蓖��
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// ���œh��Ԃ�
	draw_area = cv::Scalar(255, 255, 255);

	/***** �`���b�g�G���A�p�摜�̏��� *****/
	cv::Mat chat_area;
	// ���������蓖��
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// ���F�œh��Ԃ�
	chat_area = cv::Scalar(255, 127, 127);

	/***** �S�̕\���p�摜�i�L�����o�X�j�̏��� *****/
	cv::Mat canvas;
	// ���������蓖��
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// ���F�œh��Ԃ�
	canvas = cv::Scalar(255, 255, 255);

	// �E�B���h�E���J��
	cv::namedWindow(winname);
	// ���T�C�Y����
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);
	// �摜�̕\���\��
	cv::imshow(winname, canvas);
	// �摜�̕`��&�L�[���͑҂�(����0�̎��u���b�N�C1�ȏ�̂Ƃ��ҋ@����(ms))
	int key = cv::waitKey(1);

	cvPutTextJP pt("MS �S�V�b�N", FONT_SIZE);
	cv::Rect src_rect(0, 0, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);
	cv::Rect dst_rect(0, FONT_SIZE, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);

	// �_�u���o�b�t�@�����b�Z�[�W�\���p�摜�̈�̃N���[���Ƃ��āC�ʓr�m��
	// �_�u���i���ʁj�p�o�b�t�@
	cv::Mat doubleBuffer;
	doubleBuffer = chat_area.clone();
	doubleBuffer = cv::Scalar(255, 127, 127);

	while (connected) {

		int ret;

		// PacketData����M
		ret = PacketData_recv(sock, &packet_data);

		if (ret == 0) {
			// �\�P�b�g������ꂽ�Ƃ�
		}
		else if (ret < 0) {
			// �f�[�^����M�̂Ƃ�
		}
		else {
			cv::Mat src_roi, dst_roi;

			switch (packet_data.cmd) {
			case DAT_CHATDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_CHATDATA\n");

				PacketData2ChatData(&packet_data, &chat_data);
				ChatData_print(&chat_data);

				// �\�����b�Z�[�W�̍쐬
				char message_text[MAX_USERNAME_LEN + MAX_MESSAGE_LEN];
				sprintf_s(message_text, MAX_USERNAME_LEN + MAX_MESSAGE_LEN, "%s: %s", chat_data.user_name, chat_data.message);

				//ChatData_maketext(&chat_data, message_text);

				/* �摜�̈�ԏ�Ɉ�s�ǉ� */
				src_roi = chat_area(src_rect);
				dst_roi = doubleBuffer(dst_rect);
				src_roi.copyTo(dst_roi);

				doubleBuffer.copyTo(chat_area);

				pt.setLocate(cv::Point(0, 0));
				pt.putText(chat_area, (TCHAR*)message_text, cv::Scalar(0, 0, 0));

				// chat_area�̃A�b�v�f�[�g
				update_chat_area(canvas, chat_area);

				break;
			default:

				break;
			}

			// �摜�̕\���\��
			cv::imshow(winname, canvas);
			// �摜�̕\��&�L�[����
			key = cv::waitKey(1);

			if (key == 0x1b || key == 'q' || key == 'Q') {
				connected = false;
			}
		}
	}

	// �L�����o�X�p�����������
	canvas.release();
	draw_area.release();
	chat_area.release();

	// ��M�f�[�^����F�����擾
	doubleBuffer.release();

	_endthreadex(0);

	return 0;
}

// ���b�Z�[�W���͗p�X���b�h
unsigned __stdcall message_input_thread(void* p) {

	// �ʐM�\�P�b�g���󂯎��
	SOCKET sock = *((SOCKET*)p);

	char message[MAX_MESSAGE_LEN];

	ChatData chat_data;

	PacketData packet_data;
	PacketData_init(&packet_data);

	// �i�v���[�v
	for (;;) {
		int ret;

		// ���͗p�v�����v�g��\��
		fprintf(stdout, "Please Input: ");

		// �W�����͂���P�s����
		memset(message, '\0', MAX_MESSAGE_LEN);
		fgets(message, MAX_MESSAGE_LEN, stdin);
		// fgets�ł́C"������\n\0"�ƂȂ�̂ŁC�ȉ��̂悤�ɂ��ĉ��s�R�[�h('\n')��'\0'�ŏ㏑������
		for (unsigned int i = 0; i < strlen(message); i++)
		{
			if (message[i] == '\n')
			{
				message[i] = '\0';
				break;
			}
		}

		// ���b�Z�[�W��'quit'�̂Ƃ��Cconnected�ϐ���false�ɂ�����ŁCbreak
		if (strcmp(message, "quit") == 0) {

			/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o����폜 ***/
			// UserData�𐶐�

			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);

			ret = PacketData_send(sock, &packet_data);

			connected = false;

			break;
		}

		// ���b�Z�[�W���M
		// ���͂��烁�b�Z�[�W���쐬
		ChatData_make(&chat_data, 12, user_data.user_name, message);

		ChatData2PacketData(&chat_data, &packet_data);

		ret = PacketData_send(sock, &packet_data);

		ChatData_print(&chat_data);
	}

	PacketData_release(&packet_data);

	_endthreadex(0);

	return 0;
}

int main(int argc, char* argv[]) {
	int ret;

	char* server;
	int port;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	PacketData packet_data;
	PacketData_init(&packet_data);

	char user_name[MAX_USERNAME_LEN];

	/* �������� */
	// IP�A�h���X(�K�{)�C�|�[�g�ԍ�(�K�{)�C���[�U��(�K�{), �����F�i�f�t�H���g12�j
	// �������v���O�����{�̂��܂߂�5�i�����݂̂�4�j���ǂ����`�F�b�N
	if (argc == 4) {
		// ���������T�[�o�����邢��IP�A�h���X�Ƃ��Ď擾
		server = argv[1];
		// ���������|�[�g�ԍ��Ƃ��Ď擾
		port = atoi(argv[2]);

		sprintf_s(user_name, MAX_USERNAME_LEN, "%s", argv[3]);
		fprintf(stderr, "Username: %s, size: %zd\n", user_name, strlen(user_name));
	}
	else {
		usage(argv[0]);
		return -1;
	}

	// ���[�U�f�[�^(�o�^�p)���쐬
	UserData_make(user_name, &user_data);
	// ���[�U�f�[�^����\��
	UserData_print(&user_data);
	// �p�P�b�g�f�[�^�ɕϊ�
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// �p�P�b�g�f�[�^����\��
	PacketData_print(&packet_data);
	// �p�P�b�g�f�[�^���(�o�C�i�����)��\��
	//PacketData_print_hex(&packet_data);

	/***** �N���C�A���g�p�����ݒ� *****/
	// �\�P�b�g�p���\�[�X�̏������i�T�[�o�C�N���C�A���g���p�j
	Socket_init();
	// �N���C�A���g�\�P�b�g�i�ڑ��p�j�̍쐬
	SOCKET sock = Socket_client(server, port);

	/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o�ɓo�^ ***/
	if (sock >= 0) {
		// �p�P�b�g�f�[�^�𑗐M
		ret = PacketData_send(sock, &packet_data);
	}
	else {
		fprintf(stderr, "Chat Client: Cannot connect to server\n");
		return -1;
	}

	// �e�X���b�h���J�n
	HANDLE handle[2];
	handle[0] = (HANDLE)_beginthreadex(NULL, 0, message_input_thread, &sock, 0, NULL);
	handle[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// ���ׂẴX���b�h���I������܂őҋ@
	WaitForMultipleObjects(2, handle, TRUE, INFINITE);

	for (int i = 0; i < 2; i++) {
		CloseHandle(handle[i]);
	}

	// �ʐM�p�\�P�b�g�����
	Socket_close(sock);

	// �\�P�b�g�p���\�[�X������i�T�[�o�C�N���C�A���g���p�j
	Socket_release();

	return 0;
}