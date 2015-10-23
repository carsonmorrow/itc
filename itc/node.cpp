#include "node.h"
#include "memcheck.h"
RenderList::RenderList()
{
	root = new Node;
	RootData *rd = new RootData;
	rd->pnBorder = new Pen(Color(255, 0, 0), 2.0f);
	rd->pnBorder->SetAlignment(PenAlignmentInset);
	rd->sbBkgnd = new SolidBrush(Color(255, 255, 255));
	root->lpvData = rd;
	root->pfPadding = new PointF(0.0f, 0.0f);
}

RenderList::~RenderList()
{
	ClearList();

	// Delete root
	delete root->rf;
	delete root->rfBound;
	delete root->pfPadding;

	RootData *lpRD = ((RootData*)root->lpvData);
	delete lpRD->pnBorder;
	delete lpRD->sbBkgnd;
	delete lpRD;

	delete root;
	root = NULL;
}

void RenderList::ClearList()
{
	Node *current = root->child;

	if (NULL == current)
		return;
	else
		DeleteNode(current);

	// Delete root
	delete root->rf;
	delete root->rfBound;
	delete root->pfPadding;

	RootData *lpRD = ((RootData*)root->lpvData);
	delete lpRD->pnBorder;
	delete lpRD->sbBkgnd;
	delete lpRD;

	delete root;
	root = NULL;

	// Create root
	root = new Node;
	RootData *rd = new RootData;
	rd->pnBorder = new Pen(Color(255, 0, 0), 2.0f);
	rd->pnBorder->SetAlignment(PenAlignmentInset);
	rd->sbBkgnd = new SolidBrush(Color(255, 255, 255));
	root->lpvData = rd;
	root->pfPadding = new PointF(0.0f, 0.0f);
}

// Recursively delete nodes
Node *RenderList::DeleteNode(Node *node)
{
	Node *temp = GetBottomMost(node);
	Node *current = NULL;

	if (NULL != temp->prev)
	{
		current = temp->prev;
		current->next = NULL;
	}
	else
	{
		current = temp->parent;
		current->child = NULL;
	}

	if (1 == temp->nType)
	{
		TextData *lpTD = ((TextData*)temp->lpvData);
		delete lpTD->font;
		delete lpTD->sb;
		delete lpTD->sf;
		delete lpTD->drop;
		delete lpTD->pnOutline;
		free(lpTD->tcText);
		delete lpTD;
	}
	else if (2 == temp->nType)
	{
		ImageData *lpID = ((ImageData*)temp->lpvData);
		delete lpID->image;
		delete lpID->pnOutline;
		delete lpID->rfSize;
		delete lpID;
	}
	/* else if ( 3 == temp->nType )
	{
		// box: nothing to clean up
	}*/
	else if (4 == temp->nType)
	{
		ShapeData *sd((ShapeData*)temp->lpvData);
		delete sd->outline;
		delete sd->sb;
		delete sd->size;
		delete sd;
	}

	delete temp->rf;
	delete temp->rfBound;
	delete temp->pfPadding;

	delete temp;

	if (root == current)
		return NULL;
	else
		return DeleteNode(current);
}

// Recursively walk tree to get next node missing nFlag
Node *RenderList::GetNextForProcessing(Node *from, int nFlag)
{
	Node *current = from;

	if (!(current->nFlags & nFlag))
	{
		// this one needs to be processed
		return current;
	}
	else
	{ // already processed, lets find another
		if (NULL != current->child && !(current->child->nFlags&nFlag))
		{ // if there's a child
			return current->child;
		}
		else if (NULL != current->next && !(current->next->nFlags&nFlag))
		{ // if there's a sibling
			return current->next;
		}
		else if (root != current->parent)
		{ // if there's a parent we can use
			return GetNextForProcessing(current->parent, nFlag);
		}
		else // nothing left
			return NULL;
	}
}

