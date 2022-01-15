
#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "InputDialog.h"
#include <math.h>

#define IDC_CMD_EDIT 200
#define IDC_X_EDIT 201
#define IDC_Y_EDIT 202
#define IDC_Z_EDIT 203



#define IDC_R_EDIT                  204
#define IDC_L_EDIT                  205
#define IDC_T_EDIT                  206
#define IDC_B_EDIT                  207
#define IDC_F_EDIT                  208
#define IDC_N_EDIT                  209
#define IDC_FOVY_EDIT               210
#define IDC_ASPECT_EDIT             211


#define IDC_K_A_EDIT                212
#define IDC_K_D_EDIT                213
#define IDC_K_S_EDIT                214
#define IDC_K_E_EDIT                215
#define IDC_RED_EDIT                216
#define IDC_GREEN_EDIT              217
#define IDC_BLUE_EDIT               218

#define IDC_L_A_EDIT                219
#define IDC_L_D_EDIT                220
#define IDC_L_S_EDIT                221
#define IDC_ALPHA_EDIT              222

#define IDC_TOON_N_EDIT             223
#define IDC_TOON_T_EDIT             224
#define IDC_MOVEMENT_EDIT           225

#define CMD_EDIT_TITLE              "Command"
#define X_EDIT_TITLE                "X ="
#define Y_EDIT_TITLE                "Y ="
#define Z_EDIT_TITLE                "Z ="
#define L_A_EDIT_TITLE              "La ="
#define L_D_EDIT_TITLE              "Ld ="
#define L_S_EDIT_TITLE              "Ls ="
#define ALPHA_EDIT_TITLE            "a ="


#define R_EDIT_TITLE                "Right ="
#define T_EDIT_TITLE                "Top ="
#define F_EDIT_TITLE                "Far ="
#define L_EDIT_TITLE                "Left ="
#define B_EDIT_TITLE                "Bottom ="
#define N_EDIT_TITLE                "Near ="
#define FOVY_EDIT_TITLE             "Fovy ="
#define ASPECT_EDIT_TITLE           "Aspect ="

#define RED_EDIT_TITLE              "Red  "
#define GREEN_EDIT_TITLE            "Green"
#define BLUE_EDIT_TITLE             "Blue "
#define K_A_EDIT_TITLE              "Ka "
#define K_D_EDIT_TITLE              "Kd "
#define K_S_EDIT_TITLE              "Ks "
#define K_E_EDIT_TITLE              "Ke "
#define TOON_N_EDIT_TITLE           "Number of colors "
#define TOON_T_EDIT_TITLE           "Line Thickness "
#define MOVEMENT_TITLE              "Movement Step Size "


vec3 rtf;
vec3 lbn;
vec4 l; //la,ld,ls,alpha
vec3 RGB;
vec3 light_RGB;
vec4 ambient_rgbl;
vec4 k;
int toon_number;
float toon_tickness;
float movement_step;


void SetMovementStep(float step)
{
    movement_step = step;
}

void SetLbnRtf(vec3& Ilbn, vec3& Irtf)
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

void SetLightL(vec4& l_params)
{
    l = l_params;
}

void SetToonParams(int number,float thick)
{
    toon_number = number;
    toon_tickness = thick;
}

void SetRGBLa(vec4& rgbl)
{
    ambient_rgbl = rgbl;

}

void SetLightRGB(vec3& rgb)
{
    light_RGB = rgb;

}

void SetColorParam(vec3& colors, vec4& k_params)
{
    RGB = colors;
    k = k_params;
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



// ----------------------
//    Class CXyzDialog
// ----------------------

CMovementDialog::CMovementDialog(CString title)
    : CInputDialog(title), mMovement(movement_step)
{ }

CMovementDialog::~CMovementDialog()
{ }

float CMovementDialog::GetMovement()
{
    return mMovement;
}

void CMovementDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_MOVEMENT_EDIT, mMovement);

}

// CMovementDialog message handlers
BEGIN_MESSAGE_MAP(CMovementDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int CMovementDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mMovementEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(300, 70, 340, 90), this, IDC_MOVEMENT_EDIT);

    return 0;
}

void CMovementDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect x_rect(100, 72, 450, 90);
    dc.DrawText(CString(MOVEMENT_TITLE), -1, &x_rect, DT_SINGLELINE);

    mMovementEdit.SetFocus();
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


// ----------------------
//    Class ColorDialog
// ----------------------

ColorDialog::ColorDialog(CString title)
    : CInputDialog(title), mKa(k.x), mKd(k.y), mKs(k.z), mKe(k.w), mRed(RGB.x), mGreen(RGB.y), mBlue(RGB.z)
{ }

ColorDialog::~ColorDialog()
{ }

vec4 ColorDialog::GetK()
{
    return vec4(mKa, mKd, mKs, mKe);
}

