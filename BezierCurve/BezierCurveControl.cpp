#include "pch.h"
#include "BezierData.h"
#include "BezierCurveControl.h"

IMPLEMENT_DYNAMIC(CBezierCurveControl, CWnd)

CBezierCurveControl::CBezierCurveControl()
   : m_pD2DFactory(nullptr),
   m_pDevice(nullptr),
   m_pD2DContext(nullptr),
   m_pSwapChain(nullptr),
   m_pTargetBitmap(nullptr),
   m_pWhiteBrush(nullptr),
   m_pRedBrush(nullptr),
   m_pGreenBrush(nullptr),
   m_pBlueBrush(nullptr),
   //m_pDirect2DGeometry(nullptr),
   m_bLButtonDown(FALSE)
{
   m_pBezierData = new CBezierData();
}

CBezierCurveControl::~CBezierCurveControl()
{
   ReleaseDirect2DResources();

   if (m_pBezierData != nullptr)
   {
      delete m_pBezierData;
      m_pBezierData = nullptr;
   }
}

BEGIN_MESSAGE_MAP(CBezierCurveControl, CWnd)
   ON_WM_CREATE()
   ON_WM_SIZE()
   ON_WM_ERASEBKGND()
   ON_WM_PAINT()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONDBLCLK()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

int CBezierCurveControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   CreateDeviceIndependentResources();

   return CWnd::OnCreate(lpCreateStruct);
}

void CBezierCurveControl::OnSize(UINT nType, int cx, int cy)
{
   CWnd::OnSize(nType, cx, cy);

   if (cx > 0 && cy > 0)
   {
      ResizeDirect2DResources(cx, cy);

      UpdateBezierData();

      UpdateDirect2DGeometry();

      //UpdateDirect2DPathGeometry();
   }
}

BOOL CBezierCurveControl::OnEraseBkgnd(CDC* pDC)
{
   return FALSE;
}

void CBezierCurveControl::OnPaint()
{
   CPaintDC dc(this);

   //CreateDeviceResources();
   //UpdateDirect2DGeometry();

   if (m_pD2DFactory == nullptr)
   {
      return;
   }


   HRESULT hr = S_OK;

   // create direct2d geometry
   ComPtr<ID2D1PathGeometry> pGeometry = nullptr;
   hr = m_pD2DFactory->CreatePathGeometry(&pGeometry);
   if (FAILED(hr))
   {
      return;
   }

   ID2D1GeometrySink* pSink = nullptr;
   hr = pGeometry->Open(&pSink);
   if (FAILED(hr))
   {
      return;
   }

   m_pBezierData->CreateBezierGeometry(pSink);

   D2D1_RECT_F fRectBezierBounds = D2D1::RectF();
   hr = pGeometry->GetBounds(D2D1::Matrix3x2F::Identity(), &fRectBezierBounds);

   CRect rcClient;
   GetClientRect(rcClient);

   float fXValue = static_cast<float>(rcClient.Width()) / 8.f;
   CalculateBezierPoint(pGeometry, fXValue);

   m_pD2DContext->BeginDraw();
   m_pD2DContext->SetTarget(m_pTargetBitmap.Get());
   
   fRectBezierBounds = D2D1::RectF();
   hr = pGeometry->GetBounds(D2D1::Matrix3x2F::Identity(), &fRectBezierBounds);

   m_pD2DContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));

   //m_pD2DContext->DrawGeometry(m_pDirect2DGeometry.Get(), m_pRedBrush.Get(), 3.F);

   m_pD2DContext->DrawGeometry(pGeometry.Get(), m_pRedBrush.Get(), 3.F);

   m_pBezierData->DrawBezierPoints(m_pD2DContext, m_pGreenBrush, m_pBlueBrush);

   m_pBezierData->DrawCalculateBezierPoints(m_pD2DContext, m_pWhiteBrush);

   hr = m_pD2DContext->EndDraw();

   DXGI_PRESENT_PARAMETERS parameters = { 0 };
   parameters.DirtyRectsCount = 0;
   parameters.pDirtyRects = nullptr;
   parameters.pScrollRect = nullptr;
   parameters.pScrollOffset = nullptr;

   hr = m_pSwapChain->Present1(1, 0, &parameters);
}