// Stage 1: Measure all nodes
Node *RenderList::MeasureNode(Node *node, Graphics *graphics)
{
	Node *temp = GetNextForProcessing(node, RL_MEASURED);

	if (NULL == temp)
		return NULL;

	if (1 == temp->nType)
	{
		TextData *lpTD = ((TextData*)temp->lpvData);
		FontFamily family;
		//GraphicsPath path;
		PointF origin(0, 0);

		lpTD->font->GetFamily(&family);
		//path.AddString( lpTD->tcText, -1, &family, lpTD->font->GetStyle(), lpTD->font->GetSize(), origin, StringFormat::GenericTypographic() );

		temp->rf = new RectF();
		graphics->MeasureString(lpTD->tcText, -1, lpTD->font, origin, lpTD->sf, temp->rf);
		temp->rf->Width = ceilf(temp->rf->Width);

		temp->rfBound = new RectF(temp->pfPadding->X, temp->pfPadding->Y, temp->rf->Width, temp->rf->Height);

		REAL fBaselineOffset = lpTD->font->GetSize() / family.GetEmHeight(lpTD->font->GetStyle()) * family.GetCellAscent(lpTD->font->GetStyle());
		REAL fBaselineOffsetPixels = graphics->GetDpiY() / 72.0f * fBaselineOffset + 0.5f;
		REAL fMaxTextH = temp->parent->fMaxTextH;

		temp->parent->fMaxTextH = fMaxTextH > fBaselineOffsetPixels ? fMaxTextH : fBaselineOffsetPixels;
	}
	else if (2 == temp->nType)
	{
		ImageData *lpID = ((ImageData*)temp->lpvData);
		temp->rf = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
		temp->rfBound = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
	}
	else if (3 == temp->nType)
	{
		temp->rf = new RectF();
		temp->rfBound = new RectF();
	}
	else if (4 == temp->nType)
	{
		ShapeData *sd = ((ShapeData*)temp->lpvData);
		temp->rf = new RectF(0.0f, 0.0f, sd->size->Width, sd->size->Height);
		temp->rfBound = new RectF(0.0f, 0.0f, sd->size->Width, sd->size->Height);
	}

	temp->nFlags |= RL_MEASURED;

	if (root == temp)
	{
		temp->rf = new RectF();
		temp->rfBound = new RectF();
	}

	return MeasureNode(temp, graphics);
}

void RenderList::MeasurePass(Graphics *graphics)
{
	MeasureNode(root, graphics);
}

void RenderList::RenderPass(Graphics *graphics)
{
	RenderNode(root, graphics);
}

