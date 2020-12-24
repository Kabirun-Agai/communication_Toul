#include "Command.h"
#include "UserData.h"
#include "ChatData.h"
#include "MouseData.h"
#include "StampData.h"
#include "ImageData.h"

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

int stamp_id = 0;

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

char winname[MAX_WINNAME_LEN]; // = "IntegratedClient"; // �E�B���h�E��

/***** ���M�p�}�E�X�N���C�A���g�f�[�^ ************/
MouseData send_mouse_data;

/***** �X�^���v�@�\�ƃz���C�g�{�[�h�@�\�̋�ʂ̂��߂Ɏg�p *****/
typedef enum {
	MODE_STAMP = 0x00,
	MODE_MOUSE = 0x01,
} Mode;

// �f�t�H���g�̓X�^���v���[�h
Mode mode = MODE_STAMP;

/***** �X�^���v�f�[�^ for ���M�p */
StampData send_stamp_data;
// �X�^���v�摜�i�[�p
cv::Mat stamp[NUM_OF_STAMP];
/*********************************/

/***** �摜���L�N���C�A���g�p *****/
bool update_image_flag = false;

unsigned __stdcall open_file_dialog(void* p) {
	char* return_path = (char*)p;
	char path[MAX_FILE_PATH], name[MAX_FILE_PATH];

	// �t�@�C���_�C�A���O�\���p�̃E�B���h�E�̃n���h�����擾
	HWND hwnd = (HWND)cvGetWindowHandle(winname);

	//������Ƀk�������������Ă���
	memset(path, '\0', sizeof(path));
	memset(name, '\0', sizeof(name));

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;     //�I�����ꂽ�t�@�C�������󂯎��(�t���p�X)
	ofn.nMaxFile = MAX_FILE_PATH;
	ofn.lpstrFileTitle = name; //�I�����ꂽ�t�@�C����
	ofn.nMaxFileTitle = MAX_FILE_PATH;

	//fprintf(stderr, "Pass 01\n");

	ofn.lpstrFilter = "jpg(*.jpg)\0*.jpg\0all(*.*)\0*.*\0\0"; //�t�B���^
	ofn.lpstrTitle = "�摜���J��"; //�_�C�A���O�{�b�N�X�̃^�C�g��

	ofn.Flags = OFN_FILEMUSTEXIST;
	
	if (GetOpenFileName(&ofn) == FALSE) {
		return FALSE;
	}

	fprintf(stderr, "�I�����ꂽ�t�@�C���̃t���p�X:%s\n", path);
	fprintf(stderr, "�I�����ꂽ�t�@�C����: %s\n", name);

	memcpy(return_path, path, MAX_FILE_PATH);
	update_image_flag = true;

	_endthreadex(0);

	return 0;
}