void CBezierCurveControl::OnLButtonDown(UINT nFlags, CPoint point)
{
   SetCapture();

   m_bLButtonDown = TRUE;

   if (!m_pBezierData->LButtonDown(point))
   {
      m_pBezierData->ResetPointState();
   }

   Invalidate();
   UpdateWindow();

   CWnd::OnLButtonDown(nFlags, point);
}

void CBezierCurveControl::OnLButtonUp(UINT nFlags, CPoint point)
{
   ReleaseCapture();

   m_bLButtonDown = FALSE;

   m_pBezierData->LButtonUp();

   CWnd::OnLButtonUp(nFlags, point);
}

void CBezierCurveControl::OnMouseMove(UINT nFlags, CPoint point)
{
   CWnd::OnMouseMove(nFlags, point);

   if (m_bLButtonDown)
   {
      m_pBezierData->MouseMove(point);

      Invalidate();
      UpdateWindow();
   }
}

BOOL CBezierCurveControl::PreCreateWindow(CREATESTRUCT& cs)
{
   if (!CWnd::PreCreateWindow(cs))
   {
      return FALSE;
   }

   cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), NULL);
   return TRUE;
}

BOOL CBezierCurveControl::PreTranslateMessage(MSG* pMsg)
{
   return CWnd::PreTranslateMessage(pMsg);
}

void CBezierCurveControl::CreateDeviceIndependentResources()
{
   IDXGIAdapter* pDxgiAdapter = nullptr;
   ID3D11Device* pD3D11Device = nullptr;
   ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
   IDXGIDevice1* pDxgiDevice = nullptr;
   IDXGIFactory2* pDxgiFactory = nullptr;
   IDXGISurface* pDxgiBackBuffer = nullptr;

   UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

   D3D_FEATURE_LEVEL featureLevels[] = {
   D3D_FEATURE_LEVEL_11_1,
   D3D_FEATURE_LEVEL_11_0,
   D3D_FEATURE_LEVEL_10_1,
   D3D_FEATURE_LEVEL_10_0,
   D3D_FEATURE_LEVEL_9_3,
   D3D_FEATURE_LEVEL_9_2,
   D3D_FEATURE_LEVEL_9_1
   };

   D3D_FEATURE_LEVEL featureLevel;

   HRESULT hr = D3D11CreateDevice(pDxgiAdapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
      D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, &pD3D11DeviceContext);

   if (SUCCEEDED(hr))
   {
      hr = pD3D11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgiDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
   }

   if (SUCCEEDED(hr))
   {
      DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
      fullscreenDesc.RefreshRate.Numerator = 60;
      fullscreenDesc.RefreshRate.Denominator = 1;
      fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
      fullscreenDesc.Windowed = FALSE;

      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
      swapChainDesc.Height = 0;
      swapChainDesc.Width = 0;
      swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      swapChainDesc.BufferCount = 2;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.Flags = 0;
      swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      swapChainDesc.SampleDesc.Count = 1;
      swapChainDesc.SampleDesc.Quality = 0;
      swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
      swapChainDesc.Stereo = FALSE;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

      hr = pDxgiFactory->CreateSwapChainForHwnd(pD3D11Device, m_hWnd, &swapChainDesc, nullptr, nullptr, &m_pSwapChain);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
   }

   if (SUCCEEDED(hr))
   {
      hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &m_pD2DFactory);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DContext);
      m_pD2DContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
   }

   if (SUCCEEDED(hr))
   {
      D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
         D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
         D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
         96.0f,
         96.0f
      );

      hr = m_pD2DContext->CreateBitmapFromDxgiSurface(pDxgiBackBuffer, &bitmapProperties, &m_pTargetBitmap);
   }

   if (m_pWhiteBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
   }

   if (m_pRedBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pRedBrush);
   }

   if (m_pGreenBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_pGreenBrush);
   }

   if (m_pBlueBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_pBlueBrush);
   }
}

void CBezierCurveControl::CreateDeviceResources()
{
}

