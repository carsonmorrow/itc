#ifndef __NODE_H__INCLUDED
#define __NODE_H__INCLUDED

//#define NODE_DRAWDEBUGBOX

#include <crtdbg.h>

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <math.h>
#include "gdiplus_mem.h"
using namespace Gdiplus;

#define RL_POSITIONED 1
#define RL_MEASURED 2
#define RL_RENDERED 4
#define RL_LAYOUTVERT 8

struct TextData
{
	Font *font;
	StringFormat *sf;
	SolidBrush *sb;
	TCHAR *tcText;
	Pen *pnOutline;
	StringAlignment saAlign;
	float fDropShadowOffset;
	SolidBrush *drop;
	TextData()
	{
		font = NULL;
		sf = NULL;
		sb = NULL;
		drop = NULL;
		tcText = NULL;
		pnOutline = NULL;
		saAlign = StringAlignmentNear;
		fDropShadowOffset = 0.0f;
	}
};

enum ShapeType { ShapeCircle, ShapeRectangle, ShapeStar, ShapeHexagon, ShapeVolume };
enum ShapeStyle { FillAll, FillNone, FillHalfLR, FillHalfRL, FillHalfTB, FillHalfBT, FillCustom };
struct ShapeData
{
	Pen *outline;
	SolidBrush *sb;
	RectF *size;
	ShapeType type;
	ShapeStyle style;
	unsigned int customFill;
	ShapeData()
	{
		outline = NULL;
		sb = NULL;
		size = NULL;
		type = ShapeCircle;
		style = FillAll;
		customFill = 0;
	}
};

struct ImageData
{
	Bitmap *image;
	Pen *pnOutline;
	RectF *rfSize;
	ImageData()
	{
		image = NULL;
		pnOutline = NULL;
		rfSize = NULL;
	}
};

struct RootData
{
	Pen *pnBorder;
	SolidBrush *sbBkgnd;
	RootData()
	{
		pnBorder = NULL;
		sbBkgnd = NULL;
	}
};

struct Node
{
	Node *next;
	Node *prev;
	Node *parent;
	Node *child;
	int nType;
	int nFlags;
	REAL fMaxTextH;
	void *lpvData;
	RectF *rf; // rf is node's box
	RectF *rfBound; // rfbound is node U previous box
	PointF *pfPadding; // x is x offset, etc
	Node()
	{
		next = NULL;
		prev = NULL;
		parent = NULL;
		child = NULL;
		nType = 0;
		rf = NULL;
		rfBound = NULL;
		lpvData = NULL;
		pfPadding = NULL;
		nFlags = 0;
		fMaxTextH = 0;
	}
};

class RenderList
{
public:
	RenderList();
	~RenderList();

	Node *GetRoot() { return root; };
	void SetDisplayBorder( Color &color, REAL &width ) { RootData *lpRD = ((RootData*)root->lpvData); lpRD->pnBorder->SetColor( color ); lpRD->pnBorder->SetWidth( width ); return; }
	void SetDisplayBackground( Color &color ) { RootData *lpRD = ((RootData*)root->lpvData); lpRD->sbBkgnd->SetColor( color ); return; }

	Node *AddChild( Node *parent );
	Node *AddSibling( Node *child );

	void ClearList();
	void MeasurePass( Graphics *graphics );
	void PositionPass( Graphics *graphics );
	void RenderPass( Graphics *graphics );

	Node *GetLastSibling( Node *parent );
private:
	Node *GetBottomMost( Node *from );
	Node *GetNextForProcessing( Node *from, int nFlag );

	Node *DeleteNode( Node *node );
	Node *MeasureNode( Node *node, Graphics *graphics );
	Node *PositionNode( Node *node, Graphics *graphics );
	Node *RenderNode( Node *node, Graphics *graphics );
	Node *PropagateBounds( Node *parent, RectF *rectBounds );

	Node *root;
};

#endif