void on_mouse(int event, int x, int y, int flag, void* param) {
	std::string event_description;
	int ret;

	SOCKET sock = *(SOCKET*)param;

	PacketData packet_data;
	PacketData_init(&packet_data);

	if (0 <= x && x < DRAW_AREA_W && 0 <= y && y < DRAW_AREA_H) {
		/*** �������� ***/
		/*** �X�^���v�̕��C�����C���S�ʒu��x���W, ���S�ʒu��y���W���v�Z ***/
		int sw = stamp[send_stamp_data.stamp_id].cols;
		int sh = stamp[send_stamp_data.stamp_id].rows;
		int sx = x - sw / 2;
		int sy = y - sh / 2;

		// �X�^���v�̋�`�̈悪�摜�̍��W�͈͂ɓ����Ă����
		if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

			// �}�E�X�C�x���g�̎�ނɂ�鏈���̕���
			switch (event) {
				// �}�E�X�|�C���^���������Ƃ�
			case cv::EVENT_MOUSEMOVE:

				switch (mode)
				{
				case MODE_STAMP:
						event_description += "MOUSE_MOVE";
						break;

				case MODE_MOUSE:
					if (lbutton_down) {

						/*** �������� ***/
						MouseData_make(&send_mouse_data, x, y, true, true, color_idx);

						MouseData2PacketData(&send_mouse_data, &packet_data);
						ret = PacketData_send(sock, &packet_data);

						MouseData_print(&send_mouse_data);
						PacketData_print(&packet_data);
						break;

						/*** �����܂� ***/
					}
				default:
					break;
				}
				break;

				// ���{�^�����N���b�N���ꂽ�Ƃ�
			case cv::EVENT_LBUTTONDOWN:
				event_description += "LBUTTON_DOWN";
				switch (mode){

				case MODE_STAMP:
					std::cerr << sx << ", " << sy << ", " << sw << ", " << sh << std::endl;

					StampData_make(&send_stamp_data, x, y, send_stamp_data.stamp_id);
					StampData2PacketData(&send_stamp_data, &packet_data);

					ret = PacketData_send(sock, &packet_data);


					// ���M�����f�[�^���e�L�X�g�\��
					StampData_print(&send_stamp_data);
					break;

				case MODE_MOUSE:
					lbutton_down = true;

					MouseData_make(&send_mouse_data, x, y, true, false, color_idx);

					MouseData2PacketData(&send_mouse_data, &packet_data);
					ret = PacketData_send(sock, &packet_data);

					MouseData_print(&send_mouse_data);
					PacketData_print(&packet_data);

					break;

				default:
					break;
				}

				break;
				// ���{�^���������ꂽ�Ƃ�
			case cv::EVENT_LBUTTONUP:
				event_description += "LBUTTON_UP";
				lbutton_down = false;
				switch (mode) {

				case MODE_STAMP:
					break;

				case MODE_MOUSE:
					MouseData_make(&send_mouse_data, x, y, false, false, color_idx);

					MouseData2PacketData(&send_mouse_data, &packet_data);
					ret = PacketData_send(sock, &packet_data);

					MouseData_print(&send_mouse_data);
					PacketData_print(&packet_data);

					break;

				default:
					break;
				}
				break;
			}
		}

		/*** �����܂� ***/
	}
	//std::cerr << "mouse event: " << event_description << std::endl;

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

	char path[MAX_FILE_PATH];

	PacketData recv_packet_data;
	PacketData_init(&recv_packet_data);

	ImageData image_data;
	ImageData_init(&image_data);

	ImageData upload_image;
	ImageData_init(&upload_image);

	MouseData recv_mouse_data;
	MouseData_make(&recv_mouse_data, 0, 0, false, false, 0);

	/***** �X�^���v�f�[�^ for ��M�p */
	StampData recv_stamp_data;

	MouseData pre_mouse_data;
	MouseData_make(&pre_mouse_data, 0, 0, false, false, 0);

	ChatData chat_data;

	fprintf(stderr, "IntegratedClient: client_thread: pass 00\n");

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

	fprintf(stderr, "IntegratedClient: client_thread: pass 01\n");

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

	cvPutTextJP pt("MS �S�V�b�N", FONT_SIZE);
	cv::Rect src_rect(0, 0, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);
	cv::Rect dst_rect(0, FONT_SIZE, CHAT_AREA_W, CHAT_AREA_H - FONT_SIZE);

	// �_�u���o�b�t�@�����b�Z�[�W�\���p�摜�̈�̃N���[���Ƃ��āC�ʓr�m��
	// �_�u���i���ʁj�p�o�b�t�@
	cv::Mat doubleBuffer;
	doubleBuffer = chat_area.clone();
	doubleBuffer = cv::Scalar(255, 127, 127);

	// �}�E�X�C�x���g�����������Ƃ��ɌĂяo���֐��i�R�[���o�b�N�֐��j��o�^
	cv::setMouseCallback(winname, on_mouse, &sock);

	while (connected) {

		int ret;

		// PacketData����M
		PacketData_clear(&recv_packet_data);
		ret = PacketData_recv(sock, &recv_packet_data);
		
		if (ret == 0) {
			// �\�P�b�g������ꂽ�Ƃ�
		}
		else if (ret < 0) {
			// �f�[�^����M�̂Ƃ�
		}
		else {
			cv::Mat src_roi, dst_roi;
			int sw, sh, sx, sy;
			unsigned char cmd = recv_packet_data.cmd;

			/*** �������� ***/
			switch (cmd) {
			case DAT_STAMPDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_STAMPDATA\n");

				// ��M����packet_data����StampData�𐶐�
				PacketData2StampData(&recv_packet_data, &recv_stamp_data);

				StampData_print(&recv_stamp_data);

				// ��M�f�[�^����X�^���vID���擾
				/* �X�^���v�̕�sw�C����sh�C���S�ʒu��sx���W, ���S�ʒu��sy���W���v�Z */
				sw = stamp[recv_stamp_data.stamp_id].cols;
				sh = stamp[recv_stamp_data.stamp_id].rows;
				sx = recv_stamp_data.x - sw / 2;
				sy = recv_stamp_data.y - sh / 2;

				// �X�^���v�̋�`�̈悪�摜�̍��W�͈͂ɓ����Ă����
				if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

					// ROI(x, y, w, h)�����
					cv::Rect roi_rect(sx, sy, sw, sh);
					// draw_area���ROI��ݒ�
					cv::Mat src_roi = draw_area(roi_rect);
					// �X�^���v���L�����o�X�ɃR�s�[
					stamp[recv_stamp_data.stamp_id].copyTo(src_roi);

				}
				// draw_area�̃A�b�v�f�[�g
				update_draw_area(canvas, draw_area);
				break;

			case DAT_CHATDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_CHATDATA\n");

				PacketData2ChatData(&recv_packet_data, &chat_data);
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

			case DAT_IMAGEDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_IMAGEDATA\n");
				// PacketData����摜�f�[�^(ImageData)�ɕϊ�
			
				PacketData2ImageData(&recv_packet_data, &image_data);

				fprintf(stderr, "Integrated Client: ImageData_recv: size = %d\n", ret);
				image_data.image.copyTo(draw_area);

				// draw_area�̃A�b�v�f�[�g
				update_draw_area(canvas, draw_area);
				break;

			case DAT_MOUSEDATA:
				fprintf(stderr, "Whiteboard Client: Recv: DAT_MOUSEDATA: ");
				// ��M�f�[�^�Ɋ�Â���draw_area�ɕ`��
				/*** �������� ***/
				PacketData2MouseData(&recv_packet_data, &recv_mouse_data);

				if (recv_mouse_data.lbutton_down && !recv_mouse_data.dragged) {
					cv::circle(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), 10, color[recv_mouse_data.color_idx],-1);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);
				}
				else if (recv_mouse_data.lbutton_down && recv_mouse_data.dragged) {
					cv::line(draw_area, cv::Point(recv_mouse_data.x, recv_mouse_data.y), cv::Point(pre_mouse_data.x, pre_mouse_data.y), color[recv_mouse_data.color_idx],20);
					fprintf(stderr, "(%d. %d) - (%d. %d)\n", recv_mouse_data.x, recv_mouse_data.y, pre_mouse_data.x, pre_mouse_data.y);
					MouseData_copy(&recv_mouse_data, &pre_mouse_data);

				}

				/*** �����܂� ***/

				// draw_area�̃A�b�v�f�[�g
				update_draw_area(canvas, draw_area);
				break;

			default:
				fprintf(stderr, "Integrated Client: client_thread: Received Command: 0x%02x\n", recv_packet_data.cmd);
				break;
			}

			/*** �����܂� ***/
		}
		
		if (update_image_flag == true) {
			// �_�C�A���O�Ŏw�肵���摜�t�@�C����ǂݏo����image�ɃZ�b�g����
			
			cv::Mat image = cv::imread(path);
			if (!image.empty()) {
				PacketData image_packet_data;
				PacketData_init(&image_packet_data);
				// �ǂݏo�����摜��(IMAGE_W, IMAGE_H)�Ƀ��T�C�Y����
				cv::resize(image, upload_image.image, upload_image.image.size(), cv::INTER_CUBIC);
				
				ImageData_make(&image_data, upload_image.image);
				ImageData2PacketData(&image_data, &image_packet_data);
				PacketData_print(&image_packet_data);
				
				ret = PacketData_send(sock, &image_packet_data);
				
				PacketData_release(&image_packet_data);
			}
			update_image_flag = false;
		}

		// �摜�̕\���\��
		cv::imshow(winname, canvas);
		// �摜�̕\��&�L�[����
		key = cv::waitKey(1);

		if (key == 0x1b || key == 'q' || key == 'Q') {
			/*** ���[�U�f�[�^���f�[�^�x�[�X�T�[�o����폜 ***/
			// UserData�𐶐�
			UserData_make(NULL, &user_data);
			PacketData user_packet_data;
			PacketData_init(&user_packet_data);
			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &user_packet_data);
			
			ret = PacketData_send(sock, &user_packet_data);
			
			PacketData_release(&user_packet_data);
			connected = false;
		}
		else if (key == 'o') {

			// �t�@�C���_�C�A���O���J���C���M�������摜��I������
			HANDLE h_open = (HANDLE)_beginthreadex(NULL, 0, open_file_dialog, path, 0, NULL);
			CloseHandle(h_open);
		}
		else if (key == 'm') {
			/*** ��������(StampClient�@�\��WhiteboardClient�@�\�𓯎��Ɏ�������ꍇ�ɕK�v) ***/
			if (mode == MODE_MOUSE) {
				mode = MODE_STAMP;
			}else if(mode == MODE_STAMP) {
				mode = MODE_MOUSE;
			}

			/*** �����܂�(StampClient�@�\��WhiteboardClient�@�\�𓯎��Ɏ�������ꍇ�ɕK�v) ***/
		}
		else {
			/*** '0' ~ 'f'�܂ł̃L�[�������ꂽ�Ƃ��Ckeynum��0 ~ 15(�\�i��)��������D����ȊO�̃L�[���͂̏ꍇ�Ckeynum��-1�������� ***/
			int keynum;
			if (0x30 <= key && key <= (0x30 + 9)) { // 0 - 9
				keynum = key - 0x30;
			}
			else if (0x61 <= key && key <= (0x61 + 5)) { // 'a'(10) - 'f'(15)
				keynum = key - 0x61 + 10;
			}
			else {
				keynum = -1;
			}
			
			

			/*** ��������(StampClient�@�\��WhiteboardClient�@�\�𓯎��Ɏ�������ꍇ�ɕK�v) ***/
			/*** �X�^���v���[�h�̂Ƃ��C'0' ~ NUM_OF_STAMP�̃L�[���͂�stamp_id��ݒ� ***/
			/*** �������� ***/
			if (mode == MODE_STAMP) {
				if (0 <= keynum && keynum < NUM_OF_STAMP) {
					send_stamp_data.stamp_id = keynum;
					fprintf(stderr, "Integrated Client: stamp_data.stamp_id = %d\n", send_stamp_data.stamp_id);
				}

			}	
			/*** �����܂�***/

			/*** �z���C�g�{�[�h���[�h�̂Ƃ��C'0' ~ 'f'�̃L�[���͂�color_idx�ɐF�̃C���f�b�N�X(Command.h�Œ�`)��ݒ� ***/
			/*** �������� ***/
			else if (mode == MODE_MOUSE) {
				if (0 <= keynum && keynum < NUM_OF_COLOR) {
					send_mouse_data.color_idx = keynum;
					fprintf(stderr, "Whiteboard Client: send_mouse_data.color_idx = %d\n", send_mouse_data.color_idx);
				}
			}


			/*** �����܂� ***/
			/*** �����܂�(StampClient�@�\��WhiteboardClient�@�\�𓯎��Ɏ�������ꍇ�ɕK�v) ***/
		}
	}

	doubleBuffer.release();
	ImageData_release(&image_data);
	ImageData_release(&upload_image);
	
	PacketData_release(&recv_packet_data);

	// �L�����o�X�p�����������
	canvas.release();
	draw_area.release();
	chat_area.release();

	_endthreadex(0);

	return 0;
}

