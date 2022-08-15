#include "pch.h"
#include "SpeedData.h"
#include "SpeedCurveCtrl.h"

static constexpr auto DURATION_AREA_HEIGHT = 50;

static constexpr auto DURATION_TEXT_HORIZONTAL_MARGIN = 10;
static constexpr auto DURATION_TEXT_VERTICAL_MARGIN = 5;

static constexpr auto DURATION_ORIGINAL = _T("Original Duration : 90s");
static constexpr auto DURATION_VALUE = 90.0f;
static constexpr auto DURATION_CURRENT_TITLE = _T("Current Duration : ");

static constexpr auto SEEK_TITLE = _T("Seek : ");

static constexpr auto SPEED_MULTIPLE_MAX_VALUE_STRING = _T("10x");
static constexpr auto SPEED_MULTIPLE_BASE_VALUE_STRING = _T("1x");
static constexpr auto SPEED_MULTIPLE_MIN_VALUE_STRING = _T("0.1x");

static constexpr auto LBUTTONDOWN_LINE_WIDTH = 3.f;

IMPLEMENT_DYNAMIC(CSpeedCurveCtrl, CWnd)

CSpeedCurveCtrl::CSpeedCurveCtrl()
   : m_pD2DFactory(nullptr),
   m_pDevice(nullptr),
   m_pD2DContext(nullptr),
   m_pSwapChain(nullptr),
   m_pTargetBitmap(nullptr),
   m_pWhiteBrush(nullptr),
   m_pRedBrush(nullptr),
   m_pGreenBrush(nullptr),
   m_pBlueBrush(nullptr),
   m_fDuration(DURATION_VALUE),
   m_bLButtonDown(FALSE),
   m_fLButtonDownPercentValue(1.f)
{
   m_pData = new CSpeedData();
   m_pData->Init(m_fDuration);
}

CSpeedCurveCtrl::~CSpeedCurveCtrl()
{
   ReleaseDirect2DResources();
}

BEGIN_MESSAGE_MAP(CSpeedCurveCtrl, CWnd)
   ON_WM_CREATE()
   ON_WM_SIZE()
   ON_WM_ERASEBKGND()
   ON_WM_PAINT()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONDBLCLK()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

int CSpeedCurveCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   CreateDeviceIndependentResources();

   return CWnd::OnCreate(lpCreateStruct);
}

void CSpeedCurveCtrl::OnSize(UINT nType, int cx, int cy)
{
   CWnd::OnSize(nType, cx, cy);

   if (cx > 0 && cy > 0)
   {
      ResizeDirect2DResources(cx, cy);
   }
}

BOOL CSpeedCurveCtrl::OnEraseBkgnd(CDC* pDC)
{
   return FALSE;
}

void CSpeedCurveCtrl::DrawDurationArea()
{
   CRect rcClient;
   GetClientRect(rcClient);
   rcClient.bottom = rcClient.top + DURATION_AREA_HEIGHT;

   D2D1_POINT_2F ptStart = D2D1::Point2F(INT_TO_FLOAT(rcClient.left), INT_TO_FLOAT(rcClient.top + DURATION_AREA_HEIGHT));
   D2D1_POINT_2F ptEnd = D2D1::Point2F(INT_TO_FLOAT(rcClient.right), INT_TO_FLOAT(rcClient.top + DURATION_AREA_HEIGHT));

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pDurationAreaSeparatorBrush.Get());

   CRect rcText(rcClient);
   rcText.DeflateRect(DURATION_TEXT_HORIZONTAL_MARGIN, DURATION_TEXT_VERTICAL_MARGIN);

   // Draw original duration text
   ComPtr<IDWriteTextLayout> pOriginalDurationTextLayout = nullptr;
   CString strOriginalDuration = DURATION_ORIGINAL;
   HRESULT hr = m_pD2DWriteFactory->CreateTextLayout(
      strOriginalDuration,
      strOriginalDuration.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pOriginalDurationTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pOriginalDurationTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   DWRITE_TEXT_METRICS textMetrics;
   pOriginalDurationTextLayout->GetMetrics(&textMetrics);

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(INT_TO_FLOAT(rcText.left), INT_TO_FLOAT(rcText.top)/* - textMetrics.height + textMetrics.layoutHeight*/),
      pOriginalDurationTextLayout.Get(), m_pGreenBrush.Get());

   float fCurrentDuration = m_pData->GetCurrentDuration(m_pD2DFactory);

   // current seek value
   ComPtr<IDWriteTextLayout> pSeekTextLayout = nullptr;
   CString strSeek = SEEK_TITLE;
   float fSeek = m_pData->CalculateDecodeTime(m_pD2DFactory, m_fLButtonDownPercentValue);/*fCurrentDuration * m_fLButtonDownPercentValue;*/
   CString strSeekValue;
   strSeekValue.Format(_T("%.2f"), fSeek);
   strSeek += strSeekValue;
   strSeek += _T("s");

   hr = m_pD2DWriteFactory->CreateTextLayout(
      strSeek,
      strSeek.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pSeekTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pSeekTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   pSeekTextLayout->GetMetrics(&textMetrics);

   float fLeft = rcText.CenterPoint().x - textMetrics.width / 2.f;

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(fLeft, INT_TO_FLOAT(rcText.top)),
      pSeekTextLayout.Get(), m_pRedBrush.Get());

   // current value
   ComPtr<IDWriteTextLayout> pCurrentDurationTextLayout = nullptr;
   CString strCurrentDuration = DURATION_CURRENT_TITLE;

   CString strDurationValue;
   strDurationValue.Format(_T("%.2f"), fCurrentDuration);

   strCurrentDuration += strDurationValue;
   strCurrentDuration += _T("s");

   hr = m_pD2DWriteFactory->CreateTextLayout(
      strCurrentDuration,
      strCurrentDuration.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pCurrentDurationTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pCurrentDurationTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   pCurrentDurationTextLayout->GetMetrics(&textMetrics);

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(INT_TO_FLOAT(rcText.right) - textMetrics.width, INT_TO_FLOAT(rcText.top)/* - textMetrics.height + textMetrics.layoutHeight*/),
      pCurrentDurationTextLayout.Get(), m_pGreenBrush.Get());
}

