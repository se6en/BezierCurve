#pragma once

using namespace Microsoft::WRL;

class CSpeedCurveCtrl : public CWnd
{
   DECLARE_DYNAMIC(CSpeedCurveCtrl)

public:
   CSpeedCurveCtrl();
   ~CSpeedCurveCtrl();

   void ResetSpeedData();
   void AddSpeedDataItem();

   void SetPlayTime(double dTime);

   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   afx_msg void OnPaint();
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);

   DECLARE_MESSAGE_MAP()

private:
   BOOL PreCreateWindow(CREATESTRUCT& cs);
   BOOL PreTranslateMessage(MSG* pMsg);

   void CreateDeviceIndependentResources();
   void CreateDeviceResources();

   void ResizeDirect2DResources(int nWidth, int nHeight);
   void ReleaseDirect2DResources();

   void DrawDurationArea();

   void DrawVerticalSeparator();
   void DrawSpeedCurve();
   void DrawSpeedCurveLines();
   void DrawSpeedCurvePoint();
   void DrawSpeedCurveSelectPoint();

   void DrawCurrentTimeLine();

   void DrawElementAreaBackground();
   void DrawElementCurvePoint();
   void DrawElementTimeLine();

   void DrawElementArea();

   float CalculateTimePercent(int nPosition);

   void UpdateSeekPercentValue(int nXValue);

   void UpdateElementPercentValue(int nXValue);
   void SetElementPercentValue(float fValue);

   D2D1_RECT_F RectToFloatRect(CRect rcRect);

   CRect GetSpeedCurveRect();
   CRect GetElementRect();

   float m_fDuration;

   ComPtr<ID2D1Factory1>               m_pD2DFactory;
   ComPtr<ID2D1Device>                 m_pDevice;
   ComPtr<ID2D1DeviceContext>          m_pD2DContext;
   ComPtr<IDXGISwapChain1>             m_pSwapChain;
   ComPtr<ID2D1Bitmap1>                m_pTargetBitmap;

   ComPtr<IDWriteFactory>              m_pD2DWriteFactory;
   ComPtr<IDWriteTextFormat>           m_pD2DTextFormat;

   ComPtr<ID2D1SolidColorBrush>        m_pWhiteBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pRedBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pGreenBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pBlueBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pGrayBrush;

   ComPtr<ID2D1SolidColorBrush>        m_pDurationAreaSeparatorBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pCurveAreaSeparatorBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pCurveBaseLineSeparatorBrush;

   BOOL m_bLButtonDownOnCurveArea;
   BOOL m_bLButtonDownOnElementArea;
   float m_fSeekPercentValue;
   float m_fElementPercentValue;
};
