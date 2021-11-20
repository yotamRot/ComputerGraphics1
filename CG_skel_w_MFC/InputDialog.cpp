
#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "InputDialog.h"
#include <math.h>

#define IDC_CMD_EDIT 200
#define IDC_X_EDIT 201
#define IDC_Y_EDIT 202
#define IDC_Z_EDIT 203


#define IDC_R_EDIT 204
#define IDC_L_EDIT 205
#define IDC_T_EDIT 206
#define IDC_B_EDIT 207
#define IDC_F_EDIT 208
#define IDC_N_EDIT 209
#define IDC_FOVY_EDIT 210
#define IDC_ASPECT_EDIT 211



#define CMD_EDIT_TITLE "Command"
#define X_EDIT_TITLE "X ="
#define Y_EDIT_TITLE "Y ="
#define Z_EDIT_TITLE "Z ="


#define R_EDIT_TITLE "Right ="
#define T_EDIT_TITLE "Top ="
#define F_EDIT_TITLE "Far ="
#define L_EDIT_TITLE "Left ="
#define B_EDIT_TITLE "Bottom ="
#define N_EDIT_TITLE "Near ="
#define FOVY_EDIT_TITLE "Fovy ="
#define ASPECT_EDIT_TITLE "Aspect ="


vec3 rtf;
vec3 lbn;

void setLbnRtf(vec3 Ilbn, vec3 Irtf)
{
    Ilbn = Ilbn * 100;
    Irtf = Irtf * 100;

    rtf.x = float(round(Irtf.x) / 100);
    rtf.y = float(round(Irtf.y) / 100);
    rtf.z = float(round(Irtf.z) / 100);
    lbn.x = float(round(Ilbn.x) / 100);
    lbn.y = float(round(Ilbn.y) / 100);
    lbn.z = float(round(Ilbn.z) / 100);
}

// ------------------------
//    Class CInputDialog
// ------------------------

IMPLEMENT_DYNAMIC(CInputDialog, CDialog)

CInputDialog::CInputDialog(CString title)
	: CDialog(CInputDialog::IDD, NULL), mTitle(title)
{ }

CInputDialog::~CInputDialog()
{ }

BOOL CInputDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetWindowText(mTitle);

    return TRUE;
}

void CInputDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

// ----------------------
//    Class CCmdDialog
// ----------------------

CCmdDialog::CCmdDialog(CString title)
    : CInputDialog(title), mCmd("")
{ }

CCmdDialog::~CCmdDialog()
{ }

string CCmdDialog::GetCmd()
{
    return ((LPCTSTR)mCmd);
}

void CCmdDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_CMD_EDIT, mCmd);
}

// CCmdDialog message handlers
BEGIN_MESSAGE_MAP(CCmdDialog, CInputDialog)
    ON_WM_CREATE ()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CCmdDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mCmdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(10, 30, 450, 100), this, IDC_CMD_EDIT);

    return 0;
}

void CCmdDialog::OnPaint()
{   
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect cmd_rect(10, 10, 450, 30);
    dc.DrawText(CString(CMD_EDIT_TITLE), -1, &cmd_rect, DT_SINGLELINE);

    mCmdEdit.SetFocus();
}

// ----------------------
//    Class CXyzDialog
// ----------------------

CXyzDialog::CXyzDialog(CString title)
    : CInputDialog(title), mX(0.0), mY(0.0), mZ(0.0)
{ }

CXyzDialog::~CXyzDialog()
{ }

vec3 CXyzDialog::GetXYZ()
{
    return vec3(mX, mY, mZ);
}

void CXyzDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_X_EDIT, mX);
    DDX_Text(pDX, IDC_Y_EDIT, mY);
    DDX_Text(pDX, IDC_Z_EDIT, mZ);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(CXyzDialog, CInputDialog)
    ON_WM_CREATE ()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CXyzDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mXEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(130, 70, 340, 90), this, IDC_X_EDIT);

    mYEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(130, 140, 340, 160), this, IDC_Y_EDIT);

    mZEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(130, 210, 340, 230), this, IDC_Z_EDIT);

    return 0;
}

void CXyzDialog::OnPaint()
{   
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect x_rect(100, 72, 450, 90);
    dc.DrawText(CString(X_EDIT_TITLE), -1, &x_rect, DT_SINGLELINE);

    CRect y_rect(100, 142, 450, 160);
    dc.DrawText(CString(Y_EDIT_TITLE), -1, &y_rect, DT_SINGLELINE);

    CRect z_rect(100, 212, 450, 230);
    dc.DrawText(CString(Z_EDIT_TITLE), -1, &z_rect, DT_SINGLELINE);

    mXEdit.SetFocus();
}

// -------------------------
//    Class CCmdXyzDialog
// -------------------------

CCmdXyzDialog::CCmdXyzDialog(CString title)
    :  CInputDialog(title), mCmd(""), mX(0.0), mY(0.0), mZ(0.0)
{ }

CCmdXyzDialog::~CCmdXyzDialog()
{ }

string CCmdXyzDialog::GetCmd()
{
    return ((LPCTSTR)mCmd);
}

vec3 CCmdXyzDialog::GetXYZ()
{
    return vec3(mX, mY, mZ);
}

void CCmdXyzDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_CMD_EDIT, mCmd);
    DDX_Text(pDX, IDC_X_EDIT, mX);
    DDX_Text(pDX, IDC_Y_EDIT, mY);
    DDX_Text(pDX, IDC_Z_EDIT, mZ);
}