void CSpeedCurveCtrl::DrawVerticalSeparator()
{
   ComPtr<ID2D1StrokeStyle> pStrokeStyle;
   HRESULT hr = m_pD2DFactory->CreateStrokeStyle(D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_FLAT,
      D2D1_CAP_STYLE_FLAT,
      D2D1_CAP_STYLE_FLAT,
      D2D1_LINE_JOIN_MITER,
      10.f,
      D2D1_DASH_STYLE_DASH,
      0.f), NULL, 0, &pStrokeStyle);

   CRect rcClient;
   GetClientRect(rcClient);
   rcClient.top += DURATION_AREA_HEIGHT;

   int nSectionHeight = rcClient.Height() / 5;

   // 10x string
   CRect rcText(rcClient);
   rcText.DeflateRect(DURATION_TEXT_HORIZONTAL_MARGIN, 0);
   rcText.bottom = rcText.top + nSectionHeight;

   // Draw original duration text
   ComPtr<IDWriteTextLayout> pMultipleTextLayout = nullptr;
   CString strText = SPEED_MULTIPLE_MAX_VALUE_STRING;
   hr = m_pD2DWriteFactory->CreateTextLayout(
      strText,
      strText.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pMultipleTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pMultipleTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(INT_TO_FLOAT(rcText.left), INT_TO_FLOAT(rcText.top)),
      pMultipleTextLayout.Get(), m_pGreenBrush.Get());

   D2D1_POINT_2F ptStart = D2D1::Point2F(INT_TO_FLOAT(rcClient.left), INT_TO_FLOAT(rcClient.top + nSectionHeight));
   D2D1_POINT_2F ptEnd = D2D1::Point2F(INT_TO_FLOAT(rcClient.right), INT_TO_FLOAT(rcClient.top + nSectionHeight));

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pCurveAreaSeparatorBrush.Get(), 1.f, pStrokeStyle.Get());

   ptStart.y += INT_TO_FLOAT(nSectionHeight);
   ptEnd.y += INT_TO_FLOAT(nSectionHeight);

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pCurveBaseLineSeparatorBrush.Get(), 1.f, pStrokeStyle.Get());

   // 1x string
   rcText.top = static_cast<int>(ptEnd.y);
   rcText.bottom = rcText.top + nSectionHeight;

   strText = SPEED_MULTIPLE_BASE_VALUE_STRING;
   hr = m_pD2DWriteFactory->CreateTextLayout(
      strText,
      strText.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pMultipleTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pMultipleTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(INT_TO_FLOAT(rcText.left), INT_TO_FLOAT(rcText.top)),
      pMultipleTextLayout.Get(), m_pGreenBrush.Get());

   ptStart.y += nSectionHeight;
   ptEnd.y += nSectionHeight;

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pCurveAreaSeparatorBrush.Get(), 1.f, pStrokeStyle.Get());

   // 0.1x string
   rcText.top = static_cast<int>(ptEnd.y);
   rcText.bottom = rcText.top + nSectionHeight;

   strText = SPEED_MULTIPLE_MIN_VALUE_STRING;
   hr = m_pD2DWriteFactory->CreateTextLayout(
      strText,
      strText.GetLength(),
      m_pD2DTextFormat.Get(),
      (FLOAT)(rcText.Width()),
      (FLOAT)rcText.Height(),
      &pMultipleTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   pMultipleTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   DWRITE_TEXT_METRICS textMetrics;
   pMultipleTextLayout->GetMetrics(&textMetrics);

   m_pD2DContext->DrawTextLayout(D2D1::Point2F(INT_TO_FLOAT(rcText.left), INT_TO_FLOAT(rcText.top) + nSectionHeight - textMetrics.height),
      pMultipleTextLayout.Get(), m_pGreenBrush.Get());

   ptStart.y += nSectionHeight;
   ptEnd.y += nSectionHeight;

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pDurationAreaSeparatorBrush.Get());
}

