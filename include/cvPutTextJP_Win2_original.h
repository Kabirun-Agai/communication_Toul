#pragma once
/*
 * Windows 版 cvPutTextJP_Win2.h について
 *
 * 使用される方は、ノーサポート完全自己責任でご利用ください。
 * 改善要望やバグ報告はウェルカムですが、いつ修正されるかは全くわかりません。
 * フリーで商用利用も何をしても文句言いませんが、一応著作権は放棄しません。
 * 再配布される方は出所をどこかに明記してください。
 * 当方ブログ
 * 「私的な鉄道模型の記録」  http://blogs.yahoo.co.jp/gacchan_byg
 */

//#include "stdafx.h"
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <stdio.h>


#ifdef _OPENMP
#include <omp.h>
#endif

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/legacy/legacy.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/tracking.hpp"



// バージョン取得
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

// ビルドモード
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif


#define CV_EXT  CV_VERSION_STR CV_EXT_STR

#if 0
// ライブラリのリンク（不要な物はコメントアウト）
#pragma comment(lib,"opencv_calib3d" CV_EXT )
#pragma comment(lib,"opencv_contrib" CV_EXT )
#pragma comment(lib,"opencv_core" CV_EXT )
#pragma comment(lib,"opencv_features2d" CV_EXT )
//#pragma comment(lib,"opencv_ffmpeg" CV_VERSION_STR "_64.lib" )
#pragma comment(lib,"opencv_flann" CV_EXT )
#pragma comment(lib,"opencv_gpu" CV_EXT )
#pragma comment(lib,"opencv_highgui" CV_EXT )
#pragma comment(lib,"opencv_imgproc" CV_EXT )
#pragma comment(lib,"opencv_legacy" CV_EXT )
#pragma comment(lib,"opencv_ml" CV_EXT )
#pragma comment(lib,"opencv_nonfree" CV_EXT )
#pragma comment(lib,"opencv_objdetect" CV_EXT )
#pragma comment(lib,"opencv_ocl" CV_EXT )
#pragma comment(lib,"opencv_photo" CV_EXT )
#pragma comment(lib,"opencv_stitching" CV_EXT )
#pragma comment(lib,"opencv_superres" CV_EXT )
#pragma comment(lib,"opencv_ts" CV_EXT )
#pragma comment(lib,"opencv_video" CV_EXT )
#pragma comment(lib,"opencv_videostab" CV_EXT )
#endif


#if 0
cv::Scalar cvBlack = cv::Scalar( 0, 0, 0 );			// BGRa Black
cv::Scalar cvRed = cv::Scalar( 0, 0, 255 );			// BGRa Red
cv::Scalar cvGreen = cv::Scalar( 0, 255, 0 );			// BGRa Green
cv::Scalar cvYellow = cv::Scalar( 0, 255, 255 );		// BGRa Yellow
cv::Scalar cvBlue = cv::Scalar( 255, 0, 0 );			// BGRa Blue
cv::Scalar cvViolet = cv::Scalar( 255, 0, 255 );		// BGRa Purple
cv::Scalar cvSkyBlue = cv::Scalar( 255, 255, 0 );		// BGRa SkyBlue
cv::Scalar cvWhite = cv::Scalar( 255, 255, 255 );		// BGRa White
#endif


#define GRAY_LEVEL(_X_) ( 255l * (_X_) / ( iLevel -1 ))

