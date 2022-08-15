#pragma once

using namespace Microsoft::WRL;

class CBezierData;

class CBezierCurveControl : public CWnd
{
   DECLARE_DYNAMIC(CBezierCurveControl)

public:
   CBezierCurveControl();
   ~CBezierCurveControl();

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

   void UpdateBezierData();

   void UpdateDirect2DGeometry();

   void CalculateBezierPoint(ComPtr<ID2D1PathGeometry> pBezierGeometry, float fXValue);

   //void UpdateDirect2DPathGeometry();

   ComPtr<ID2D1Factory1>               m_pD2DFactory;
   ComPtr<ID2D1Device>                 m_pDevice;
   ComPtr<ID2D1DeviceContext>          m_pD2DContext;
   ComPtr<IDXGISwapChain1>             m_pSwapChain;
   ComPtr<ID2D1Bitmap1>                m_pTargetBitmap;

   ComPtr<ID2D1SolidColorBrush>        m_pWhiteBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pRedBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pGreenBrush;
   ComPtr<ID2D1SolidColorBrush>        m_pBlueBrush;

   ComPtr<IWICImagingFactory2>         m_pWicFactory;
   //ComPtr<ID2D1PathGeometry>           m_pDirect2DGeometry;

   BOOL m_bLButtonDown;

   CBezierData* m_pBezierData;
};
