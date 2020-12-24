#include "Command.h"
#include "UserData.h"
#include "StampData.h"

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

// ホワイトボード，スタンプ，画像描画用の画像サイズ
#define DRAW_AREA_W 640
#define DRAW_AREA_H 480

// チャット表示用の画像サイズ
#define CHAT_AREA_W 640
#define CHAT_AREA_H 240

// 全体用の画像サイズ
#define CANVAS_W (DRAW_AREA_W)
#define CANVAS_H (DRAW_AREA_H + CHAT_AREA_H)

#define FONT_SIZE 16 // points

cv::Mat canvas;
cv::Mat draw_area;
cv::Mat chat_area;

// falseのときスレッドを終了
bool connected = true;

// ユーザデータ(登録，削除用)
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

// 起動時オプションの表示
int usage(char* program_name) {
	printf("%s server_ip server_port user_name\n", program_name);
	return -1;
}
/****************************************************************/

char* winname = "Stamp Client"; // ウィンドウ名

/***** スタンプクライアント ***********/
StampData stamp_data;
// スタンプ画像格納用
cv::Mat stamp[NUM_OF_STAMP];
/*********************************/

void on_mouse(int event, int x, int y, int flag, void* param) {
	std::string event_description;
	int ret;

	SOCKET sock = *(SOCKET*)param;

	PacketData packet_data;
	PacketData_init(&packet_data);

	if (0 <= x && x < DRAW_AREA_W && 0 <= y && y < DRAW_AREA_H) {

		/*** スタンプの幅，高さ，中心位置のx座標, 中心位置のy座標を計算 ***/
		int sw = stamp[stamp_data.stamp_id].cols;
		int sh = stamp[stamp_data.stamp_id].rows;
		int sx = x - sw / 2;
		int sy = y - sh / 2;

		// スタンプの矩形領域が画像の座標範囲に入っていれば
		if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

			// マウスイベントの種類による処理の分岐
			switch (event) {
				// マウスポインタが動いたとき
			case cv::EVENT_MOUSEMOVE:
				event_description += "MOUSE_MOVE";
				break;
				// 左ボタンがクリックされたとき
			case cv::EVENT_LBUTTONDOWN:
				event_description += "LBUTTON_DOWN";
				std::cerr << sx << ", " << sy << ", " << sw << ", " << sh << std::endl;

				/* スタンプデータを送信 */
				StampData_make(&stamp_data, x, y, stamp_data.stamp_id);
				StampData2PacketData(&stamp_data, &packet_data);

				ret = PacketData_send(sock, &packet_data);

				// 送信したデータをテキスト表示
				//StampData_print(&stamp_data);

				break;
				// 左ボタンが離されたとき
			case cv::EVENT_LBUTTONUP:
				event_description += "LBUTTON_UP";
				break;
			}
		}
	}

	// std::cerr << "mouse event: " << event_description << std::endl;

	PacketData_release(&packet_data);

	return;
}

