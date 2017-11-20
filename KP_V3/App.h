#include <afxwin.h>

class  CApp :public CWinApp
{
public:
	BOOL InitInstance(void);

};
class  CMain :public CFrameWnd
{
	CPalette *m_pPal;
	HGLRC m_hrc;
public:
	CMain(void);
	void GLInit();
	void OnVScroll(UINT SBCode, UINT Pos, CScrollBar *SB);
	void OnHScroll(UINT SBCode, UINT Pos, CScrollBar *SB);
	BOOL OnMouseWheel(UINT nFlag, short zDelta, CPoint pt);
	void OnSize(UINT nType, int cx, int cy);
	void OnPaint();
	void makeHand1stHalf();
	void makeHand2ndHalf();
	void takeSphere();
	//void MakeCheckImage();
	BOOL CreateRGBPalette(HDC hDC);
	void OnOpenGLFirst();
	DECLARE_MESSAGE_MAP()


	void OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlag);
};