class cvPutTextJP {
public:
	//
	// cv::Mat をはみ出した時の挙動を指定する
	//
	enum JP_WRAP_TYPE { JP_WRAP_VOID, JP_WRAP_CR };

private:
	LOGFONT			lFont;			// フォント取得用データ
	HFONT			hFont;			// フォントハンドル
	HFONT			oldFont;		// 元々のフォントのハンドル
	HDC				hdc;			// フォント用のデバイスコンテキスト
	TEXTMETRIC		tm;				// テキストメトリック
	int				baseLineDist;	// 行サイズ
	int				ggo_gray;		// グリフの諧調指定
	cv::Point2i		iCursor;		// 表示用座標
	JP_WRAP_TYPE	jpWrapType;		// オーバーフロー時のラップ定義
	//
public:
	//
	// コンストラクタ
	//
	//	std::string &_fontName = "メイリオ"		フォントフェースを指定します
	//	int _fontSize = 32						フォントサイズを指定します
	//	int _weight = 0							フォントの幅(?)を指定します
	//		windows.h での参考
	//		FW_DONTCARE = 0;
	//		FW_THIN = 100;
	//		FW_NORMAL = 400;
	//		FW_BOLD = 700;
	//	CV_JP_WRAP_TYPE _wrapType = CV_JP_WRAP_VOID		cv::Mat をはみ出した時の挙動を指定します
	//
	cvPutTextJP(
		const std::string &_fontName = "メイリオ",
		const int _fontSize = 32,
		const int _weight = 0,
		const JP_WRAP_TYPE _wrapType = JP_WRAP_VOID,
		const int _gray = GGO_GRAY8_BITMAP ) :
		iCursor( 0, 0 ) {
		//
		CV_Assert( ( _gray == GGO_GRAY2_BITMAP ) || ( _gray == GGO_GRAY4_BITMAP ) || ( _gray == GGO_GRAY8_BITMAP ) || ( _gray == GGO_BITMAP ) );
		//
		lFont.lfHeight = _fontSize;
		lFont.lfWidth = 0;
		lFont.lfEscapement = 0;
		lFont.lfOrientation = 0;
		lFont.lfWeight = _weight;
		lFont.lfItalic = 0;
		lFont.lfUnderline = 0;
		lFont.lfStrikeOut = 0;
		lFont.lfCharSet = DEFAULT_CHARSET;				// SHIFTJIS_CHARSET;
		lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;		// OUT_TT_ONLY_PRECIS; // ??
		lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lFont.lfQuality = ANTIALIASED_QUALITY;			// DEFAULT_QUALITY;	PROOF_QUALITY;
		lFont.lfPitchAndFamily = ( DEFAULT_PITCH | FF_DONTCARE );		// ( FIXED_PITCH | FF_MODERN );
		CV_Assert( strcpy_s( (char*)lFont.lfFaceName, sizeof( lFont.lfFaceName ), _fontName.c_str() ) == 0 );
		CV_Assert( ( hFont = CreateFontIndirect( &lFont ) ) != NULL );
		//
		jpWrapType = _wrapType;
		ggo_gray = _gray;
		//
		hdc = GetDC( NULL );
		oldFont = (HFONT) SelectObject( hdc, hFont );
		GetTextMetrics( hdc, &tm );
		//
		baseLineDist = tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
	}
	//
	// 行の高さを取得します
	//
	int getBaselineDistance() {
		return( baseLineDist );
	}
	//
	// デストラクタ
	//
	~cvPutTextJP() {
		SelectObject( hdc, oldFont );
		DeleteObject( hFont );
		ReleaseDC( NULL, hdc );
	}
	//
	// 文字の出力幅を取得
	//
	int getCharWidth( const TCHAR *c ) {
		GLYPHMETRICS gm;
		memset( &gm, 0, sizeof( gm ) );
		const MAT2 mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

		UINT code = 0;
#if UNICODE
		// unicodeの場合、文字コードは単純にワイド文字のUINT変換です
		code = (UINT) *c;
#else
		// マルチバイト文字の場合、
		// 1バイト文字のコードは1バイト目のUINT変換、
		// 2バイト文字のコードは[先導コード]*256 + [文字コード]です
		if( IsDBCSLeadByte( *c ) ) {
			code = ( ( (BYTE) c[ 0 ] << 8 ) | ( (BYTE) c[ 1 ] ) );
		}
		else {
			code = c[ 0 ];
		}
#endif
		DWORD rSize = ::GetGlyphOutline(	// 最初のGetGryphOutline関数はcbBufferパラメータに0を渡し、
			hdc,
			code,			// 結果のデータを格納するためのサイズだけを返してもらう
			GGO_GRAY8_BITMAP,	// TT なら、こっち GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			0,
			NULL,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// 失敗した場合 GDI_ERRORが返る（0でないので注意）
		// 正しいバッファサイズが返ってきた
		char *buff = new char[ rSize ];		// 得られたサイズでバッファを生成する
		rSize = GetGlyphOutline(		// バッファのサイズと先頭アドレスを渡し、
			hdc,							// アウトラインのデータをそこに取得する
			code,
			GGO_GRAY8_BITMAP,	// TT なら、こっち GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			rSize,
			buff,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// 失敗した場合 GDI_ERRORが返る（0でないので注意）
		return( gm.gmCellIncX );
	}
	//
	// 文字位置を設定する
	//
	//	cv::Point2i pos		文字の左上座標
	//
	//	表示位置の Point2i pos は文字列の左上の指定です。OpenCV の putText() は
	//	左下がデフォールトのようですが、面倒くさいので左上オンリーです。
	//
	void setLocate( cv::Point2i pos ) {
		iCursor = pos;
		//
		if( iCursor.x < 0 ) {
			iCursor.x = 0;
		}
		if( iCursor.y < 0 ) {
			iCursor.y = 0;
		}
	}
	//
	// 1文字描画
	//
	//	cv::Mat &mat					書込み対象の画像を指定します　現在は CV_8UC3 のみ
	//	TCHAR *c						書き込む文字列です
	//	cv::Scalar sc					出力色を指定します
	//
	//	return:		true == 補正した、またははみ出すから描画しなかった
	//				false == 問題なし
	//
	bool putChar( cv::Mat &mat, TCHAR *c, cv::Scalar sc ) {
		CV_Assert( mat.type() == CV_8UC3 );
		//
		bool wrap = false;
		const MAT2 mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
		char *buff = NULL;
		//
		// CR LF の対応・・・いらないかも
		//
		if( *c == 0x0d ) {
			// CR
			iCursor.x = 0;
			iCursor.y += baseLineDist;	// かなり適当
			goto end;
		}
		if( *c == 0x0a ) {
			// LF
			iCursor.x = 0;
			iCursor.y += baseLineDist;	// かなり適当
			goto end;
		}
		//
		int iLevel = 2;	// GGO_BITMAP
		switch( ggo_gray ) {
			case GGO_GRAY2_BITMAP:
				iLevel = 5;
				break;
			case GGO_GRAY4_BITMAP:
				iLevel = 17;
				break;
			case GGO_GRAY8_BITMAP:
				iLevel = 65;
				break;
		}
		GLYPHMETRICS gm;
		memset( &gm, 0, sizeof( gm ) );

		UINT code = 0;
#if UNICODE
		// unicodeの場合、文字コードは単純にワイド文字のUINT変換です
		code = (UINT) *c;
#else
		// マルチバイト文字の場合、
		// 1バイト文字のコードは1バイト目のUINT変換、
		// 2バイト文字のコードは[先導コード]*256 + [文字コード]です
		if( IsDBCSLeadByte( *c ) ) {
			code = ( ( (BYTE) c[ 0 ] << 8 ) | ( (BYTE) c[ 1 ] ) );
		}
		else {
			code = c[ 0 ];
		}
#endif
		DWORD rSize = ::GetGlyphOutline(	// 最初のGetGryphOutline関数はcbBufferパラメータに0を渡し、
			hdc,
			code,			// 結果のデータを格納するためのサイズだけを返してもらう
			GGO_GRAY8_BITMAP,	// TT なら、こっち GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			0,
			NULL,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// 失敗した場合 GDI_ERRORが返る（0でないので注意）
		// 正しいバッファサイズが返ってきた
		buff = new char[ rSize ];		// 得られたサイズでバッファを生成する
		rSize = GetGlyphOutline(		// バッファのサイズと先頭アドレスを渡し、
			hdc,							// アウトラインのデータをそこに取得する
			code,
			GGO_GRAY8_BITMAP,	// TT なら、こっち GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			rSize,
			buff,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// 失敗した場合 GDI_ERRORが返る（0でないので注意）
		//
		// ここで得られたグリフの処理を行う
		//
		if( rSize > 0 ) {
			//
			// まず位置とラップ確認
			//
			int width = gm.gmCellIncX;
			if( iCursor.x + width >= mat.size().width ) {
				// ラップせねば・・・
				wrap = true;
				if( jpWrapType == JP_WRAP_VOID ) {
					goto end;
				}
				// いろいろやらないと
				iCursor.x = 0;
				iCursor.y += baseLineDist;	// かなり適当
			}
			if( iCursor.y + baseLineDist >= mat.size().height ) {
				// ラップせねば・・・
				wrap = true;
				if( jpWrapType == JP_WRAP_VOID ) {
					goto end;
				}
				// いろいろやらないと
				iCursor.x = 0;
				iCursor.y = 0;
			}
			// iOfs_x, iOfs_y : 書き出し位置(左上)
			// iBmp_w, iBmp_h : フォントビットマップの幅高
			int iOfs_x = gm.gmptGlyphOrigin.x + iCursor.x;
			int iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y + iCursor.y;
			int iBmp_w = gm.gmBlackBoxX + ( 4 - ( gm.gmBlackBoxX % 4 ) ) % 4;
			int iBmp_h = gm.gmBlackBoxY;
			//
#pragma omp parallel for
			for( int y = iOfs_y; y < iOfs_y + iBmp_h; y++ ) {
				for( unsigned int x = iOfs_x; x < iOfs_x + gm.gmBlackBoxX; x++ ) {
					DWORD Alpha = GRAY_LEVEL( buff[ x - iOfs_x + iBmp_w *( y - iOfs_y ) ] );
					UCHAR *p = mat.ptr( y, x );
					if( Alpha != 0 ) {
						if( Alpha == 255 ) {
							*( p ) = (UCHAR) sc[ 0 ];
							*( p + 1 ) = (UCHAR) sc[ 1 ];
							*( p + 2 ) = (UCHAR) sc[ 2 ];
						}
						else {
							// 混ぜ
							double alp1 = (double) Alpha / 255.0;		// 書く色の割合
							double B0 = *( p );	// 元の色
							double G0 = *( p + 1 );
							double R0 = *( p + 2 );
							double B = B0 + ( sc[ 0 ] - B0 )* alp1;
							double G = G0 + ( sc[ 1 ] - G0 )* alp1;
							double R = R0 + ( sc[ 2 ] - R0 )* alp1;
							*( p ) = (UCHAR) B;
							*( p + 1 ) = (UCHAR) G;
							*( p + 2 ) = (UCHAR) R;
						}
					}
				}
			}
		}
		iCursor.x += gm.gmCellIncX;
end:
		if( buff != NULL ) {
			delete[] buff; // バッファ解放
		}
		return( wrap );
	}
	//
	// 文字列の出力幅を取得
	//
	int getTextWidth( TCHAR *c ) {
		int width = 0;
		while( *c ) {
			width += getCharWidth( c );
#if UNICODE
			// unicodeの場合、文字コードは単純にワイド文字のUINT変換です
			c += sizeof( UINT );
#else
			c += ( IsDBCSLeadByte( *c ) ? 2 : 1 );
#endif
		}
		return( width );
	}
	//
	// 文字列描画
	//
	//	cv::Mat &mat					書込み対象の画像を指定します　現在は CV_8UC3 のみ
	//	TCHAR *c						書き込む文字列です
	//	cv::Scalar sc					出力色を指定します
	//
	void putText( cv::Mat &mat, TCHAR *c, cv::Scalar sc ) {
		CV_Assert( mat.type() == CV_8UC3 );
		//
		while( *c ) {
			if( putChar( mat, c, sc ) && ( jpWrapType == JP_WRAP_VOID ) ) {
				return;
			}
#if UNICODE
			// unicodeの場合、文字コードは単純にワイド文字のUINT変換です
			c += sizeof( UINT );
#else
			c += ( IsDBCSLeadByte( *c ) ? 2 : 1 );
#endif
		}
	}
};