void CBezierCurveControl::ResizeDirect2DResources(int nWidth, int nHeight)
{
   if (m_pSwapChain == nullptr || m_pD2DContext == nullptr)
   {
      return;
   }

   CRect rcCient;
   GetClientRect(rcCient);

   if (rcCient.IsRectEmpty())
   {
      return;
   }

   ID2D1Image* pImage = nullptr;
   m_pD2DContext->GetTarget(&pImage);
   m_pD2DContext->SetTarget(nullptr);
   if (pImage)
   {
      pImage->Release();
   }

   m_pTargetBitmap = nullptr;

   IDXGISurface* pBuffer = NULL;
   m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
   if (pBuffer)
   {
      int i = pBuffer->Release();
      while (i > 0)
         i = pBuffer->Release();
   }
   pBuffer = NULL;
   m_pSwapChain->GetBuffer(1, IID_PPV_ARGS(&pBuffer));
   if (pBuffer)
   {
      int i = pBuffer->Release();
      while (i > 0)
         i = pBuffer->Release();
   }

   HRESULT hr = m_pSwapChain->ResizeBuffers(0, rcCient.Width(), rcCient.Height(), DXGI_FORMAT_UNKNOWN, 0);

   if (FAILED(hr))
   {
      return;
   }

   FLOAT dpiX = 96.0;
   FLOAT dpiY = 96.0;
   // Direct2D needs the dxgi version of the backbuffer surface pointer.
   ComPtr<IDXGISurface> dxgiBackBuffer;
   hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
   if (hr != S_OK)
   {
      return;
   }
   // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
   //DON'T CHANGE THESE VALUES!!! Bitmap creation will fail with anything but these values.
   D2D1_BITMAP_PROPERTIES1 bitmapProperties =
      D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
         D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

   // Get a D2D surface from the DXGI back buffer to use as the D2D render target.

   hr = m_pD2DContext->CreateBitmapFromDxgiSurface(
      dxgiBackBuffer.Get(),
      &bitmapProperties,
      &m_pTargetBitmap);

   if (hr != S_OK)
   {
      return;
   }
}

void CBezierCurveControl::ReleaseDirect2DResources()
{
   m_pD2DFactory = nullptr;
   m_pDevice = nullptr;
   m_pD2DContext = nullptr;
   m_pSwapChain = nullptr;
   m_pTargetBitmap = nullptr;
   m_pWhiteBrush = nullptr;
   m_pRedBrush = nullptr;
   m_pGreenBrush = nullptr;
   m_pBlueBrush = nullptr;
}

void CBezierCurveControl::UpdateBezierData()
{
   CRect rcRect;
   GetClientRect(rcRect);

   if (rcRect.Width() < 0 || rcRect.Height() < 0)
   {
      return;
   }

   ASSERT(m_pBezierData != nullptr);

   m_pBezierData->SetBezierStartPoint(CPoint(rcRect.left, rcRect.top + rcRect.Height() / 2));
   m_pBezierData->UpdateBezierPoint(rcRect);
}

void CBezierCurveControl::CalculateBezierPoint(ComPtr<ID2D1PathGeometry> pBezierGeometry, float fXValue)
{
   if (pBezierGeometry == nullptr)
   {
      return;
   }

   D2D1_RECT_F fRectBezierBounds = D2D1::RectF();
   D2D1_MATRIX_3X2_F matrixTransform = D2D1::Matrix3x2F::Identity();

   HRESULT hr = pBezierGeometry->GetBounds(nullptr, &fRectBezierBounds);

   // create direct2d geometry
   ComPtr<ID2D1PathGeometry> pLineGeometry = nullptr;
   hr = m_pD2DFactory->CreatePathGeometry(&pLineGeometry);
   if (FAILED(hr))
   {
      return;
   }

   ID2D1GeometrySink* pLineSink = nullptr;
   hr = pLineGeometry->Open(&pLineSink);
   if (FAILED(hr))
   {
      return;
   }

   D2D1_POINT_2F pointLineStart = D2D1::Point2F(fXValue, fRectBezierBounds.top);
   D2D1_POINT_2F pointLineEnd = D2D1::Point2F(fXValue, fRectBezierBounds.bottom);

   D2D1_POINT_2F points[2] = { pointLineStart, pointLineEnd };

   pLineSink->BeginFigure(pointLineStart, D2D1_FIGURE_BEGIN_HOLLOW);
   pLineSink->AddLine(pointLineEnd);
   pLineSink->EndFigure(D2D1_FIGURE_END_CLOSED);

   hr = pLineSink->Close();

   // intersect two geometry
   SpecializedSink* pIntersectSink = new SpecializedSink();

   hr = pLineGeometry->CombineWithGeometry(pBezierGeometry.Get(),
      D2D1_COMBINE_MODE_INTERSECT,
      NULL,
      pIntersectSink);

   if (SUCCEEDED(hr))
   {
      size_t nCount = pIntersectSink->GetIntersectPointsCount();

      if (nCount > 0)
      {
         std::vector<D2D1_POINT_2F>* vecPoints = pIntersectSink->GetIntersectPoins();

         for (size_t i = 0; i < nCount; i++)
         {
            D2D1_POINT_2F ptIntersect = vecPoints->at(i);
            BOOL bIsPointOnCurve = FALSE;
            HRESULT hr = pBezierGeometry->StrokeContainsPoint(ptIntersect, 1.f, nullptr, D2D1::Matrix3x2F::Identity(), &bIsPointOnCurve);
            if (SUCCEEDED(hr))
            {
               if (bIsPointOnCurve)
               {
                  OutputDebugString(_T("Point is on curve"));
               }
            }
         }
      }

      hr = pIntersectSink->Close();
   }

}