// ���b�Z�[�W���͗p�X���b�h
unsigned __stdcall keyboard_input_thread(void* p) {

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

	sprintf_s(winname, MAX_WINNAME_LEN, "IntegratedClient - %s", user_name);

	// ���[�U�f�[�^(�o�^�p)���쐬
	UserData_make(user_name, &user_data);
	// ���[�U�f�[�^����\��
	UserData_print(&user_data);
	// �p�P�b�g�f�[�^�ɕϊ�
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// �p�P�b�g�f�[�^����\��
	PacketData_print(&packet_data);
	// �p�P�b�g�f�[�^���(�o�C�i�����)��\��
	PacketData_print_binary(&packet_data);

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
		fprintf(stderr, "IntegratedClient: Cannot connect to server\n");
		return -1;
	}

	/* �X�^���v�摜�̓ǂݍ��� */
	char filename[80];
	for (int i = 0; i < NUM_OF_STAMP; i++) {
		sprintf_s(filename, 80, "..\\..\\images\\stamp%02d.png", i);
		stamp[i] = cv::imread(filename);
		if (stamp[i].empty()) {
			fprintf(stderr, "Cannot open %s\n", filename);
			system("PAUSE");
			return -1;
		}
	}

	MouseData_make(&send_mouse_data, 0, 0, false, false, 12);

	// �e�X���b�h���J�n
	HANDLE hThread[2];
	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, keyboard_input_thread, &sock, 0, NULL);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// ���ׂẴX���b�h���I������܂őҋ@
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	for (int i = 0; i < 2; i++) {
		CloseHandle(hThread[i]);
	}

	PacketData_release(&packet_data);

	// �ʐM�p�\�P�b�g�����
	Socket_close(sock);

	// �\�P�b�g�p���\�[�X������i�T�[�o�C�N���C�A���g���p�j
	Socket_release();

	return 0;
}