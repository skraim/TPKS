#define CDS_FULLSCREEN 4
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#include <afxwin.h>
#include <afxext.h>
#include <memory.h>
#pragma comment(lib, "legacy_stdio_definitions.lib")
#include <math.h> 
#include "gl/gl.h" 
#include "gl/glu.h" 
#include "App.h"
#include "gl/GLAUX.H"

struct {
	int baseR = 330;
	int kneeM = 65;
	int shoulderM = 60;
	int wristR = 170;
	int kleshnyaL = -2;
	int kleshnyaR = -2;
}dosmth;
double zoom = -6;
int vspos = 50, hspos = 50;
AUX_RGBImageRec *pImage;
AUX_RGBImageRec *pImage2;
AUX_RGBImageRec *pImage3; 
GLUquadricObj* m_qObj;
CMain::CMain()
{
	
	pImage = auxDIBImageLoad("white.bmp");
	pImage2 = auxDIBImageLoad("black.bmp");
	pImage3 = auxDIBImageLoad("eye.bmp");
	Create(NULL, "MR-999e", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, rectDefault, NULL, NULL);
	SetScrollPos(SB_HORZ, 50);
	SetScrollPos(SB_VERT, 50);
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	CClientDC dc(this);
	int nPixelFormat = ChoosePixelFormat(dc.m_hDC, &pfd);
	BOOL bResult = SetPixelFormat(dc.m_hDC, nPixelFormat, &pfd);
	m_hrc = wglCreateContext(dc.m_hDC);
	wglMakeCurrent(dc.m_hDC, m_hrc);
	CreateRGBPalette(dc.m_hDC);
	m_pPal = NULL;
}

BOOL CApp::InitInstance()
{
	m_pMainWnd = new CMain;
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

CApp App;

BEGIN_MESSAGE_MAP(CMain, CFrameWnd)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BOOL CMain::CreateRGBPalette(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int n = GetPixelFormat(hDC);
	DescribePixelFormat(hDC, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (!(pfd.dwFlags & PFD_NEED_PALETTE)) return FALSE;
	LOGPALETTE* pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE)
		+ 256 * sizeof(PALETTEENTRY));
	if (!pPal) { TRACE("Out of memory for logpal"); return FALSE; }
	pPal->palNumEntries = 256;
	BOOL bResult = m_pPal->CreatePalette(pPal);
	free(pPal);
	return bResult;
}


void CMain::OnPaint()
{
	CPaintDC pDC(this);
	CPalette* ppalOld = NULL;
	if (m_pPal) { ppalOld = pDC.SelectPalette(m_pPal, 0); pDC.RealizePalette(); }
	BOOL bResult = wglMakeCurrent(pDC.m_hDC, m_hrc);
	GLInit();
	OnOpenGLFirst();
	SwapBuffers(pDC.m_hDC);
	if (ppalOld) pDC.SelectPalette(ppalOld, 0);
	wglMakeCurrent(NULL, NULL);
}

void CMain::OnSize(UINT nType, int cx, int cy)
{
	CClientDC dc(this);
	BOOL bResult = wglMakeCurrent(dc.m_hDC, m_hrc);
	GLdouble gldAspect = (GLdouble)cx / (GLdouble)cy;
	glMatrixMode(GL_PROJECTION);// OutputGlError("MatrixMode") ;
	glLoadIdentity();
	gluPerspective(30.0, gldAspect, 1.0, 10.0);
	glViewport(0, 0, cx, cy);
	wglMakeCurrent(NULL, NULL);
}

void CMain::OnVScroll(UINT SBCode, UINT Pos, CScrollBar *SB)
{
	switch (SBCode)
	{
	case SB_LINEDOWN: vspos++; break;
	case SB_LINEUP:  vspos--; break;
	case SB_PAGEDOWN: vspos += 5; break;
	case SB_PAGEUP:  vspos -= 5; break;
	case SB_THUMBTRACK: vspos = Pos; break;
	case SB_THUMBPOSITION: vspos = Pos; break;
	}
	Invalidate(FALSE);
	SetScrollPos(SB_VERT, vspos);
}