Node *RenderList::RenderNode(Node *node, Graphics *graphics)
{
	Node *temp = GetNextForProcessing(node, RL_RENDERED);

	if (NULL == temp)
		return NULL;

	if (1 == temp->nType && temp->rf->Width > 0.0f)
	{
		TextData *lpTD = ((TextData*)temp->lpvData);

		// DEBUGGING BOX
#ifdef NODE_DRAWDEBUGBOX
		Pen *pen = new Pen(Color(170, 255, 0, 0), 0.5);
		graphics->DrawRectangle(pen, *(temp->rf));
		delete pen;
#endif

		lpTD->sf->SetAlignment(lpTD->saAlign);
		lpTD->sf->SetTrimming(StringTrimmingEllipsisCharacter);
		graphics->SetSmoothingMode(SmoothingModeAntiAlias);
		graphics->SetPixelOffsetMode(PixelOffsetModeHalf);

		Color color;
		lpTD->pnOutline->GetColor(&color);

		RectF shadowRect(*(temp->rf));
		shadowRect.Offset(lpTD->fDropShadowOffset, lpTD->fDropShadowOffset);
		SolidBrush* shadowBrush = lpTD->drop;

		if (color.GetAlpha() == 0)
		{
			if (0.0f != lpTD->fDropShadowOffset)
				graphics->DrawString(lpTD->tcText, -1, lpTD->font, shadowRect, lpTD->sf, shadowBrush);
			graphics->DrawString(lpTD->tcText, -1, lpTD->font, *(temp->rf), lpTD->sf, lpTD->sb);
		}
		else
		{
			GraphicsPath path;
			GraphicsPath shadowPath;
			FontFamily fontFamily;

			lpTD->font->GetFamily(&fontFamily);
			REAL emSize = floorf(lpTD->font->GetSize() * graphics->GetDpiY() / 72.0f);
			path.AddString(lpTD->tcText, -1, &fontFamily, lpTD->font->GetStyle(), emSize, *(temp->rf), lpTD->sf);
			if (0.0f != lpTD->fDropShadowOffset)
				shadowPath.AddString(lpTD->tcText, -1, &fontFamily, lpTD->font->GetStyle(), emSize, shadowRect, lpTD->sf);

			if (0.0f != lpTD->fDropShadowOffset)
				graphics->FillPath(shadowBrush, &shadowPath);
			graphics->FillPath(lpTD->sb, &path);
			graphics->DrawPath(lpTD->pnOutline, &path);
		}

		graphics->SetSmoothingMode(SmoothingModeNone);

	}
	else if (2 == temp->nType)
	{
		ImageData *lpID = ((ImageData*)temp->lpvData);

		graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
		graphics->DrawImage(lpID->image, *(temp->rf));
		graphics->SetInterpolationMode(InterpolationModeBilinear);

		// outline is surrounding picture
		graphics->SetSmoothingMode(SmoothingModeNone);
		lpID->pnOutline->SetAlignment(PenAlignmentInset);
		graphics->DrawRectangle(lpID->pnOutline, *(temp->rf));

		// DEBUGGING BOX
#ifdef NODE_DRAWDEBUGBOX
		Pen *pen = new Pen(Color(170, 0, 255, 0), 0.5);
		graphics->DrawRectangle(pen, *(temp->rf));
		delete pen;
#endif
	}
	else if (3 == temp->nType)
	{
		// DEBUGGING BOX
#ifdef NODE_DRAWDEBUGBOX
		Pen *pen = new Pen(Color(170, 0, 0, 255), 0.5);
		graphics->DrawRectangle(pen, *(temp->rfBound));
		delete pen;
#endif
	}
	else if (4 == temp->nType)
	{
		ShapeData *sd = ((ShapeData*)temp->lpvData);
		RectF *rf = temp->rf;
		rf->X += 4.0f;
		rf->Y += 4.0f;
		rf->Width -= 8.0f;
		rf->Height -= 8.0f;

		GraphicsPath path;
		path.StartFigure();

		RectF *fill = new RectF(*rf);

		// Create shape in path
		if (sd->type == ShapeCircle)
		{
			graphics->SetSmoothingMode(SmoothingModeAntiAlias);
			path.AddEllipse(*(rf));
		}
		else if (sd->type == ShapeRectangle)
		{
			graphics->SetSmoothingMode(SmoothingModeNone);
			path.AddRectangle(*(rf));
		}
		else if (sd->type == ShapeHexagon)
		{
			graphics->SetSmoothingMode(SmoothingModeAntiAlias);
			// Set up some variables for convenience
			REAL xoffset = rf->X;
			REAL yoffset = rf->Y;
			REAL size = rf->Width;
			// Create hexagon
			path.AddLine(xoffset + 0.0f, yoffset + size / 2.0f, xoffset + size / 4.0f, yoffset + 0.0f);
			path.AddLine(xoffset + size / 4.0f, yoffset + 0.0f, xoffset + 3.0f*size / 4.0f, yoffset + 0.0f);
			path.AddLine(xoffset + 3.0f*size / 4.0f, yoffset + 0.0f, xoffset + size, yoffset + size / 2.0f);
			path.AddLine(xoffset + size, yoffset + size / 2.0f, xoffset + 3.0f*size / 4.0f, yoffset + size);
			path.AddLine(xoffset + 3.0f*size / 4.0f, yoffset + size, xoffset + size / 4.0f, yoffset + size);
		}
		else if (sd->type == ShapeStar)
		{
			graphics->SetSmoothingMode(SmoothingModeAntiAlias);
			// Set up some variables for convenience
			REAL size = rf->Width;
			REAL r = size / 2.0f;
			REAL xc = rf->X + r;
			REAL yc = rf->Y + r;
			REAL r1 = r / 2.0f;
			REAL a = 72.0f / 180.0f*3.14159f;
			REAL B = 36.0f / 180.0f*3.14159f;
			// Create star (http://www.codeproject.com/KB/cs/DrawUSFlag.aspx)
			path.AddLine(xc, yc - r, xc + r1*sinf(B), yc - r1*cosf(B)); //0-1
			path.AddLine(xc + r1*sinf(B), yc - r1*cosf(B), xc + r*sinf(a), yc - r*cosf(a)); //1-2
			path.AddLine(xc + r*sinf(a), yc - r*cosf(a), xc + r1*sinf(a), yc + r1*cosf(a)); //2-3
			path.AddLine(xc + r1*sinf(a), yc + r1*cosf(a), xc + r*sinf(B), yc + r*cosf(B)); //3-4
			path.AddLine(xc + r*sinf(B), yc + r*cosf(B), xc, yc + r1); //4-5
			path.AddLine(xc, yc + r1, xc - r*sinf(B), yc + r*cosf(B)); //5-6
			path.AddLine(xc - r*sinf(B), yc + r*cosf(B), xc - r1*sinf(a), yc + r1*cosf(a)); //6-7
			path.AddLine(xc - r1*sinf(a), yc + r1*cosf(a), xc - r*sinf(a), yc - r*cosf(a)); //7-8
			path.AddLine(xc - r*sinf(a), yc - r*cosf(a), xc - r1*sinf(B), yc - r1*cosf(B)); //8-9
		}
		else if (sd->type == ShapeVolume)
		{
			graphics->SetSmoothingMode(SmoothingModeNone);
			// Add outline
			path.AddRectangle(*(rf));
		}

		path.CloseFigure();

		// Actually draw shape
		switch (sd->style)
		{
		case FillNone:
			break;
		case FillAll:
			graphics->FillPath(sd->sb, &path); break;
		case FillHalfLR:
			fill->Width = fill->Width / 2.0f;
			graphics->SetClip(&path);
			graphics->FillRectangle(sd->sb, *(fill));
			graphics->ResetClip(); break;
		case FillHalfRL:
			fill->Width = fill->Width / 2.0f;
			fill->X += fill->Width;
			graphics->SetClip(&path);
			graphics->FillRectangle(sd->sb, *(fill));
			graphics->ResetClip(); break;
		case FillHalfTB:
			fill->Height = fill->Height / 2.0f;
			graphics->SetClip(&path);
			graphics->FillRectangle(sd->sb, *(fill));
			graphics->ResetClip(); break;
		case FillHalfBT:
			fill->Height = fill->Height / 2.0f;
			fill->Y += fill->Height;
			graphics->SetClip(&path);
			graphics->FillRectangle(sd->sb, *(fill));
			graphics->ResetClip(); break;
		case FillCustom:
			fill->Width *= sd->customFill / 100.0f;
			graphics->SetClip(&path);
			graphics->FillRectangle(sd->sb, *(fill));
			graphics->ResetClip(); break;
		}
		graphics->DrawPath(sd->outline, &path);

		delete fill;

		// DEBUGGING BOX
#ifdef NODE_DRAWDEBUGBOX
		Pen *pen = new Pen(Color(170, 0, 255, 0), 0.5);
		graphics->DrawRectangle(pen, *(temp->rf));
		delete pen;
#endif
	}

	temp->nFlags |= RL_RENDERED;

	return RenderNode(temp, graphics);
}