void CSpeedCurveCtrl::DrawSpeedCurve()
{
   if (m_pData == nullptr)
   {
      return;
   }

   ComPtr<ID2D1PathGeometry> pGeometry = nullptr;
   HRESULT hr = m_pD2DFactory->CreatePathGeometry(&pGeometry);
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

   if (FAILED(m_pData->CreateSpeedCurveGeometry(pSink, GetSpeedCurveRect())))
   {
      return;
   }


   m_pD2DContext->DrawGeometry(pGeometry.Get(), m_pBlueBrush.Get(), 3.F);
}

void CSpeedCurveCtrl::DrawSpeedCurveLines()
{
   if (m_pData == nullptr)
   {
      return;
   }

   ComPtr<ID2D1PathGeometry> pGeometry = nullptr;
   HRESULT hr = m_pD2DFactory->CreatePathGeometry(&pGeometry);
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

   if (FAILED(m_pData->CreateSpeedCurvePointLineGeometry(pSink, GetSpeedCurveRect())))
   {
      return;
   }

   ComPtr<ID2D1StrokeStyle> pStrokeStyle;
   if (FAILED(m_pD2DFactory->CreateStrokeStyle(D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_FLAT,
      D2D1_CAP_STYLE_FLAT,
      D2D1_CAP_STYLE_FLAT,
      D2D1_LINE_JOIN_MITER,
      10.f,
      D2D1_DASH_STYLE_DASH,
      0.f), NULL, 0, &pStrokeStyle)))
   {
      return;
   }

   m_pD2DContext->DrawGeometry(pGeometry.Get(), m_pCurveAreaSeparatorBrush.Get(), 3.F, pStrokeStyle.Get());
}

void CSpeedCurveCtrl::DrawSpeedCurvePoint()
{
   if (m_pData == nullptr)
   {
      return;
   }

   ComPtr<ID2D1PathGeometry> pGeometry = nullptr;
   HRESULT hr = m_pD2DFactory->CreatePathGeometry(&pGeometry);
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

   if (FAILED(m_pData->CreateSpeedDataPointGeometry(pSink, GetSpeedCurveRect())))
   {
      return;
   }

   m_pD2DContext->FillGeometry(pGeometry.Get(), m_pCurveAreaSeparatorBrush.Get());
}

void CSpeedCurveCtrl::DrawSpeedCurveSelectPoint()
{
   if (m_pData == nullptr)
   {
      return;
   }

   ComPtr<ID2D1PathGeometry> pGeometry = nullptr;
   HRESULT hr = m_pD2DFactory->CreatePathGeometry(&pGeometry);
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

   if (FAILED(m_pData->CreateSpeedDataSelectPointGeometry(pSink, GetSpeedCurveRect())))
   {
      return;
   }

   m_pD2DContext->FillGeometry(pGeometry.Get(), m_pWhiteBrush.Get());
}

void CSpeedCurveCtrl::DrawCurrentTimeLine()
{
   CRect rcClient;
   GetClientRect(rcClient);
   rcClient.top += DURATION_AREA_HEIGHT;

   int nSectionHeight = rcClient.Height() / 5;
   
   float fHorizontalValue = static_cast<float>(rcClient.Width()) * m_fLButtonDownPercentValue;

   D2D1_POINT_2F ptStart = D2D1::Point2F(fHorizontalValue, INT_TO_FLOAT(rcClient.top));
   D2D1_POINT_2F ptEnd = D2D1::Point2F(fHorizontalValue, INT_TO_FLOAT(rcClient.bottom - nSectionHeight));

   m_pD2DContext->DrawLine(ptStart, ptEnd, m_pRedBrush.Get(), LBUTTONDOWN_LINE_WIDTH);
}