BOOL CMain::OnMouseWheel(UINT nFlag, short zDelta, CPoint pt) {
	CWindowDC paintDC(this);
	if (zDelta < 0) {
		zoom -= 0.5;
		if (zoom <= -8) {
			zoom = -8;
		}
	}
	else {
		zoom += 0.5;
		if (zoom > -2) {
			zoom = -2;
		}
	}
	Invalidate(FALSE);
	return TRUE;
}

void CMain::OnHScroll(UINT SBCode, UINT Pos, CScrollBar *SB)
{
	switch (SBCode)
	{
	case SB_LINERIGHT: hspos++; break;
	case SB_LINELEFT:   hspos--; break;
	case SB_PAGERIGHT: hspos += 5; break;
	case SB_PAGELEFT:  hspos -= 5; break;
	case SB_THUMBTRACK: hspos = Pos; break;
	case SB_THUMBPOSITION: hspos = Pos; break;
	}
	Invalidate(FALSE);
	SetScrollPos(SB_HORZ, hspos);
}

void CMain::GLInit()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1);
	glEnable(GL_DEPTH_TEST);
	glTranslated(0, 0, zoom);
}



void CMain::OnOpenGLFirst()
{
	m_qObj = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, pImage3->sizeX, pImage3->sizeY,
		GL_RGB, GL_UNSIGNED_BYTE, pImage3->data); 
	gluQuadricTexture(m_qObj, GL_TRUE);
	glTranslatef(0.0, -1.0, 0);
	glRotatef(360.0*hspos / 100, 0, 1, 0);
	glRotatef(360.0*vspos / 100, 1, 0, 0);


	glRotated(90, 1, 0, 0);
	glRotatef(180, 0, 0, 1);
	glPushMatrix();
	glPushMatrix();
	glTranslated(-2,0,0);
	gluSphere(m_qObj,0.2,10,10);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, pImage->sizeX, pImage->sizeY,
		GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0,0); glVertex3d(-4, -4, 7);
	glTexCoord2i(0, 6); glVertex3d(-4, 4, 7);
	glTexCoord2i(6, 0); glVertex3d(4, -4, 7);
	glTexCoord2i(6, 6); glVertex3d(4, 4, 7);
	glTexCoord2i(0, 0); glVertex3d(4, -4, -0.6);
	glTexCoord2i(0, 6); glVertex3d(4, 4, -0.6);
	glTexCoord2i(6, 0); glVertex3d(-4, -4, -0.6);
	glTexCoord2i(6, 6); glVertex3d(-4, 4, -0.6);
	glTexCoord2i(0, 0); glVertex3d(-4, -4, 7);
	glTexCoord2i(0, 6); glVertex3d(-4, 4, 7);
	glEnd();
	glBegin(GL_QUADS);
	glTexCoord2i(0, 6); glVertex3d(-4, 4, 7);
	glTexCoord2i(0, 0); glVertex3d(-4, 4, -0.6);
	glTexCoord2i(6, 0); glVertex3d(4, 4, -0.6);
	glTexCoord2i(6, 6); glVertex3d(4, 4, 7);
	glEnd();
	glTranslated(-0.3, -0.6, 0);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, pImage2->sizeX, pImage2->sizeY,
		GL_RGB, GL_UNSIGNED_BYTE, pImage2->data);
	/*Manipulator's base*/
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0); glVertex3d(0, 0, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0, 0, 0);
	glTexCoord2i(1, 0);	glVertex3d(0, 0.2, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0, 0.2, 0);
	glTexCoord2i(0, 0);	glVertex3d(-0.1, 0.3, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.1, 0.3, 0);
	glTexCoord2i(1, 0);	glVertex3d(-0.3, 0.3, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(-0.3, 0.3, 0);
	glTexCoord2i(0, 0);	glVertex3d(-0.3, 0.9, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.3, 0.9, 0);
	glTexCoord2i(1, 0);	glVertex3d(-0.3, 0.9, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(-0.3, 0.9, 0);
	glTexCoord2i(0, 0);	glVertex3d(-0.1, 0.9, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.1, 0.9, 0);
	glTexCoord2i(1, 0);	glVertex3d(0, 1, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0, 1, 0);
	glTexCoord2i(0, 0);	glVertex3d(0, 1.2, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0, 1.2, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.6, 1.2, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.6, 1.2, 0);
	glTexCoord2i(0, 0);	glVertex3d(0.6, 1, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.6, 1, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.7, 0.9, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.7, 0.9, 0);
	glTexCoord2i(0, 0);	glVertex3d(0.9, 0.9, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.9, 0.9, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.9, 0.3, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.9, 0.3, 0);
	glTexCoord2i(0, 0);	glVertex3d(0.7, 0.3, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.7, 0.3, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.6, 0.2, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.6, 0.2, 0);
	glTexCoord2i(0, 0);	glVertex3d(0.6, 0, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.6, 0, 0);
	glTexCoord2i(1, 0);	glVertex3d(0, 0, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0, 0, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0.12); glVertex3d(0.4, 1.2, -0.3);
	glTexCoord2d(0, 0.88); glVertex3d(0.9, 0.7, -0.3);
	glTexCoord2d(0.12, 1); glVertex3d(0.9, 0.5, -0.3);
	glTexCoord2d(0.88, 1); glVertex3d(0.4, 0, -0.3);
	glTexCoord2d(1, 0.88); glVertex3d(0.2, 0, -0.3);
	glTexCoord2d(1, 0.12); glVertex3d(-0.3, 0.5, -0.3);
	glTexCoord2d(0.88, 0); glVertex3d(-0.3, 0.7, -0.3);
	glTexCoord2d(0.12, 0); glVertex3d(0.2, 1.2, -0.3);
	glEnd();

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0.12); glVertex3d(0.4, 1.2, 0);
	glTexCoord2d(0, 0.88); glVertex3d(0.9, 0.7, 0);
	glTexCoord2d(0.12, 1); glVertex3d(0.9, 0.5, 0);
	glTexCoord2d(0.88, 1); glVertex3d(0.4, 0, 0);
	glTexCoord2d(1, 0.88); glVertex3d(0.2, 0, 0);
	glTexCoord2d(1, 0.12); glVertex3d(-0.3, 0.5, 0);
	glTexCoord2d(0.88, 0); glVertex3d(-0.3, 0.7, 0);
	glTexCoord2d(0.12, 0); glVertex3d(0.2, 1.2, 0);
	glEnd();
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3d(0, 0, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(0, 1.2, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.6, 1.2, -0.3);
	glTexCoord2i(1, 0);	glVertex3d(0.6, 0, -0.3);
	glTexCoord2i(0, 0);	glVertex3d(-0.3, 0.3, -0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.3, 0.9, -0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.9, 0.9, -0.3);
	glTexCoord2i(1, 0);	glVertex3d(0.9, 0.3, -0.3);
	glTexCoord2i(0, 0);	glVertex3d(0, 0, 0);
	glTexCoord2i(0, 1);	glVertex3d(0, 1.2, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.6, 1.2, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.6, 0, 0);
	glTexCoord2i(0, 0);	glVertex3d(-0.3, 0.3, 0);
	glTexCoord2i(0, 1);	glVertex3d(-0.3, 0.9, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.9, 0.9, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.9, 0.3, 0);
	glEnd();
	glPopMatrix();
	gluCylinder(m_qObj, 0.49, 0.39, 0.05, 50, 10);
	glTranslated(0, 0, 0.05);
	gluDisk(m_qObj, 0.05, 0.39, 50, 4);
	glRotated(dosmth.baseR, 0, 0, 1);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(-0.33, -0.21, 0);
	glTexCoord2i(0, 1);	glVertex3d(-0.33, -0.18, 0.3);
	glTexCoord2i(1, 0);	glVertex3d(-0.33, 0.21, 0);
	glTexCoord2i(1, 1);	glVertex3d(-0.33, 0.18, 0.3);
	glTexCoord2i(0, 0);	glVertex3d(0.33, 0.21, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.33, 0.18, 0.3);
	glTexCoord2i(1, 0);	glVertex3d(0.33, -0.21, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.33, -0.18, 0.3);
	glTexCoord2i(0, 0);	glVertex3d(-0.33, -0.21, 0);
	glTexCoord2i(0, 1);	glVertex3d(-0.33, -0.18, 0.3);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(-0.33, -0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.33, -0.16, 0.5);
	glTexCoord2i(1, 0);	glVertex3d(-0.33, 0.18, 0.3);
	glTexCoord2i(1, 1);	glVertex3d(-0.33, 0.16, 0.5);
	glTexCoord2i(0, 0);	glVertex3d(-0.17, 0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.17, 0.16, 0.5);
	glTexCoord2i(1, 0);	glVertex3d(-0.17, -0.18, 0.3);
	glTexCoord2i(1, 1);	glVertex3d(-0.17, -0.16, 0.5);
	glTexCoord2i(0, 0);	glVertex3d(-0.33, -0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(-0.33, -0.16, 0.5);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(0.33, -0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.33, -0.16, 0.5);
	glTexCoord2i(1, 0);	glVertex3d(0.33, 0.18, 0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.33, 0.16, 0.5);
	glTexCoord2i(0, 0);	glVertex3d(0.17, 0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.17, 0.16, 0.5);
	glTexCoord2i(1, 0);	glVertex3d(0.17, -0.18, 0.3);
	glTexCoord2i(1, 1);	glVertex3d(0.17, -0.16, 0.5);
	glTexCoord2i(0, 0);	glVertex3d(0.33, -0.18, 0.3);
	glTexCoord2i(0, 1);	glVertex3d(0.33, -0.16, 0.5);
	glEnd();

	glRotated(90, 0, 1, 0);
	glTranslated(-0.5, 0, -0.33);
	glRotated(dosmth.kneeM, 0, 0, 1);

	gluCylinder(m_qObj, 0.16, 0.16, 0.66, 50, 10);
	gluDisk(m_qObj, 0, 0.16, 50, 4);
	glTranslated(0, 0, 0.67);
	gluDisk(m_qObj, 0, 0.16, 50, 4);
	glTranslated(0, 0, -0.33);
	glRotated(90, 0, -1, 0);
	gluCylinder(m_qObj, 0.16, 0.16, 0.74, 50, 10);


	glTranslated(-0.13, 0, 0.75);
	glRotated(90, 0, 1, 0);
	glRotated(dosmth.shoulderM, 0, 0, 1);

	gluCylinder(m_qObj, 0.13, 0.13, 0.25, 50, 10);
	gluDisk(m_qObj, 0, 0.13, 50, 4);
	glTranslated(0, 0, 0.25);
	gluDisk(m_qObj, 0, 0.13, 50, 4);
	glTranslated(-0.05, 0, -0.125);
	glRotated(90, 0, -1, 0);
	gluCylinder(m_qObj, 0.15, 0.15, 0.70, 50, 10);
	gluDisk(m_qObj, 0, 0.15, 50, 4);
	glTranslated(0, 0, 0.70);
	gluDisk(m_qObj, 0, 0.15, 50, 4);

	glRotated(dosmth.wristR, 0, 0, 1);
	gluCylinder(m_qObj, 0.14, 0.14, 0.24, 50, 10);
	glTranslated(0, 0, 0.24);
	gluDisk(m_qObj, 0, 0.14, 50, 4);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 1);	glVertex3d(-0.19, 0.11, 0);
	glTexCoord2i(0, 0);	glVertex3d(-0.19, -0.11, 0);
	glTexCoord2i(1, 0);	glVertex3d(0.19, -0.11, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.19, 0.11, 0);
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(-0.19, 0.11, 0);
	glTexCoord2i(0, 1);	glVertex3d(-0.28, 0.11, 0.25);
	glTexCoord2i(1, 0);	glVertex3d(-0.19, -0.11, 0);
	glTexCoord2i(1, 1);	glVertex3d(-0.28, -0.11, 0.25);
	glTexCoord2i(0, 0);	glVertex3d(0.19, -0.11, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.28, -0.11, 0.25);
	glTexCoord2i(1, 0);	glVertex3d(0.19, 0.11, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.28, 0.11, 0.25);
	glTexCoord2i(0, 0);	glVertex3d(-0.19, 0.11, 0);
	glTexCoord2i(0, 1);	glVertex3d(-0.28, 0.11, 0.25);
	glEnd();

	glRotated(90, 1, 0, 0);
	glPushMatrix();
	glRotated(dosmth.kleshnyaL*1.5, 0, 0, 1);
	makeHand1stHalf();
	glRotated(dosmth.kleshnyaL * 1.35, 0, 0, 1);
	makeHand2ndHalf();

	glPopMatrix();
	glRotated(180, 0, 1, 0);
	glRotated(dosmth.kleshnyaR*1.5, 0, 0, 1);
	makeHand1stHalf();
	glRotated(dosmth.kleshnyaR * 1.35, 0, 0, 1);
	makeHand2ndHalf();
	glPopMatrix();
}

void CMain::makeHand1stHalf() {
	glTranslated(0.12, 0.12, -0.109);
	gluCylinder(m_qObj, 0.03, 0.03, 0.21, 50, 10);
	glTranslated(-0.11, 0, 0.2);
	glRotated(90, -1, 0, 0);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(0, 0, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.2, 0, 0.25);
	glTexCoord2i(1, 0);	glVertex3d(0.18, 0, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.38, 0, 0.25);
	glTexCoord2i(0, 0);	glVertex3d(0.18, 0.2, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.38, 0.2, 0.25);
	glTexCoord2i(1, 0);	glVertex3d(0, 0.2, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.2, 0.2, 0.25);
	glTexCoord2i(0, 0);	glVertex3d(0, 0, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.2, 0, 0.25);
	glEnd();
	glRotated(90, -1, 0, 0);
	glTranslated(0.23, -0.22, 0);
}

void CMain::makeHand2ndHalf() {
	gluCylinder(m_qObj, 0.03, 0.03, 0.2, 50, 10);
	glRotated(90, 1, 0, 0);
	glRotated(180, 0, 0, 1);
	glTranslated(-0.16, -0.195, 0);
	glBegin(GL_QUAD_STRIP);
	glTexCoord2i(0, 0);	glVertex3d(0.05, 0, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.3, 0, 0.35);
	glTexCoord2i(1, 0);	glVertex3d(0.2, 0, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.3, 0, 0.15);
	glTexCoord2i(0, 0);	glVertex3d(0.2, 0.19, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.3, 0.19, 0.15);
	glTexCoord2i(1, 0);	glVertex3d(0.05, 0.19, 0);
	glTexCoord2i(1, 1);	glVertex3d(0.3, 0.19, 0.35);
	glTexCoord2i(0, 0);	glVertex3d(0.05, 0, 0);
	glTexCoord2i(0, 1);	glVertex3d(0.3, 0, 0.35);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2i(0, 1);	glVertex3d(0.3, 0, 0.35);
	glTexCoord2i(0, 0);	glVertex3d(0.3, 0, 0.15);
	glTexCoord2i(1, 0);	glVertex3d(0.3, 0.19, 0.15);
	glTexCoord2i(1, 1);	glVertex3d(0.3, 0.19, 0.35);
	glEnd();
}

void CMain::OnKeyDown(UINT nChar, UINT nRepeat, UINT nFlag)
{
	switch (nChar) {
	case VK_NUMPAD4:
		if (dosmth.baseR > 0) {
			dosmth.baseR -= 3;
		}
		break;
	case VK_NUMPAD6:
		if (dosmth.baseR <= 350) {
			dosmth.baseR += 3;
		}
		break;
	case VK_NUMPAD2:
		if (dosmth.kneeM > 0) {
			dosmth.kneeM -= 3;
		}
		break;
	case VK_NUMPAD0:
		if (dosmth.kneeM + dosmth.shoulderM <= 140 && dosmth.kneeM <= 120) {
			dosmth.kneeM += 3;
		}
		break;
	case VK_NUMPAD8:
		if (dosmth.shoulderM > 0) {
			dosmth.shoulderM -= 3;
		}
		break;
	case VK_NUMPAD5:
		if (dosmth.kneeM + dosmth.shoulderM <= 140 && dosmth.shoulderM <= 135) {
			dosmth.shoulderM += 3;
		}
		break;
	case VK_NUMPAD1:
		if (dosmth.wristR > 0) {
			dosmth.wristR -= 3;
		}
		break;
	case VK_NUMPAD3:
		if (dosmth.wristR <= 340) {
			dosmth.wristR += 3;
		}
		break;
	case VK_NUMPAD7:
		if (dosmth.kleshnyaL > -10 && dosmth.kleshnyaR > -10) {
			dosmth.kleshnyaL--;
			dosmth.kleshnyaR--;
		}
		break;
	case VK_NUMPAD9:
		if (dosmth.kleshnyaL < 10 && dosmth.kleshnyaR < 10) {
			dosmth.kleshnyaL++;
			dosmth.kleshnyaR++;
		}
		break;
	case VK_SPACE: 
		/*dosmth.kleshnyaL=-10;
		dosmth.kleshnyaR = -10;
		dosmth.baseR = 270;
		dosmth.kneeM = 68;
		dosmth.shoulderM = 54;
		dosmth.wristR = 170;*/
		takeSphere();


	}
	Invalidate(false);
}

void CMain::takeSphere() {
	while (dosmth.kleshnyaL > -10) {
		dosmth.kleshnyaL--;
		dosmth.kleshnyaR--;
		Sleep(100);
	}
	if (dosmth.baseR > 270) {
		while (dosmth.baseR > 270) {
			dosmth.baseR--;
			Sleep(100);
		}
	}
	if (dosmth.baseR < 270) {
		while (dosmth.baseR < 270) {
			dosmth.baseR++;
			Sleep(100);
		}
	}
	if (dosmth.kneeM > 68) {
		while (dosmth.kneeM > 68) {
			dosmth.kneeM--;
			Sleep(100);
		}
	}
	if (dosmth.kneeM < 68) {
		while (dosmth.kneeM < 68) {
			dosmth.kneeM++;
			Sleep(100);
		}
	}
	if (dosmth.shoulderM > 54) {
		while (dosmth.shoulderM > 54) {
			dosmth.shoulderM--;
			Sleep(100);
		}
	}
	if (dosmth.shoulderM < 54) {
		while (dosmth.shoulderM < 54) {
			dosmth.shoulderM++;
			Sleep(100);
		}
	}
	if (dosmth.wristR > 170) {
		while (dosmth.wristR > 170) {
			dosmth.wristR--;
			Sleep(100);
		}
	}
	if (dosmth.wristR > 170) {
		while (dosmth.wristR > 170) {
			dosmth.wristR++;
			Sleep(100);
		}
	} 
}
