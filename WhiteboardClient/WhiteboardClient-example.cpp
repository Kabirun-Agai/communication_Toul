#include "Command.h"
#include "UserData.h"
#include "MouseData.h"

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

#define MAX_WINNAME_LEN 80

cv::Mat canvas;
cv::Mat draw_area;
cv::Mat chat_area;

// false�̂Ƃ��X���b�h���I��
bool connected = true;

bool lbutton_down = false;
int color_idx = 12; // black in Command.h

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

char winname[MAX_WINNAME_LEN]; // = "Whiteboard Client"; // �E�B���h�E��

/***** ���M�p�}�E�X�N���C�A���g�f�[�^ ************/
MouseData send_mouse_data;

void on_mouse(int event, int x, int y, int flag, void* param) {
	std::string event_description;
	int ret;

	SOCKET sock = *(SOCKET*)param;

	PacketData packet_data;
	PacketData_init(&packet_data);

	if (0 <= x && x < DRAW_AREA_W && 0 <= y && y < DRAW_AREA_H) {

		// �}�E�X�C�x���g�̎�ނɂ�鏈���̕���
		switch (event) {
			// �}�E�X�|�C���^���ړ������Ƃ�
		case cv::EVENT_MOUSEMOVE:
			event_description += "MOUSE_MOVE";

			// ���{�^�����������ꂽ��Ԃ̂Ƃ�
			if (lbutton_down) {

				// �}�E�X�f�[�^���\�z����
				MouseData_make(&send_mouse_data, x, y, true, true, color_idx);

				MouseData2PacketData(&send_mouse_data, &packet_data);
				ret = PacketData_send(sock, &packet_data);

				// ���M�����f�[�^���e�L�X�g�\��
				MouseData_print(&send_mouse_data);
				PacketData_print(&packet_data);
			}
			break;
			// ���{�^�����������ꂽ�Ƃ�
		case cv::EVENT_LBUTTONDOWN:
			event_description += "LBUTTON_DOWN";
			lbutton_down = true;
			// �}�E�X�f�[�^���\�z����
			MouseData_make(&send_mouse_data, x, y, true, false, color_idx);

			MouseData2PacketData(&send_mouse_data, &packet_data);
			ret = PacketData_send(sock, &packet_data);

			// ���M�����f�[�^���e�L�X�g�\��
			MouseData_print(&send_mouse_data);
			PacketData_print(&packet_data);
			break;
			// ���{�^���������ꂽ�Ƃ�
		case cv::EVENT_LBUTTONUP:
			event_description += "LBUTTON_UP";
			lbutton_down = false;
			// �}�E�X�f�[�^���\�z����
			MouseData_make(&send_mouse_data, x, y, false, false, color_idx);

			MouseData2PacketData(&send_mouse_data, &packet_data);
			ret = PacketData_send(sock, &packet_data);

			// ���M�����f�[�^���e�L�X�g�\��
			MouseData_print(&send_mouse_data);
			PacketData_print(&packet_data);
			break;
		default:
			break;
		}
	}

	// std::cerr << "mouse event: " << event_description << std::endl;

	PacketData_release(&packet_data);

	return;
}