void CSpeedCurveCtrl::OnPaint()
{
   CPaintDC dc(this);

   if (m_pD2DFactory == nullptr)
   {
      return;
   }


   HRESULT hr = S_OK;

   m_pD2DContext->BeginDraw();
   m_pD2DContext->SetTarget(m_pTargetBitmap.Get());

   m_pD2DContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));

   DrawDurationArea();

   DrawVerticalSeparator();
   DrawSpeedCurve();
   DrawSpeedCurveLines();
   DrawSpeedCurvePoint();
   DrawSpeedCurveSelectPoint();

   DrawCurrentTimeLine();

   hr = m_pD2DContext->EndDraw();

   DXGI_PRESENT_PARAMETERS parameters = { 0 };
   parameters.DirtyRectsCount = 0;
   parameters.pDirtyRects = nullptr;
   parameters.pScrollRect = nullptr;
   parameters.pScrollOffset = nullptr;

   hr = m_pSwapChain->Present1(1, 0, &parameters);
}

void CSpeedCurveCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
   SetCapture();

   m_bLButtonDown = TRUE;

   m_pData->ResetDataSelectState();

   CRect rcClient;
   GetClientRect(rcClient);

   if (!m_pData->SelectDataItem(point, GetSpeedCurveRect()))
   {
      m_fLButtonDownPercentValue = CalculateTimePercent(point.x);
   }

   Invalidate();
   UpdateWindow();

   CWnd::OnLButtonDown(nFlags, point);
}

void CSpeedCurveCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
   ReleaseCapture();

   m_bLButtonDown = FALSE;

   CWnd::OnLButtonUp(nFlags, point);
}

void CSpeedCurveCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
   if (m_bLButtonDown)
   {
      if (!m_pData->AnyItemSelect())
      {
         m_fLButtonDownPercentValue = CalculateTimePercent(point.x);
      }
      else
      {
         m_pData->MoveDataItem(point, GetSpeedCurveRect());
      }

      Invalidate();
      UpdateWindow();
   }

   CWnd::OnMouseMove(nFlags, point);
}

BOOL CSpeedCurveCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
   if (!CWnd::PreCreateWindow(cs))
   {
      return FALSE;
   }

   cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), NULL);
   return TRUE;
}

BOOL CSpeedCurveCtrl::PreTranslateMessage(MSG* pMsg)
{
   return CWnd::PreTranslateMessage(pMsg);
}

void CSpeedCurveCtrl::CreateDeviceIndependentResources()
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

   if (m_pDurationAreaSeparatorBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(59.0f / 255.0f, 59.0f / 255.0f, 59.0f / 255.0f), &m_pDurationAreaSeparatorBrush);
   }

   if (m_pCurveAreaSeparatorBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(82.0f / 255.0f, 82.0f / 255.0f, 82.0f / 255.0f), &m_pCurveAreaSeparatorBrush);
   }

   if (m_pCurveBaseLineSeparatorBrush == nullptr)
   {
      m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(153.0f / 255.0f, 153.0f / 255.0f, 153.0f / 255.0f), &m_pCurveBaseLineSeparatorBrush);
   }

   hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pD2DWriteFactory);

   if (SUCCEEDED(hr))
   {
      hr = m_pD2DWriteFactory->CreateTextFormat(_T("Segio UI"),
         NULL,
         DWRITE_FONT_WEIGHT_NORMAL,
         DWRITE_FONT_STYLE_NORMAL,
         DWRITE_FONT_STRETCH_NORMAL,
         16,
         _T(""),
         &m_pD2DTextFormat);
      m_pD2DTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
   }
}

void CSpeedCurveCtrl::CreateDeviceResources()
{
}

void CSpeedCurveCtrl::ResizeDirect2DResources(int nWidth, int nHeight)
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

void CSpeedCurveCtrl::ReleaseDirect2DResources()
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

float CSpeedCurveCtrl::CalculateTimePercent(int nPosition)
{
   CRect rcClient;
   GetClientRect(rcClient);

   if (rcClient.Width() <= 0)
   {
      return 0.f;
   }

   float fPercent = INT_TO_FLOAT(nPosition) / INT_TO_FLOAT(rcClient.Width());

   fPercent = fminf(fPercent, 1.f);
   fPercent = fmaxf(0.f, fPercent);

   return fPercent;
}

CRect CSpeedCurveCtrl::GetSpeedCurveRect()
{
   CRect rcClient;
   GetClientRect(rcClient);
   rcClient.top += DURATION_AREA_HEIGHT;

   return rcClient;
}

void CSpeedCurveCtrl::ResetSpeedData()
{
   ASSERT(m_pData != nullptr);

   m_pData->Init(m_fDuration);

   Invalidate();
   UpdateWindow();
}

void CSpeedCurveCtrl::AddSpeedDataItem()
{
}
