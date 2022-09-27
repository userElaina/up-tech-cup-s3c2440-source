#include "figure.h"

#if USE_MINIGUI==0

void CopyRect(structRECT* prect1, structRECT* prect2)
{
	prect1->left=prect2->left;
	prect1->top=prect2->top;
	prect1->right=prect2->right;
	prect1->bottom=prect2->bottom;
}

void SetRect(structRECT* prect, int left, int top, int right, int bottom)
{
	prect->left=left;
	prect->top=top;
	prect->right=right;
	prect->bottom=bottom;
}

void InflateRect(structRECT* prect, int cx,int cy)
{
	prect->left-=cx;
	prect->top-=cy;
	prect->right+=cx;
	prect->bottom+=cy;
}

U8 IsInRect(structRECT *prect, int x, int y)
{
	if(x>=prect->left && x<=prect->right && y>=prect->top && y<=prect->bottom)
		return TRUE;

	return FALSE;
}

U8 IsInRect2(structRECT *prect, structPOINT*ppt)
{
	return IsInRect(prect, ppt->x, ppt->y);
}

int GetRectWidth(structRECT* prect)
{
	return prect->right-prect->left;
}

int GetRectHeight(structRECT* prect)
{
	return prect->bottom-prect->top;
}

void RectOffSet(structRECT* prect, int x,int y)
{
	prect->left+=x;
	prect->right+=x;
	prect->top+=y;
	prect->bottom+=y;
}

#endif //#ifdef USE_MINIGUI==0