vec3 ColorDialog::GetRGB()
{
    return vec3(mRed, mGreen, mBlue);
}

void ColorDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_K_A_EDIT, mKa);
    DDX_Text(pDX, IDC_K_D_EDIT, mKd);
    DDX_Text(pDX, IDC_K_S_EDIT, mKs);
    DDX_Text(pDX, IDC_K_E_EDIT, mKe);
    DDX_Text(pDX, IDC_RED_EDIT, mRed);
    DDX_Text(pDX, IDC_GREEN_EDIT, mGreen);
    DDX_Text(pDX, IDC_BLUE_EDIT, mBlue);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(ColorDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int ColorDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mKaEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 70, 200, 90), this, IDC_K_A_EDIT);

    mKdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(300, 70, 330, 90), this, IDC_K_D_EDIT);

    mKsEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(440, 70, 460, 90), this, IDC_K_S_EDIT);

    mKeEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(560, 70, 600, 90), this, IDC_K_E_EDIT);

    mRedEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(160, 170, 200, 190), this, IDC_RED_EDIT);

    mGreenEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(360, 170, 400, 190), this, IDC_GREEN_EDIT);

    mBlueEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(560, 170, 600, 190), this, IDC_BLUE_EDIT);

    return 0;
}

void ColorDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 200, 95);
    dc.DrawText(CString(K_A_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect t_rect(230, 72, 330, 95);
    dc.DrawText(CString(K_D_EDIT_TITLE), -1, &t_rect, DT_SINGLELINE);

    CRect f_rect(360, 72, 460, 95);
    dc.DrawText(CString(K_S_EDIT_TITLE), -1, &f_rect, DT_SINGLELINE);

    CRect e_rect(500, 72, 600, 95);
    dc.DrawText(CString(K_E_EDIT_TITLE), -1, &e_rect, DT_SINGLELINE);

    CRect l_rect(100, 172, 200, 195);
    dc.DrawText(CString(RED_EDIT_TITLE), -1, &l_rect, DT_SINGLELINE);

    CRect b_rect(250, 172, 400, 195);
    dc.DrawText(CString(GREEN_EDIT_TITLE), -1, &b_rect, DT_SINGLELINE);

    CRect n_rect(500, 172, 600, 195);
    dc.DrawText(CString(BLUE_EDIT_TITLE), -1, &n_rect, DT_SINGLELINE);

    mKaEdit.SetFocus();
}

// ----------------------
//    Class LDialog
// ----------------------

LDialog::LDialog(CString title)
    : CInputDialog(title), mLa(l.x), mLd(l.y), mLs(l.z), mAlpha(l.w)
{ }

LDialog::~LDialog()
{ }

vec4 LDialog::GetL()
{
    return vec4(mLa, mLd, mLs, mAlpha);
}

void LDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_L_A_EDIT, mLa);
    DDX_Text(pDX, IDC_L_D_EDIT, mLd);
    DDX_Text(pDX, IDC_L_S_EDIT, mLs);
    DDX_Text(pDX, IDC_ALPHA_EDIT, mAlpha);
}

// LDialog message handlers
BEGIN_MESSAGE_MAP(LDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int LDialog::OnCreate(LPCREATESTRUCT lpcs)
{
    mLaEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(130, 70, 340, 90), this, IDC_L_A_EDIT);

    mLdEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(130, 140, 340, 160), this, IDC_L_D_EDIT);

    mLsEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(130, 210, 340, 230), this, IDC_L_S_EDIT);

    mAlphaEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(130, 280, 340, 300), this, IDC_ALPHA_EDIT);

    return 0;
}

void LDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect x_rect(100, 72, 450, 90);
    dc.DrawText(CString(L_A_EDIT_TITLE), -1, &x_rect, DT_SINGLELINE);

    CRect y_rect(100, 142, 450, 160);
    dc.DrawText(CString(L_D_EDIT_TITLE), -1, &y_rect, DT_SINGLELINE);

    CRect z_rect(100, 212, 450, 230);
    dc.DrawText(CString(L_S_EDIT_TITLE), -1, &z_rect, DT_SINGLELINE);

    CRect w_rect(100, 282, 450, 300);
    dc.DrawText(CString(ALPHA_EDIT_TITLE), -1, &w_rect, DT_SINGLELINE);

    mLaEdit.SetFocus();
}


// ----------------------
//    Class RGBDialog
// ----------------------

RgbDialog::RgbDialog(CString title)
    : CInputDialog(title), mRed(ambient_rgbl.x), mGreen(ambient_rgbl.y), mBlue(ambient_rgbl.z), mLa(ambient_rgbl.w)
{ }

RgbDialog::~RgbDialog()
{ }