void CBezierCurveControl::UpdateDirect2DGeometry()
{
   /*if (m_pDirect2DGeometry != nullptr)
   {
      ID2D1GeometrySink* pSink = nullptr;
      HRESULT hr = m_pDirect2DGeometry->Open(&pSink);
      if (SUCCEEDED(hr))
      {
         m_pBezierData->CreateBezierGeometry(pSink);
      }
   }*/
}

//void CBezierCurveControl::UpdateDirect2DPathGeometry()
//{
//   CRect rcRect;
//   GetClientRect(rcRect);
//
//   if (rcRect.Width() < 0 || rcRect.Height() < 0)
//   {
//      return;
//   }
//
//   ASSERT(m_pD2DFactory != nullptr);
//
//   HRESULT hr = S_OK;
//
//   if (m_pDirect2DGeometry == nullptr)
//   {
//      hr = m_pD2DFactory->CreatePathGeometry(&m_pDirect2DGeometry);
//
//      if (FAILED(hr))
//      {
//         return;
//      }
//   }
//
//   int nVerticalMiddle = rcRect.Height() / 2;
//   int nHorizontalMiddle = rcRect.Width() / 2;
//
//   ID2D1GeometrySink* pSink = nullptr;
//   hr = m_pDirect2DGeometry->Open(&pSink);
//
//   if (SUCCEEDED(hr))
//   {
//      pSink->BeginFigure(D2D1::Point2F(0, nVerticalMiddle), D2D1_FIGURE_BEGIN_HOLLOW);
//
//      pSink->AddBezier(D2D1::BezierSegment(
//         D2D1::Point2F(rcRect.Width() / 8, 3 * rcRect.Height() / 8),
//         D2D1::Point2F(rcRect.Width() / 8, rcRect.Height() / 4),
//         D2D1::Point2F(rcRect.Width() / 4, rcRect.Height() / 4)
//      ));
//
//      pSink->AddBezier(D2D1::BezierSegment(
//         D2D1::Point2F(3 * rcRect.Width() / 8, rcRect.Height() / 4),
//         D2D1::Point2F(3 * rcRect.Width() / 8, 3 * rcRect.Height() / 8),
//         D2D1::Point2F(rcRect.Width() / 2, rcRect.Height() / 2)
//      ));
//
//      pSink->AddBezier(D2D1::BezierSegment(
//         D2D1::Point2F(5 * rcRect.Width() / 8, 5 * rcRect.Height() / 8),
//         D2D1::Point2F(5 * rcRect.Width() / 8, 3 * rcRect.Height() / 4),
//         D2D1::Point2F(3 * rcRect.Width() / 4, 3 * rcRect.Height() / 4)
//      ));
//
//      pSink->AddBezier(D2D1::BezierSegment(
//         D2D1::Point2F(7 * rcRect.Width() / 8, 5 * rcRect.Height() / 8),
//         D2D1::Point2F(rcRect.Width(), rcRect.Height() / 2),
//         D2D1::Point2F(rcRect.Width(), rcRect.Height() / 2)
//      ));
//
//      pSink->EndFigure(D2D1_FIGURE_END_OPEN);
//
//      pSink->Close();
//   }
//}






