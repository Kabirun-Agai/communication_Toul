#pragma once
/*
 * Windows �� cvPutTextJP_Win2.h �ɂ���
 *
 * �g�p�������́A�m�[�T�|�[�g���S���ȐӔC�ł����p���������B
 * ���P�v�]��o�O�񍐂̓E�F���J���ł����A���C������邩�͑S���킩��܂���B
 * �t���[�ŏ��p���p���������Ă����匾���܂��񂪁A�ꉞ���쌠�͕������܂���B
 * �Ĕz�z�������͏o�����ǂ����ɖ��L���Ă��������B
 * �����u���O
 * �u���I�ȓS���͌^�̋L�^�v  http://blogs.yahoo.co.jp/gacchan_byg
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



// �o�[�W�����擾
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

// �r���h���[�h
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif


#define CV_EXT  CV_VERSION_STR CV_EXT_STR

#if 0
// ���C�u�����̃����N�i�s�v�ȕ��̓R�����g�A�E�g�j
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
	// cv::Mat ���͂ݏo�������̋������w�肷��
	//
	enum JP_WRAP_TYPE { JP_WRAP_VOID, JP_WRAP_CR };

private:
	LOGFONT			lFont;			// �t�H���g�擾�p�f�[�^
	HFONT			hFont;			// �t�H���g�n���h��
	HFONT			oldFont;		// ���X�̃t�H���g�̃n���h��
	HDC				hdc;			// �t�H���g�p�̃f�o�C�X�R���e�L�X�g
	TEXTMETRIC		tm;				// �e�L�X�g���g���b�N
	int				baseLineDist;	// �s�T�C�Y
	int				ggo_gray;		// �O���t���~���w��
	cv::Point2i		iCursor;		// �\���p���W
	JP_WRAP_TYPE	jpWrapType;		// �I�[�o�[�t���[���̃��b�v��`
	//
public:
	//
	// �R���X�g���N�^
	//
	//	std::string &_fontName = "���C���I"		�t�H���g�t�F�[�X���w�肵�܂�
	//	int _fontSize = 32						�t�H���g�T�C�Y���w�肵�܂�
	//	int _weight = 0							�t�H���g�̕�(?)���w�肵�܂�
	//		windows.h �ł̎Q�l
	//		FW_DONTCARE = 0;
	//		FW_THIN = 100;
	//		FW_NORMAL = 400;
	//		FW_BOLD = 700;
	//	CV_JP_WRAP_TYPE _wrapType = CV_JP_WRAP_VOID		cv::Mat ���͂ݏo�������̋������w�肵�܂�
	//
	cvPutTextJP(
		const std::string &_fontName = "���C���I",
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
	// �s�̍������擾���܂�
	//
	int getBaselineDistance() {
		return( baseLineDist );
	}
	//
	// �f�X�g���N�^
	//
	~cvPutTextJP() {
		SelectObject( hdc, oldFont );
		DeleteObject( hFont );
		ReleaseDC( NULL, hdc );
	}
	//
	// �����̏o�͕����擾
	//
	int getCharWidth( const TCHAR *c ) {
		GLYPHMETRICS gm;
		memset( &gm, 0, sizeof( gm ) );
		const MAT2 mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

		UINT code = 0;
#if UNICODE
		// unicode�̏ꍇ�A�����R�[�h�͒P���Ƀ��C�h������UINT�ϊ��ł�
		code = (UINT) *c;
#else
		// �}���`�o�C�g�����̏ꍇ�A
		// 1�o�C�g�����̃R�[�h��1�o�C�g�ڂ�UINT�ϊ��A
		// 2�o�C�g�����̃R�[�h��[�擱�R�[�h]*256 + [�����R�[�h]�ł�
		if( IsDBCSLeadByte( *c ) ) {
			code = ( ( (BYTE) c[ 0 ] << 8 ) | ( (BYTE) c[ 1 ] ) );
		}
		else {
			code = c[ 0 ];
		}
#endif
		DWORD rSize = ::GetGlyphOutline(	// �ŏ���GetGryphOutline�֐���cbBuffer�p�����[�^��0��n���A
			hdc,
			code,			// ���ʂ̃f�[�^���i�[���邽�߂̃T�C�Y������Ԃ��Ă��炤
			GGO_GRAY8_BITMAP,	// TT �Ȃ�A������ GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			0,
			NULL,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// ���s�����ꍇ GDI_ERROR���Ԃ�i0�łȂ��̂Œ��Ӂj
		// �������o�b�t�@�T�C�Y���Ԃ��Ă���
		char *buff = new char[ rSize ];		// ����ꂽ�T�C�Y�Ńo�b�t�@�𐶐�����
		rSize = GetGlyphOutline(		// �o�b�t�@�̃T�C�Y�Ɛ擪�A�h���X��n���A
			hdc,							// �A�E�g���C���̃f�[�^�������Ɏ擾����
			code,
			GGO_GRAY8_BITMAP,	// TT �Ȃ�A������ GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			rSize,
			buff,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// ���s�����ꍇ GDI_ERROR���Ԃ�i0�łȂ��̂Œ��Ӂj
		return( gm.gmCellIncX );
	}
	//
	// �����ʒu��ݒ肷��
	//
	//	cv::Point2i pos		�����̍�����W
	//
	//	�\���ʒu�� Point2i pos �͕�����̍���̎w��ł��BOpenCV �� putText() ��
	//	�������f�t�H�[���g�̂悤�ł����A�ʓ|�������̂ō���I�����[�ł��B
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
	// 1�����`��
	//
	//	cv::Mat &mat					�����ݑΏۂ̉摜���w�肵�܂��@���݂� CV_8UC3 �̂�
	//	TCHAR *c						�������ޕ�����ł�
	//	cv::Scalar sc					�o�͐F���w�肵�܂�
	//
	//	return:		true == �␳�����A�܂��͂͂ݏo������`�悵�Ȃ�����
	//				false == ���Ȃ�
	//
	bool putChar( cv::Mat &mat, TCHAR *c, cv::Scalar sc ) {
		CV_Assert( mat.type() == CV_8UC3 );
		//
		bool wrap = false;
		const MAT2 mat2 = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
		char *buff = NULL;
		//
		// CR LF �̑Ή��E�E�E����Ȃ�����
		//
		if( *c == 0x0d ) {
			// CR
			iCursor.x = 0;
			iCursor.y += baseLineDist;	// ���Ȃ�K��
			goto end;
		}
		if( *c == 0x0a ) {
			// LF
			iCursor.x = 0;
			iCursor.y += baseLineDist;	// ���Ȃ�K��
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
		// unicode�̏ꍇ�A�����R�[�h�͒P���Ƀ��C�h������UINT�ϊ��ł�
		code = (UINT) *c;
#else
		// �}���`�o�C�g�����̏ꍇ�A
		// 1�o�C�g�����̃R�[�h��1�o�C�g�ڂ�UINT�ϊ��A
		// 2�o�C�g�����̃R�[�h��[�擱�R�[�h]*256 + [�����R�[�h]�ł�
		if( IsDBCSLeadByte( *c ) ) {
			code = ( ( (BYTE) c[ 0 ] << 8 ) | ( (BYTE) c[ 1 ] ) );
		}
		else {
			code = c[ 0 ];
		}
#endif
		DWORD rSize = ::GetGlyphOutline(	// �ŏ���GetGryphOutline�֐���cbBuffer�p�����[�^��0��n���A
			hdc,
			code,			// ���ʂ̃f�[�^���i�[���邽�߂̃T�C�Y������Ԃ��Ă��炤
			GGO_GRAY8_BITMAP,	// TT �Ȃ�A������ GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			0,
			NULL,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// ���s�����ꍇ GDI_ERROR���Ԃ�i0�łȂ��̂Œ��Ӂj
		// �������o�b�t�@�T�C�Y���Ԃ��Ă���
		buff = new char[ rSize ];		// ����ꂽ�T�C�Y�Ńo�b�t�@�𐶐�����
		rSize = GetGlyphOutline(		// �o�b�t�@�̃T�C�Y�Ɛ擪�A�h���X��n���A
			hdc,							// �A�E�g���C���̃f�[�^�������Ɏ擾����
			code,
			GGO_GRAY8_BITMAP,	// TT �Ȃ�A������ GGO_BEZIER|GGO_NATIVE|GGO_UNHINTED,
			&gm,
			rSize,
			buff,
			&mat2 );
		CV_Assert( rSize != GDI_ERROR );	// ���s�����ꍇ GDI_ERROR���Ԃ�i0�łȂ��̂Œ��Ӂj
		//
		// �����œ���ꂽ�O���t�̏������s��
		//
		if( rSize > 0 ) {
			//
			// �܂��ʒu�ƃ��b�v�m�F
			//
			int width = gm.gmCellIncX;
			if( iCursor.x + width >= mat.size().width ) {
				// ���b�v���˂΁E�E�E
				wrap = true;
				if( jpWrapType == JP_WRAP_VOID ) {
					goto end;
				}
				// ���낢����Ȃ���
				iCursor.x = 0;
				iCursor.y += baseLineDist;	// ���Ȃ�K��
			}
			if( iCursor.y + baseLineDist >= mat.size().height ) {
				// ���b�v���˂΁E�E�E
				wrap = true;
				if( jpWrapType == JP_WRAP_VOID ) {
					goto end;
				}
				// ���낢����Ȃ���
				iCursor.x = 0;
				iCursor.y = 0;
			}
			// iOfs_x, iOfs_y : �����o���ʒu(����)
			// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
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
							// ����
							double alp1 = (double) Alpha / 255.0;		// �����F�̊���
							double B0 = *( p );	// ���̐F
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
			delete[] buff; // �o�b�t�@���
		}
		return( wrap );
	}
	//
	// ������̏o�͕����擾
	//
	int getTextWidth( TCHAR *c ) {
		int width = 0;
		while( *c ) {
			width += getCharWidth( c );
#if UNICODE
			// unicode�̏ꍇ�A�����R�[�h�͒P���Ƀ��C�h������UINT�ϊ��ł�
			c += sizeof( UINT );
#else
			c += ( IsDBCSLeadByte( *c ) ? 2 : 1 );
#endif
		}
		return( width );
	}
	//
	// ������`��
	//
	//	cv::Mat &mat					�����ݑΏۂ̉摜���w�肵�܂��@���݂� CV_8UC3 �̂�
	//	TCHAR *c						�������ޕ�����ł�
	//	cv::Scalar sc					�o�͐F���w�肵�܂�
	//
	void putText( cv::Mat &mat, TCHAR *c, cv::Scalar sc ) {
		CV_Assert( mat.type() == CV_8UC3 );
		//
		while( *c ) {
			if( putChar( mat, c, sc ) && ( jpWrapType == JP_WRAP_VOID ) ) {
				return;
			}
#if UNICODE
			// unicode�̏ꍇ�A�����R�[�h�͒P���Ƀ��C�h������UINT�ϊ��ł�
			c += sizeof( UINT );
#else
			c += ( IsDBCSLeadByte( *c ) ? 2 : 1 );
#endif
		}
	}
};