vec4 RgbDialog::GetRGBLa()
{
    return vec4(mRed, mGreen, mBlue, mLa);
}

void RgbDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_RED_EDIT, mRed);
    DDX_Text(pDX, IDC_GREEN_EDIT, mGreen);
    DDX_Text(pDX, IDC_BLUE_EDIT, mBlue);
    DDX_Text(pDX, IDC_L_A_EDIT, mLa);
}

// CXyzDialog message handlers
BEGIN_MESSAGE_MAP(RgbDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int RgbDialog::OnCreate(LPCREATESTRUCT lpcs)
{

    mRedEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 70, 340, 90), this, IDC_RED_EDIT);

    mGreenEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 140, 340, 160), this, IDC_GREEN_EDIT);

    mBlueEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 210, 340, 230), this, IDC_BLUE_EDIT);

    mLaEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 280, 340, 300), this, IDC_L_A_EDIT);

    return 0;
}

void RgbDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 450, 90);
    dc.DrawText(CString(RED_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect g_rect(100, 142, 450, 160);
    dc.DrawText(CString(GREEN_EDIT_TITLE), -1, &g_rect, DT_SINGLELINE);

    CRect b_rect(100, 212, 450, 230);
    dc.DrawText(CString(BLUE_EDIT_TITLE), -1, &b_rect, DT_SINGLELINE);
    
    CRect l_rect(100, 282, 450, 300);
    dc.DrawText(CString(L_A_EDIT_TITLE), -1, &l_rect, DT_SINGLELINE);

    mRedEdit.SetFocus();
}


// ----------------------
//    Class LightRGBDialog
// ----------------------

LightRGBDialog::LightRGBDialog(CString title)
    : CInputDialog(title), mRed(light_RGB.x), mGreen(light_RGB.y), mBlue(light_RGB.z)
{ }

LightRGBDialog::~LightRGBDialog()
{ }


vec3 LightRGBDialog::GetRGB()
{
    return vec3(mRed, mGreen, mBlue);
}

void LightRGBDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_RED_EDIT, mRed);
    DDX_Text(pDX, IDC_GREEN_EDIT, mGreen);
    DDX_Text(pDX, IDC_BLUE_EDIT, mBlue);
}

// LightRGBDialog message handlers
BEGIN_MESSAGE_MAP(LightRGBDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int LightRGBDialog::OnCreate(LPCREATESTRUCT lpcs)
{

    mRedEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 70, 340, 90), this, IDC_RED_EDIT);

    mGreenEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 140, 340, 160), this, IDC_GREEN_EDIT);

    mBlueEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(150, 210, 340, 230), this, IDC_BLUE_EDIT);

    return 0;
}

void LightRGBDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 450, 90);
    dc.DrawText(CString(RED_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect g_rect(100, 142, 450, 160);
    dc.DrawText(CString(GREEN_EDIT_TITLE), -1, &g_rect, DT_SINGLELINE);

    CRect b_rect(100, 212, 450, 230);
    dc.DrawText(CString(BLUE_EDIT_TITLE), -1, &b_rect, DT_SINGLELINE);

    mRedEdit.SetFocus();
}


// ----------------------
//    Class ToonDialog
// ----------------------

ToonDialog::ToonDialog(CString title)
    : CInputDialog(title), mColorNum(toon_number), mLineThick(toon_tickness)
{ }

ToonDialog::~ToonDialog()
{ }


int ToonDialog::GetNum()
{
    return mColorNum;
}

float ToonDialog::GetTick()
{
    return mLineThick;
}

int GetNum();  //mColorNum, mLineThick
float GetTick();  //mColorNum, mLineThick

void ToonDialog::DoDataExchange(CDataExchange* pDX)
{
    CInputDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_TOON_N_EDIT, mColorNum);
    DDX_Text(pDX, IDC_TOON_T_EDIT, mLineThick);
}

// ToonDialog message handlers
BEGIN_MESSAGE_MAP(ToonDialog, CInputDialog)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

int ToonDialog::OnCreate(LPCREATESTRUCT lpcs)
{

    mColorNumEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(250, 70, 340, 90), this, IDC_TOON_N_EDIT);

    mLineThickEdit.Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        CRect(250, 140, 340, 160), this, IDC_TOON_T_EDIT);

    return 0;
}

void ToonDialog::OnPaint()
{
    CPaintDC dc(this);
    dc.SetBkMode(TRANSPARENT);

    CRect r_rect(100, 72, 450, 90);
    dc.DrawText(CString(TOON_N_EDIT_TITLE), -1, &r_rect, DT_SINGLELINE);

    CRect g_rect(100, 142, 450, 160);
    dc.DrawText(CString(TOON_T_EDIT_TITLE), -1, &g_rect, DT_SINGLELINE);


    mColorNumEdit.SetFocus();
}