Node *RenderList::PositionNode(Node *node, Graphics *graphics)
{
	Node *current = GetNextForProcessing(node, RL_POSITIONED);

	if (1 == current->nType || 2 == current->nType || 3 == current->nType || 4 == current->nType)
	{
		if (current->nFlags & RL_LAYOUTVERT)
		{ // vertical layout
			current->rf->Offset(floorf(current->parent->rf->GetLeft() + current->pfPadding->X),
				floorf(current->parent->rfBound->GetBottom() + current->pfPadding->Y));
		}
		else
		{ // horizontal layout
			current->rf->Offset(floorf(current->parent->rfBound->GetRight() + current->pfPadding->X),
				floorf(current->parent->rf->GetTop() + current->pfPadding->Y));
		}

		if (1 == current->nType)
		{
			TextData *lpTD = ((TextData*)current->lpvData);
			FontFamily fFamily;
			lpTD->font->GetFamily(&fFamily);

			REAL fBaselineOffset = lpTD->font->GetSize() / fFamily.GetEmHeight(lpTD->font->GetStyle()) * fFamily.GetCellAscent(lpTD->font->GetStyle());
			REAL fBaselineOffsetPixels = graphics->GetDpiY() / 72.0f * fBaselineOffset;
			current->rf->Y = current->rf->Y + current->parent->fMaxTextH - (fBaselineOffsetPixels + 0.5f);
		}
		/*else if ( 3 == current->nType )
		{
			// Nothing required
		}*/
	}

	PropagateBounds(current, current->rf);

	current->nFlags |= RL_POSITIONED;

	current = GetNextForProcessing(current, RL_POSITIONED);
	if (NULL == current)
		return NULL;
	else
		return PositionNode(current, graphics);
}

Node *RenderList::PropagateBounds(Node *parent, RectF *rectBounds)
{
	Node *current = parent;

	if (NULL == current)
		return NULL;
	else if (root == current)
	{
		RectF::Union(*(current->rfBound), *(rectBounds), *(current->rfBound));

		return NULL;
	}
	else
	{
		RectF::Union(*(current->rfBound), *(rectBounds), *(current->rfBound));

		return PropagateBounds(current->parent, current->rfBound);
	}
}

void RenderList::PositionPass(Graphics *graphics)
{
	PositionNode(root, graphics);
	root->rfBound->Height += root->child->pfPadding->Y + 1.0f;
	root->rfBound->Width += root->child->pfPadding->X + 1.0f;
}

Node *RenderList::GetBottomMost(Node *from)
{
	Node *current = from;
	Node *temp = NULL;

	do
	{
		temp = GetLastSibling(current);
		current = temp->child;
	} while (NULL != current);

	return temp;
}

Node *RenderList::AddChild(Node *parent)
{
	Node *newChild = new Node;
	Node *child = parent->child;

	if (NULL == child)
		parent->child = newChild;
	else
	{
		child = GetLastSibling(parent->child);
		child->next = newChild;
		newChild->prev = child;
	}

	newChild->parent = parent;

	return newChild;
}

Node *RenderList::GetLastSibling(Node *sibling)
{
	if (NULL != sibling->next)
		return this->GetLastSibling(sibling->next);
	else
		return sibling;
}

Node *RenderList::AddSibling(Node *fromNode)
{
	Node *newSibling = new Node;
	Node *sibling = fromNode->next;

	if (NULL == sibling)
	{
		fromNode->next = newSibling;
		newSibling->prev = fromNode;
	}
	else
	{
		sibling = GetLastSibling(fromNode);
		sibling->next = newSibling;
		newSibling->prev = sibling;
	}

	newSibling->parent = fromNode->parent;

	return newSibling;
}
