
#pragma once

#include <string>
using std::string;

#include "vec.h"

void SetLbnRtf(vec3& Ilbn, vec3& Irtf);
void SetLightL(vec4& l_params);
void SetColorParam(vec3& colors, vec4& k_params);
void SetRGBLa(vec4& rgbl);
void SetToonParams(int number, float thick);
// ------------------------
//    Class CInputDialog
// ------------------------

class CInputDialog : public CDialog
{
    DECLARE_DYNAMIC(CInputDialog)

public:
    CInputDialog(CString title = "Input Dialog");
    virtual ~CInputDialog();

    virtual BOOL OnInitDialog();

    enum { IDD = IDD_INPUTDIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

private:
    CString mTitle;
};

// ----------------------
//    Class CCmdDialog
// ----------------------

class CCmdDialog : public CInputDialog
{
public:
    CCmdDialog(CString title = "Input Dialog");
    virtual ~CCmdDialog();

    string GetCmd();

protected:
    CString mCmd;
    CEdit mCmdEdit;

    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate (LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

// ----------------------
//    Class CXyzDialog
// ----------------------

class CXyzDialog : public CInputDialog
{
public:
    CXyzDialog(CString title = "Input Dialog");
    virtual ~CXyzDialog();

    vec3 GetXYZ();

protected:
    float mX;
    float mY;
    float mZ;
    CEdit mXEdit;
    CEdit mYEdit;
    CEdit mZEdit;

    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate (LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

// -------------------------
//    Class CCmdXyzDialog
// -------------------------

class CCmdXyzDialog : public CInputDialog
{
public:
    CCmdXyzDialog(CString title = "Input Dialog");
    virtual ~CCmdXyzDialog();

    string GetCmd();
    vec3 GetXYZ();

protected:
    CString mCmd;
    float mX;
    float mY;
    float mZ;
    CEdit mCmdEdit;
    CEdit mXEdit;
    CEdit mYEdit;
    CEdit mZEdit;

    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate (LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};




class CRltbnfDialog : public CInputDialog
{
public:
    CRltbnfDialog(CString title = "Input Dialog");
    virtual ~CRltbnfDialog();

    vec3 GetRTF();  //Right Top Far
    vec3 GetLBN();  //Left Bottom Near
protected:
    float mRight;
    float mLeft;
    float mTop;
    float mBottom;
    float mFar;
    float mNear;
    CEdit mRightEdit;
    CEdit mLeftEdit;
    CEdit mTopEdit;
    CEdit mBottomEdit;
    CEdit mFarEdit;
    CEdit mNearEdit;


    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};


class CPerspDialog : public CInputDialog
{
public:
    CPerspDialog(CString title = "Input Dialog");
    virtual ~CPerspDialog();

    vec4 GetParams();  //fovy aspect near Far

protected:
    float mFovy;
    float mAspect;
    float mFar;
    float mNear;
    CEdit mFovyEdit;
    CEdit mAspectEdit;
    CEdit mFarEdit;
    CEdit mNearEdit;


    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};


class ColorDialog : public CInputDialog
{
public:
    ColorDialog(CString title = "Input Dialog");
    virtual ~ColorDialog();

    vec4 GetK();  //Ka Kd Ks Ke
    vec3 GetRGB();  //R G B

protected:
    float mKa;// mRight;
    float mKd;// mLeft;
    float mKs;// mTop;
    float mKe;// mTop;
    float mRed;// mBottom;
    float mGreen;// mFar;
    float mBlue;// mNear;
    CEdit mKaEdit;
    CEdit mKdEdit;
    CEdit mKsEdit;
    CEdit mKeEdit;
    CEdit mRedEdit;
    CEdit mGreenEdit;
    CEdit mBlueEdit;


    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};



// ----------------------
//    Class LDialog
// ----------------------

class LDialog : public CInputDialog
{
public:
    LDialog(CString title = "Input Dialog");
    virtual ~LDialog();

    vec4 GetL();

protected:
    float mLa;
    float mLd;
    float mLs;
    int mAlpha;
    CEdit mLaEdit;
    CEdit mLdEdit;
    CEdit mLsEdit;
    CEdit mAlphaEdit;

    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};



class RgbDialog : public CInputDialog
{
public:
    RgbDialog(CString title = "Input Dialog");
    virtual ~RgbDialog();

    vec4 GetRGBLa();  //R G B La

protected:
    float mRed;
    float mGreen;
    float mBlue;
    float mLa;
    CEdit mRedEdit;
    CEdit mGreenEdit;
    CEdit mBlueEdit;
    CEdit mLaEdit;


    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

class ToonDialog : public CInputDialog
{
public:
    ToonDialog(CString title = "Input Dialog");
    virtual ~ToonDialog();

    int GetNum();  //mColorNum, mLineThick
    float GetTick();  //mColorNum, mLineThick

protected:
    float mColorNum;
    float mLineThick;
    CEdit mColorNumEdit;
    CEdit mLineThickEdit;



    virtual void DoDataExchange(CDataExchange* pDX);

    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};