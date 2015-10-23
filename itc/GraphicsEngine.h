#pragma once

#include <windows.h>
#include <tchar.h>
#include "gdiplus_mem.h"
using namespace Gdiplus;

#include <math.h>

#define IMG_SCALENONE 1
#define IMG_SCALEDOWN 2
#define IMG_SCALEUP 4
#define IMG_SCALEALL 6
#define ART_SCALEMODEHEIGHT 1
#define ART_SCALEMODESIZE 2

struct GraphicsEngineOptions
{
	int nBorderType; // 0 - rect, 1 - rounded
	float fBorderRadius; // radius of rounded borders
	int nDisplayAlpha; // display alpha in %
	bool bDisplayTopMost; // attempt to keep display at top of z-order
	int nMaxWidth; // max width of display in pixels (-1 to disable)
	int nArtScaleMode; // 0 - scale to text height, 1 - scale to fixed size
	StringAlignment saAlign;
	bool bForceToWidth;
	GraphicsEngineOptions()
	{
		nBorderType = 0;
		fBorderRadius = 40.0f;
		nDisplayAlpha = 0;
		bDisplayTopMost = true;
		nMaxWidth = -1;
		nArtScaleMode = ART_SCALEMODEHEIGHT;
		saAlign = StringAlignmentNear;
		bForceToWidth = false;
	}
};

#include "node.h"

class GraphicsEngine
{
public:
	GraphicsEngine(int id);
	~GraphicsEngine();

	void BeginRender( int nDisplayLayout=0 );
	void AddString( const wchar_t* tcString, bool bVertical=false, int nFontStyle=0, Gdiplus::REAL rSize=18, Color cColor=Color(0,0,0), TCHAR *tcFont=_T("Segoe UI"), Color cOutlineColor=Color(0,0,0,0), REAL fOutlineWidth=1.0f, float fDropShadowOffset=0.0f, Color cDropShadow=Color(0,0,0,0) );
	void AddImage( TCHAR *tcPath, bool bVertical=false, Color cOutlineColor=Color(0,0,0,0), REAL fOutlineWidth=1.0f, REAL rMaxDimension=0.0f, int nFlags=IMG_SCALENONE );
	void AddArt( TCHAR *tcPath, bool bVertical=false, Color cOutlineColor=Color(0,0,0,0), REAL fOutlineWidth=1.0f, REAL rMaxDimension=0.0f, int nFlags=IMG_SCALENONE );
	void AddShape( ShapeType type, ShapeStyle style, RectF &rfSize, bool bVertical=false, Color cOutlineColor=Color(0,0,0), REAL fOutlineWidth=1.0f, Color cColor=(0,0,0), unsigned int uiCustomFill=0 );
	Node *AddBox( bool bVertical=false, bool bAutoEntryType=true, REAL rPaddingX=0.0f, REAL rPaddingY=0.0f );
	void BeginLine( void );
	void EndLine( void );
	void EndRender( POINT point, Bitmap *bmp, RECT rcScreen );
	void EndRender( POINT point, RECT rcScreen , bool bSave=false, Bitmap **bmpSave=NULL );
	void AscendTree();
	REAL GetTextHeight( TCHAR *name, REAL size, int style );

	void SetEntryType( bool bSiblingEntry );
	void SetDisplayBorder( Color &color, REAL &width ) { renderList->SetDisplayBorder( color, width ); return; }
	void SetDisplayBackground( Color &color ) { renderList->SetDisplayBackground( color ); return; }
	void SetOptions( GraphicsEngineOptions &geo ) { _geoOptions = geo; return; }

	void DisplayHide( bool bFade );
	void DisplayShow( bool bFade );
	bool IsDisplayVisible( void ) { return ( 0 != IsWindowVisible( _hwndDisplayWindow ) ); };

	ULONG CopyPNGToMemory(Bitmap* src, void* dest);

private:
	double GetLineHeights( void );
	double GetLineWidths( void );
	void AdjustLineWidths( double dMax );
	void AlignLines( void );
	void AlignArt( void );

	static DWORD WINAPI WindowThreadProc( LPVOID lpParam );
	DWORD WindowThreadProc( void );

	static LRESULT CALLBACK LightWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT GraphicsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	UINT BitmapIntoLayeredWindow(HWND hWnd, Bitmap &bmp, POINT &point);
	Bitmap *CreateGdiplusBitmapInStream(Bitmap *pbmSource);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	int RoundUpPow2(int x);

	HANDLE _hWindowThread;
	HWND _hwndDisplayWindow;
	wchar_t _wcDisplayWindowTitle[32];
	RenderList *renderList;
	Node *currentRLPosition;
	Node *_ndArtBox;
	Node *_ndInfoBox;
	bool _bSiblingEntry;
	bool _bAutoEntrySet;
	int _nAlpha;
	int _nDisplayLayout;
	GraphicsEngineOptions _geoOptions;
	UINT_PTR _uiScheduleTimer;
};