// CCmdXyzDialog message handlers
BEGIN_MESSAGE_MAP(CCmdXyzDialog, CInputDialog)
    ON_WM_CREATE ()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CCmdXyzDialog::OnCreate(LPCREATESTRUCT lpcs)
{    
    mCmdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(10, 30, 450, 100), this, IDC_CMD_EDIT);

    mXEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(40, 135, 250, 155), this, IDC_X_EDIT);

    mYEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(40, 190, 250, 210), this, IDC_Y_EDIT);

    mZEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
      CRect(40, 245, 250, 265), this, IDC_Z_EDIT);

    return 0;
}

void CCmdXyzDialog::OnPaint()
{   
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect cmd_rect(10, 10, 450, 30);
    dc.DrawText(CString(CMD_EDIT_TITLE), -1, &cmd_rect, DT_SINGLELINE);

    CRect x_rect(10, 137, 450, 155);
    dc.DrawText(CString(X_EDIT_TITLE), -1, &x_rect, DT_SINGLELINE);

    CRect y_rect(10, 192, 450, 210);
    dc.DrawText(CString(Y_EDIT_TITLE), -1, &y_rect, DT_SINGLELINE);

    CRect z_rect(10, 247, 450, 265);
    dc.DrawText(CString(Z_EDIT_TITLE), -1, &z_rect, DT_SINGLELINE);

    mCmdEdit.SetFocus();
}



// ----------------------
//    Class CRltbnfDialog
// ----------------------

CRltbnfDialog::CRltbnfDialog(CString title )
    : CInputDialog(title), mRight(rtf.x), mLeft(lbn.x), mTop(rtf.y), mBottom(lbn.y), mFar(rtf.z), mNear(lbn.z)
{ }

CRltbnfDialog::~CRltbnfDialog()
{ }

vec3 CRltbnfDialog::GetRTF()
{
    return vec3(mRight, mTop, mFar);
}

vec3 CRltbnfDialog::GetLBN()
{
    return vec3(mLeft, mBottom, mNear);
}

void CRltbnfDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_R_EDIT, mRight);
    DDX_Text(pDX, IDC_L_EDIT, mLeft);
    DDX_Text(pDX, IDC_T_EDIT, mTop);
    DDX_Text(pDX, IDC_B_EDIT, mBottom);
    DDX_Text(pDX, IDC_F_EDIT, mFar);
    DDX_Text(pDX, IDC_N_EDIT, mNear);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(CRltbnfDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CRltbnfDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mRightEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 70, 200, 90), this, IDC_R_EDIT);

    mLeftEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(360, 70, 400, 90), this, IDC_T_EDIT);

    mTopEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(560, 70, 600, 90), this, IDC_F_EDIT);

    mBottomEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 170, 200, 190), this, IDC_L_EDIT);

    mFarEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(360, 170, 400, 190), this, IDC_B_EDIT);

    mNearEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(560, 170, 600, 190), this, IDC_N_EDIT);

    return 0;
}

void CRltbnfDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 200, 95);
    dc.DrawText(CString(R_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect t_rect(250, 72, 400, 95);
    dc.DrawText(CString(T_EDIT_TITLE), -1, &t_rect, DT_SINGLELINE);

    CRect f_rect(500, 72, 600, 95);
    dc.DrawText(CString(F_EDIT_TITLE), -1, &f_rect, DT_SINGLELINE);

    CRect l_rect(100, 172, 200, 195);
    dc.DrawText(CString(L_EDIT_TITLE), -1, &l_rect, DT_SINGLELINE);

    CRect b_rect(250, 172, 400, 195);
    dc.DrawText(CString(B_EDIT_TITLE), -1, &b_rect, DT_SINGLELINE);

    CRect n_rect(500, 172, 600, 195);
    dc.DrawText(CString(N_EDIT_TITLE), -1, &n_rect, DT_SINGLELINE);

    mRightEdit.SetFocus();
}


// ----------------------
//    Class CPerspDialog
// ----------------------

CPerspDialog::CPerspDialog(CString title)
    : CInputDialog(title), mFovy(0.0), mAspect(0.0), mFar(0.0), mNear(0.0)
{ }

CPerspDialog::~CPerspDialog()
{ }

vec4 CPerspDialog::GetParams()
{
    return vec4(mFovy, mAspect, mNear, mFar);
}

void CPerspDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FOVY_EDIT, mFovy);
    DDX_Text(pDX, IDC_ASPECT_EDIT, mAspect);
    DDX_Text(pDX, IDC_F_EDIT, mFar);
    DDX_Text(pDX, IDC_N_EDIT, mNear);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(CPerspDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CPerspDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mFovyEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 70, 200, 90), this, IDC_FOVY_EDIT);

    mNearEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(360, 70, 400, 90), this, IDC_N_EDIT);


    mAspectEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 170, 200, 190), this, IDC_ASPECT_EDIT);

    mFarEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(360, 170, 400, 190), this, IDC_F_EDIT);


    return 0;
}

void CPerspDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 200, 95);
    dc.DrawText(CString(FOVY_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect t_rect(250, 72, 400, 95);
    dc.DrawText(CString(N_EDIT_TITLE), -1, &t_rect, DT_SINGLELINE);


    CRect l_rect(100, 172, 200, 195);
    dc.DrawText(CString(ASPECT_EDIT_TITLE), -1, &l_rect, DT_SINGLELINE);

    CRect b_rect(250, 172, 400, 195);
    dc.DrawText(CString(F_EDIT_TITLE), -1, &b_rect, DT_SINGLELINE);


    mFovyEdit.SetFocus();
}