// �N���C�A���g�X���b�h
unsigned __stdcall client_thread(void* p) {
	int key;

	// �ʐM�\�P�b�g���󂯎��
	SOCKET sock = *(SOCKET*)p;

	// �\�P�b�g���u���b�L���O���[�h�ɂ���
	Socket_setNonBlockingMode(sock);

	PacketData packet_data;
	PacketData_init(&packet_data);

	MouseData recv_mouse_data;
	MouseData_make(&recv_mouse_data, 0, 0, false, false, 0);

	MouseData pre_mouse_data;
	MouseData_make(&pre_mouse_data, 0, 0, false, false, 0);

	/***** �h���[�G���A�p�摜�̏��� *****/
	// ���������蓖��
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// ���œh��Ԃ�
	draw_area = cv::Scalar(255, 255, 255);

	/***** �`���b�g�G���A�p�摜�̏��� *****/

	// ���������蓖��
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// ���F�œh��Ԃ�
	chat_area = cv::Scalar(255, 127, 127);

	fprintf(stderr, "Whiteboard Client: client_thread: pass 01\n");

	/***** �S�̕\���p�摜�i�L�����o�X�j�̏��� *****/
	// ���������蓖��
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// ���F�œh��Ԃ�
	canvas = cv::Scalar(255, 255, 255);

	// �E�B���h�E���J��
	cv::namedWindow(winname);
	// ���T�C�Y����
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);

	update_draw_area(canvas, draw_area);
	update_chat_area(canvas, chat_area);

	// �摜�̕\���\��
	cv::imshow(winname, canvas);
	// �摜�̕`��&�L�[���͑҂�(����0�̎��u���b�N�C1�ȏ�̂Ƃ��ҋ@����(ms))
	key = cv::waitKey(1);

	// �}�E�X�C�x���g�����������Ƃ��ɌĂяo���֐��i�R�[���o�b�N�֐��j��o�^
	cv::setMouseCallback(winname, on_mouse, &sock);

	while (connected) {

		int ret;

		// PacketData����M
		ret = PacketData_recv(sock, &packet_data);

		if (ret == 0) {
			// �\�P�b�g������ꂽ�Ƃ�
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK) {
				connected = false;
			}
		}
		else if (ret < 0) {
			// �f�[�^����M�̂Ƃ�
		}
		else {
			switch (packet_data.cmd) {
			case DAT_MOUSEDATA:
				fprintf(stderr, "Whiteboard Client: Recv: DAT_MOUSEDATA: ");

				PacketData2MouseData(&packet_data, &recv_mouse_data);
				MouseData_print(&recv_mouse_data);

				if (recv_mouse_data.lbutton_down && !recv_mouse_data.dragged) {
					// ���݂̈ʒu�ɓ_��ł�
					cv::circle(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), 10, color[recv_mouse_data.color_idx], -1);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);
				}
				else if (recv_mouse_data.lbutton_down && recv_mouse_data.dragged) {
					// ��O�̈ʒu�ƌ��݂̈ʒu�̊Ԃɐ�������
					cv::line(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), cv::Point(pre_mouse_data.x, pre_mouse_data.y), color[recv_mouse_data.color_idx], 20);
					fprintf(stderr, "(%d, %d) - (%d, %d)\n", recv_mouse_data.x, recv_mouse_data.y, pre_mouse_data.x, pre_mouse_data.y);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);
				}

				// draw_area�̃A�b�v�f�[�g
				update_draw_area(canvas, draw_area);
				break;

			default:
				fprintf(stderr, "Whiteboard Client: client_thread: Received Command: 0x%02x\n", packet_data.cmd);
				break;
			}
		}

		// �摜�̕\���\��
		cv::imshow(winname, canvas);
		// �摜�̕\��&�L�[����
		key = cv::waitKey(1);

		if (key == 0x1b || key == 'q' || key == 'Q') {
			/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o����폜 ***/
			// UserData�𐶐�
			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);

			ret = PacketData_send(sock, &packet_data);

			connected = false;
		}
		else {

			int keynum;
			if (0x30 <= key && key <= (0x30 + 9)) { // '0' - '9'
				keynum = key - 0x30;
			}
			else if (0x61 <= key && key <= (0x61 + 5)) { // 'a'(10) - 'f'(15)
				keynum = key - 0x61 + 10;
			}
			else {
				keynum = -1;
			}

			if (0 <= keynum && keynum < NUM_OF_COLOR) {
				color_idx = keynum;
				fprintf(stderr, "Whiteboard Client: color_idx = %d\n", color_idx);
			}
		}
	}

	PacketData_release(&packet_data);

	// �L�����o�X�p�����������
	canvas.release();
	draw_area.release();
	chat_area.release();

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

	sprintf_s(winname, MAX_WINNAME_LEN, "WhiteboardClient - %s", user_name);

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
		fprintf(stderr, "Whiteboard Client: UserData_send: ");
		PacketData_print(&packet_data);
	}
	else {
		fprintf(stderr, "Whiteboard Client: Cannot connect to server\n");
		return -1;
	}

	// �X���b�h�̋N���i�o�^�j
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, client_thread, (void*)&sock, 0, NULL);

	// �X���b�h���I������܂őҋ@
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	// �ʐM�p�\�P�b�g�����
	Socket_close(sock);
	// �\�P�b�g�p���\�[�X������i�T�[�o�C�N���C�A���g���p�j
	Socket_release();

	return 0;
}