// クライアントスレッド
unsigned __stdcall client_thread(void* p) {
	int key;

	// 通信ソケットを受け取る
	SOCKET sock = *(SOCKET*)p;

	// ソケットを非ブロッキングモードにする
	Socket_setNonBlockingMode(sock);

	PacketData packet_data;
	PacketData_init(&packet_data);

	/***** ドローエリア用画像の準備 *****/
	// メモリ割り当て
	draw_area.create(cv::Size(DRAW_AREA_W, DRAW_AREA_H), CV_8UC3);
	// 白で塗りつぶす
	draw_area = cv::Scalar(255, 255, 255);

	/***** チャットエリア用画像の準備 *****/

	// メモリ割り当て
	chat_area.create(cv::Size(CHAT_AREA_W, CHAT_AREA_H), CV_8UC3);
	// 薄青色で塗りつぶす
	chat_area = cv::Scalar(255, 127, 127);

	fprintf(stderr, "IntegratedClient: client_thread: pass 01\n");

	/***** 全体表示用画像（キャンバス）の準備 *****/
	// メモリ割り当て
	canvas.create(cv::Size(CANVAS_W, CANVAS_H), CV_8UC3);
	// 薄青色で塗りつぶす
	canvas = cv::Scalar(255, 255, 255);

	// ウィンドウを開く
	cv::namedWindow(winname);
	// リサイズする
	cv::resizeWindow(winname, CANVAS_W, CANVAS_H);

	update_draw_area(canvas, draw_area);
	update_chat_area(canvas, chat_area);

	// 画像の表示予約
	cv::imshow(winname, canvas);
	// 画像の描画&キー入力待ち(引数0の時ブロック，1以上のとき待機時間(ms))
	key = cv::waitKey(1);

	// マウスイベントが発生したときに呼び出す関数（コールバック関数）を登録
	cv::setMouseCallback(winname, on_mouse, &sock);

	while (connected) {

		int ret;

		// PacketDataを受信
		ret = PacketData_recv(sock, &packet_data);

		if (ret == 0) {
			// ソケットが閉じられたとき
		}
		else if (ret < 0) {
			// データ未受信のとき
		}
		else {
			cv::Mat src_roi, dst_roi;
			int sw, sh, sx, sy;
			switch (packet_data.cmd) {
			case DAT_STAMPDATA:
				fprintf(stderr, "Integrated Client: Recv: DAT_STAMPDATA\n");
				PacketData2StampData(&packet_data, &stamp_data);

				StampData_print(&stamp_data);

				// 受信データからスタンプIDを取得
				/* スタンプの幅sw，高さsh，中心位置のsx座標, 中心位置のsy座標を計算 */
				sw = stamp[stamp_data.stamp_id].cols;
				sh = stamp[stamp_data.stamp_id].rows;
				sx = stamp_data.x - sw / 2;
				sy = stamp_data.y - sh / 2;

				// スタンプの矩形領域が画像の座標範囲に入っていれば
				if (0 <= sx && sx + sw < draw_area.cols && 0 <= sy && sy + sh < draw_area.rows) {

					// ROI(x, y, w, h)を作る
					cv::Rect roi_rect(sx, sy, sw, sh);
					// draw_area上にROIを設定
					cv::Mat src_roi = draw_area(roi_rect);
					// スタンプをキャンバスにコピー
					stamp[stamp_data.stamp_id].copyTo(src_roi);

				}
				// draw_areaのアップデート
				update_draw_area(canvas, draw_area);
				break;

			default:
				fprintf(stderr, "Integrated Client: client_thread: Received Command: 0x%02x\n", packet_data.cmd);
				break;
			}
		}

		// 画像の表示予約
		cv::imshow(winname, canvas);
		// 画像の表示&キー入力
		key = cv::waitKey(1);

		if (key == 0x1b || key == 'q' || key == 'Q') {
			/*** ユーザデータをデータベースサーバから削除 ***/
			// UserDataを生成
			UserData_make(NULL, &user_data);

			UserData2PacketData(CMD_USERDATA_UNREGIST, &user_data, &packet_data);

			ret = PacketData_send(sock, &packet_data);

			connected = false;
		}
		else {

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

			if (0 <= keynum && keynum < NUM_OF_STAMP) {
				stamp_data.stamp_id = keynum;
				fprintf(stderr, "Integrated Client: stamp_data.stamp_id = %d\n", stamp_data.stamp_id);
			}
		}
	}
	
	PacketData_release(&packet_data);
	
	// キャンバス用メモリを解放
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

	/* 引数処理 */
	// IPアドレス(必須)，ポート番号(必須)，ユーザ名(必須), 文字色（デフォルト12）
	// 引数がプログラム本体を含めて5個（引数のみで4個）かどうかチェック
	if (argc == 4) {
		// 第一引数をサーバ名あるいはIPアドレスとして取得
		server = argv[1];
		// 第二引数をポート番号として取得
		port = atoi(argv[2]);

		sprintf_s(user_name, MAX_USERNAME_LEN, "%s", argv[3]);
		fprintf(stderr, "Username: %s, size: %zd\n", user_name, strlen(user_name));
	}
	else {
		usage(argv[0]);
		return -1;
	}

	/* スタンプ画像の読み込み */
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

	// ユーザデータ(登録用)を作成
	UserData_make(user_name, &user_data);
	// ユーザデータ情報を表示
	UserData_print(&user_data);
	// パケットデータに変換
	UserData2PacketData(CMD_USERDATA_REGIST, &user_data, &packet_data);
	// パケットデータ情報を表示
	PacketData_print(&packet_data);
	// パケットデータ情報(バイナリ情報)を表示
	//PacketData_print_hex(&packet_data);

	/***** クライアント用初期設定 *****/
	// ソケット用リソースの初期化（サーバ，クライアント共用）
	Socket_init();
	// クライアントソケット（接続用）の作成
	SOCKET sock = Socket_client(server, port);

	/*** ユーザデータをデータベースサーバに登録 ***/
	if (sock >= 0) {
		// パケットデータを送信
		ret = PacketData_send(sock, &packet_data);
	}
	else {
		fprintf(stderr, "Stamp Client: Cannot connect to server\n");
		return -1;
	}

	// スレッドの起動（登録）
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, client_thread, &sock, 0, NULL);

	// スレッドが終了するまで待機
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	// 通信用ソケットを閉じる
	Socket_close(sock);
	// ソケット用リソースを解放（サーバ，クライアント共用）
	Socket_release();

	return 0;
}