#include "pch.h"
#include "SpeedDataItem.h"
#include "SpeedData.h"
#include "SpecializedSink.h"

//D2D1_BEZIER_SEGMENT
//BezierSegment(
//   _In_ CONST D2D1_POINT_2F& point1,                   // right control point of previous point
//   _In_ CONST D2D1_POINT_2F& point2,                   // left control point of current point
//   _In_ CONST D2D1_POINT_2F& point3                    // current curve point
//)

static constexpr auto SPEED_MULTIPLE_MAX_VALUE = 10.f;
static constexpr auto SPEED_MULTIPLE_BASE_VALUE = 1.f;
static constexpr auto SPEED_MULTIPLE_MIN_VALUE = 0.1f;

static constexpr auto SPEED_POINT_INTERIOR_MARGIN = 6.f;

static constexpr auto SPEED_POINT_RADIUS_X = 8.f;
static constexpr auto SPEED_POINT_RADIUS_Y = 8.f;

CSpeedData::CSpeedData()
{
   m_pDataItemController = new CSpeedDataItemController();
}

CSpeedData::~CSpeedData()
{
   if (m_pDataItemController != nullptr)
   {
      delete m_pDataItemController;
      m_pDataItemController = nullptr;
   }
}

void CSpeedData::Init(float const& duration)
{
   m_fDuration = duration;

   m_pDataItemController->RemoveAll();

   CSpeedDataItem* pItem = new CSpeedDataItem(0.0f, 1.0f, 1.0f);
   m_pDataItemController->AddItem(pItem);

   pItem = new CSpeedDataItem(0.25f, 1.0f, 1.0f);
   m_pDataItemController->AddItem(pItem);

   pItem = new CSpeedDataItem(0.5f, 1.0f, 1.0f);
   m_pDataItemController->AddItem(pItem);

   pItem = new CSpeedDataItem(0.75f, 1.0f, 1.0f);
   m_pDataItemController->AddItem(pItem);

   pItem = new CSpeedDataItem(1.0f, 1.0f, 1.0f);
   m_pDataItemController->AddItem(pItem);
}

void CSpeedData::SetDuration(float const& fDuration)
{
   m_fDuration = fDuration;
}

float CSpeedData::GetDuration() const
{
   return m_fDuration;
}

BOOL CSpeedData::IsAllowToAddItem(float fTimePercent, CRect rcClient)
{
   int nAddXValue = FLOAT_TO_INT(PercentToXValue(fTimePercent, rcClient));

   int nTolerance = FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN + 2 * SPEED_POINT_RADIUS_X);

   // find from the first one to the first item who's percent is bigger than current one
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);

      float fItemPercent = pItem->GetTimePercentValue();
      int nItemXValue = FLOAT_TO_INT(PercentToXValue(fItemPercent, rcClient));

      if (fItemPercent > fTimePercent)
      {
         break;
      }
   }

   return TRUE;
}

void CSpeedData::AddItem(float fTimePercent, float fSpeedValue)
{

}

float CSpeedData::PercentToXValue(float const& fPercent, CRect rcClient)
{
   return INT_TO_FLOAT(rcClient.Width()) * fPercent;
}

float CSpeedData::XValueToPercent(int nXValue, CRect rcClient)
{
   int nValue = max(nXValue, rcClient.left);
   nValue = min(nValue, rcClient.right);

   return INT_TO_FLOAT(nValue) / INT_TO_FLOAT(rcClient.Width());
}

float CSpeedData::SpeedValueToYValue(float const& fSpeed, CRect rcClient)
{
   int nSectionHeight = rcClient.Height() / 5;

   int nBaseYValue = rcClient.top + 2 * nSectionHeight;

   float fSpeedValue = fminf(fSpeed, SPEED_MULTIPLE_MAX_VALUE);
   fSpeedValue = fmaxf(fSpeedValue, SPEED_MULTIPLE_MIN_VALUE);

   if (fSpeedValue > SPEED_MULTIPLE_BASE_VALUE)
   {
      float fYValue = INT_TO_FLOAT(nBaseYValue) - (fSpeedValue - SPEED_MULTIPLE_BASE_VALUE) / (SPEED_MULTIPLE_MAX_VALUE - SPEED_MULTIPLE_BASE_VALUE) * (2.f * INT_TO_FLOAT(nSectionHeight));
      return fYValue;
   }

   float fYValue = INT_TO_FLOAT(nBaseYValue) + (SPEED_MULTIPLE_BASE_VALUE - fSpeedValue) / (SPEED_MULTIPLE_BASE_VALUE - SPEED_MULTIPLE_MIN_VALUE) * (2.f * INT_TO_FLOAT(nSectionHeight));
   return fYValue;
}

float CSpeedData::YValueToSpeedValue(int nYValue, CRect rcClient)
{
   int nSectionHeight = rcClient.Height() / 5;

   int nBaseYValue = rcClient.top + 2 * nSectionHeight;

   int nCurrentYValue = max(rcClient.top, nYValue);
   nCurrentYValue = min(rcClient.bottom - nSectionHeight, nCurrentYValue);

   if (nCurrentYValue > nBaseYValue)
   {
      int nOffset = nCurrentYValue - nBaseYValue;

      float fValue = SPEED_MULTIPLE_BASE_VALUE - INT_TO_FLOAT(nOffset) / (2.f * INT_TO_FLOAT(nSectionHeight)) * (SPEED_MULTIPLE_BASE_VALUE - SPEED_MULTIPLE_MIN_VALUE);

      return fValue;
   }

   int nOffset = nBaseYValue - nCurrentYValue;

   return SPEED_MULTIPLE_BASE_VALUE + INT_TO_FLOAT(nOffset) / (2.f * INT_TO_FLOAT(nSectionHeight)) * (SPEED_MULTIPLE_MAX_VALUE - SPEED_MULTIPLE_BASE_VALUE);
}

void CSpeedData::ResetDataSelectState()
{
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);
      
      pItem->SelectLeftItem(FALSE);
      pItem->SelectRightItem(FALSE);
   }
}

BOOL CSpeedData::AnyItemSelect()
{
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);

      if (pItem->IsLeftItemSelect() || pItem->IsRightItemSelect())
      {
         return TRUE;
      }
   }

   return FALSE;
}

BOOL CSpeedData::SelectDataItem(CPoint point, CRect rcClient)
{
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);

      // check left part
      D2D1_POINT_2F ptLeftItem = D2D1::Point2F();
      ptLeftItem.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
      ptLeftItem.x -= (SPEED_POINT_INTERIOR_MARGIN / 2.f);
      ptLeftItem.y = SpeedValueToYValue(pItem->GetLeftSpeedValue(), rcClient);

      CRect rcLeftItem;
      rcLeftItem.right = FLOAT_TO_INT(ptLeftItem.x);
      rcLeftItem.left = rcLeftItem.right - FLOAT_TO_INT(SPEED_POINT_RADIUS_X);
      rcLeftItem.top = FLOAT_TO_INT(ptLeftItem.y - (SPEED_POINT_RADIUS_Y / 2.f));
      rcLeftItem.bottom = rcLeftItem.top + FLOAT_TO_INT(SPEED_POINT_RADIUS_Y);

      if (rcLeftItem.PtInRect(point))
      {
         pItem->SelectLeftItem(TRUE);
         return TRUE;
      }

      D2D1_POINT_2F ptRightItem = D2D1::Point2F();
      ptRightItem.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
      ptRightItem.x += (SPEED_POINT_INTERIOR_MARGIN / 2.f);
      ptRightItem.y = SpeedValueToYValue(pItem->GetRightSpeedValue(), rcClient);

      CRect rcRightItem;
      rcRightItem.left = FLOAT_TO_INT(ptRightItem.x);
      rcRightItem.right = rcRightItem.left + FLOAT_TO_INT(SPEED_POINT_RADIUS_X);
      rcRightItem.top = FLOAT_TO_INT(ptRightItem.y - (SPEED_POINT_RADIUS_Y / 2.f));
      rcRightItem.bottom = rcRightItem.top + FLOAT_TO_INT(SPEED_POINT_RADIUS_Y);

      if (rcRightItem.PtInRect(point))
      {
         pItem->SelectRightItem(TRUE);
         return TRUE;
      }
   }

   return FALSE;
}

void CSpeedData::MoveDataItem(CPoint point, CRect rcClient)
{
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);

      if (!pItem->IsLeftItemSelect() && !pItem->IsRightItemSelect())
      {
         continue;
      }

      if (pItem->GetNextItem() != nullptr && pItem->GetPrevItem() != nullptr)
      {
         float fPrevPercent = pItem->GetPrevItem()->GetTimePercentValue();
         int nPrevEdge = FLOAT_TO_INT(PercentToXValue(fPrevPercent, rcClient));
         nPrevEdge += FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f);
         nPrevEdge += FLOAT_TO_INT(SPEED_POINT_RADIUS_X);

         float fNextPercent = pItem->GetNextItem()->GetTimePercentValue();
         int nNextEdge = FLOAT_TO_INT(PercentToXValue(fNextPercent, rcClient));
         nNextEdge -= FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f);
         nNextEdge -= FLOAT_TO_INT(SPEED_POINT_RADIUS_X);

         int nPoint = point.x;
         if (nPoint - FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f) - FLOAT_TO_INT(SPEED_POINT_RADIUS_X) < nPrevEdge)
         {
            nPoint = nPrevEdge + FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f) + FLOAT_TO_INT(SPEED_POINT_RADIUS_X);
         }

         if (nPoint + FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f) + FLOAT_TO_INT(SPEED_POINT_RADIUS_X) > nNextEdge)
         {
            nPoint = nNextEdge - FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN / 2.f) - FLOAT_TO_INT(SPEED_POINT_RADIUS_X);
         }

         float fTimePercentValue = XValueToPercent(nPoint, rcClient);

         pItem->SetTimePercentValue(fTimePercentValue);
      }

      float fPercentValue = YValueToSpeedValue(point.y, rcClient);
      fPercentValue = fmaxf(fPercentValue, SPEED_MULTIPLE_MIN_VALUE);
      fPercentValue = fminf(fPercentValue, SPEED_MULTIPLE_MAX_VALUE);

      if (pItem->IsLeftItemSelect())
      {
         pItem->SetLeftSpeedValue(fPercentValue);
      }

      if (pItem->IsRightItemSelect())
      {
         pItem->SetRightSpeedValue(fPercentValue);
      }

      break;
   }
}

HRESULT CSpeedData::CreateSpeedCurveGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemController == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);
      CSpeedDataItem* pNextItem = pItem->GetNextItem();
      if (pNextItem == nullptr)
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F();
      ptStart.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
      ptStart.x += (SPEED_POINT_INTERIOR_MARGIN / 2.f);
      ptStart.y = SpeedValueToYValue(pItem->GetRightSpeedValue(), rcClient);

      pSink->BeginFigure(ptStart, D2D1_FIGURE_BEGIN_HOLLOW);

      pSink->AddBezier(D2D1::BezierSegment(ptStart, ptStart, ptStart));

      D2D1_POINT_2F ptSecondPoint = D2D1::Point2F(PercentToXValue(pNextItem->GetTimePercentValue(), rcClient),
         SpeedValueToYValue(pNextItem->GetLeftSpeedValue(), rcClient));
      ptSecondPoint.x -= SPEED_POINT_INTERIOR_MARGIN / 2.f;

      float fDistance = (ptSecondPoint.x - ptStart.x) / 2.f;

      D2D1_POINT_2F ptFirstControlPoint = ptStart;
      ptFirstControlPoint.x += fDistance;

      D2D1_POINT_2F ptSecondControlPoint = ptSecondPoint;
      ptSecondControlPoint.x -= fDistance;

      pSink->AddBezier(D2D1::BezierSegment(ptFirstControlPoint, ptSecondControlPoint, ptSecondPoint));

      pSink->EndFigure(D2D1_FIGURE_END_OPEN);
   }

   pSink->Close();

   return S_OK;
}

HRESULT CSpeedData::CreateSpeedCurvePointLineGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemController == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);
      D2D1_POINT_2F ptStart = D2D1::Point2F();
      ptStart.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
      //ptStart.x -= (SPEED_POINT_INTERIOR_MARGIN / 2.f);
      ptStart.y = SpeedValueToYValue(pItem->GetLeftSpeedValue(), rcClient);

      pSink->BeginFigure(ptStart, D2D1_FIGURE_BEGIN_HOLLOW);

      D2D1_POINT_2F ptEnd = D2D1::Point2F(PercentToXValue(pItem->GetTimePercentValue(), rcClient),
         SpeedValueToYValue(pItem->GetRightSpeedValue(), rcClient));

      pSink->AddLine(ptEnd);

      pSink->EndFigure(D2D1_FIGURE_END_OPEN);
   }

   pSink->Close();

   return S_OK;
}

HRESULT CSpeedData::CreateSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemController == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemController->GetHeadPosition();
   
   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);
      CSpeedDataItem* pNextItem = pItem->GetNextItem();
      if (pNextItem == nullptr)
      {
         break;
      }

      if (!pItem->IsRightItemSelect() && !pItem->IsLeftItemSelect())
      {
         D2D1_POINT_2F ptStart = D2D1::Point2F();
         ptStart.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
         ptStart.x += (SPEED_POINT_INTERIOR_MARGIN / 2.f);
         ptStart.y = SpeedValueToYValue(pItem->GetRightSpeedValue(), rcClient);
         ptStart.y -= SPEED_POINT_RADIUS_Y;

         pSink->BeginFigure(ptStart, D2D1_FIGURE_BEGIN_FILLED);

         pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptStart.x, ptStart.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
            0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

         pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
      }

      if (!pNextItem->IsLeftItemSelect() || !pNextItem->IsRightItemSelect())

      {
         D2D1_POINT_2F ptEnd = D2D1::Point2F(PercentToXValue(pNextItem->GetTimePercentValue(), rcClient),
            SpeedValueToYValue(pNextItem->GetLeftSpeedValue(), rcClient));
         ptEnd.x -= SPEED_POINT_INTERIOR_MARGIN / 2.f;
         ptEnd.y -= SPEED_POINT_RADIUS_Y;

         pSink->BeginFigure(ptEnd, D2D1_FIGURE_BEGIN_FILLED);

         pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptEnd.x, ptEnd.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
            0.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

         pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
      }

   }

   pSink->Close();

   return S_OK;
}

HRESULT CSpeedData::CreateSpeedDataSelectPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemController == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);

      if (pItem->IsRightItemSelect() || pItem->IsLeftItemSelect())
      {
         D2D1_POINT_2F ptStart = D2D1::Point2F();
         ptStart.x = PercentToXValue(pItem->GetTimePercentValue(), rcClient);
         ptStart.x += (SPEED_POINT_INTERIOR_MARGIN / 2.f);
         ptStart.y = SpeedValueToYValue(pItem->GetRightSpeedValue(), rcClient);
         ptStart.y -= SPEED_POINT_RADIUS_Y;

         pSink->BeginFigure(ptStart, D2D1_FIGURE_BEGIN_FILLED);

         pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptStart.x, ptStart.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
            0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

         pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

         D2D1_POINT_2F ptEnd = D2D1::Point2F(PercentToXValue(pItem->GetTimePercentValue(), rcClient),
            SpeedValueToYValue(pItem->GetLeftSpeedValue(), rcClient));
         ptEnd.x -= SPEED_POINT_INTERIOR_MARGIN / 2.f;
         ptEnd.y -= SPEED_POINT_RADIUS_Y;

         pSink->BeginFigure(ptEnd, D2D1_FIGURE_BEGIN_FILLED);

         pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptEnd.x, ptEnd.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
            0.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

         pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

         pSink->Close();

         return S_OK;
      }
   }

   return E_FAIL;
}

HRESULT CSpeedData::CreateVirtualBaseGeometry(ID2D1GeometrySink* pSink)
{
   pSink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   pSink->AddLine(D2D1::Point2F(0.f, 1.f));
   pSink->AddLine(D2D1::Point2F(1.f, 1.f));
   pSink->AddLine(D2D1::Point2F(1.f, 0.f));
   pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
 
   pSink->Close();

   return S_OK;
}

HRESULT CSpeedData::CreateVirtualCurrentGeometry(ID2D1GeometrySink* pSink)
{
   POSITION pos = m_pDataItemController->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemController->GetNextItem(pos);
      CSpeedDataItem* pNextItem = pItem->GetNextItem();
      if (pNextItem == nullptr)
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F();
      ptStart.x = pItem->GetTimePercentValue();
      ptStart.y = pItem->GetRightSpeedValue();

      pSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);

      pSink->AddLine(ptStart);

      pSink->AddBezier(D2D1::BezierSegment(ptStart, ptStart, ptStart));

      D2D1_POINT_2F ptSecondPoint = D2D1::Point2F(pNextItem->GetTimePercentValue(),pNextItem->GetLeftSpeedValue());

      float fDistance = (ptSecondPoint.x - ptStart.x) / 2.f; 

      D2D1_POINT_2F ptFirstControlPoint = ptStart;
      ptFirstControlPoint.x += fDistance;

      D2D1_POINT_2F ptSecondControlPoint = ptSecondPoint;
      ptSecondControlPoint.x -= fDistance;

      pSink->AddBezier(D2D1::BezierSegment(ptFirstControlPoint, ptSecondControlPoint, ptSecondPoint));

      pSink->AddLine(D2D1::Point2F(ptSecondPoint.x, 0.f));

      pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
   }

   pSink->Close();

   return S_OK;
}

float CSpeedData::GetCurrentDuration(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory)
{
   if (pD2DFactory == nullptr || m_pDataItemController == nullptr)
   {
      return m_fDuration;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return m_fDuration;
   }

   // 1. create original geometry
   ComPtr<ID2D1PathGeometry> pBaseGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pBaseGeometry);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ID2D1GeometrySink* pBaseSink = nullptr;
   hr = pBaseGeometry->Open(&pBaseSink);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   if (FAILED(CreateVirtualBaseGeometry(pBaseSink)))
   {
      return m_fDuration;
   }

   float fOriginArea = 1.f;
   hr = pBaseGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fOriginArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   // 2. create current geometry
   ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ID2D1GeometrySink* pCurrentSink = nullptr;
   hr = pCurrentGeometry->Open(&pCurrentSink);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   if (FAILED(CreateVirtualCurrentGeometry(pCurrentSink)))
   {
      return m_fDuration;
   }

   float fCurrentArea = 1.f;
   hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ASSERT(fOriginArea > 0.f && fCurrentArea > 0.f);

   float fCurrentDuration = m_fDuration * fOriginArea / fCurrentArea;

   return fCurrentDuration;
}

float CSpeedData::CalculateDecodeTime(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent)
{
   if (pD2DFactory == nullptr || m_pDataItemController == nullptr)
   {
      return m_fDuration;
   }

   if (m_pDataItemController->GetItemCount() <= 0)
   {
      return m_fDuration;
   }

   // 1. create original geometry
   ComPtr<ID2D1PathGeometry> pBaseGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pBaseGeometry);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ID2D1GeometrySink* pBaseSink = nullptr;
   hr = pBaseGeometry->Open(&pBaseSink);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   if (FAILED(CreateVirtualBaseGeometry(pBaseSink)))
   {
      return m_fDuration;
   }

   float fOriginArea = 1.f;
   hr = pBaseGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fOriginArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   // 2. create current geometry
   ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ID2D1GeometrySink* pCurrentSink = nullptr;
   hr = pCurrentGeometry->Open(&pCurrentSink);
   if (FAILED(hr))
   {
      return m_fDuration;
   }

   if (FAILED(CreateVirtualCurrentGeometry(pCurrentSink)))
   {
      return m_fDuration;
   }

   float fCurrentArea = 1.f;
   hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ASSERT(fOriginArea > 0.f && fCurrentArea > 0.f);

   float fCurrentDuration = m_fDuration * fOriginArea / fCurrentArea;

   D2D1_RECT_F fRectBezierBounds = D2D1::RectF();
   D2D1_MATRIX_3X2_F matrixTransform = D2D1::Matrix3x2F::Identity();

   if (FAILED(pCurrentGeometry->GetBounds(nullptr, &fRectBezierBounds)))
   {
      return m_fDuration;
   }

   //ComPtr<ID2D1RectangleGeometry> pRectangleGeometry = nullptr;
   //hr = pD2DFactory->CreateRectangleGeometry(D2D1::RectF(0.f, 0.f/* fRectBezierBounds.bottom - fRectBezierBounds.top*/, fPercent, SPEED_MULTIPLE_MAX_VALUE), &pRectangleGeometry);

   
   ComPtr<ID2D1PathGeometry> pRectangleGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pRectangleGeometry);
   ID2D1GeometrySink* pRectangleGeometrySink = nullptr;
   hr = pRectangleGeometry->Open(&pRectangleGeometrySink);

   pRectangleGeometrySink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   pRectangleGeometrySink->AddLine(D2D1::Point2F(0.f, SPEED_MULTIPLE_MAX_VALUE));
   pRectangleGeometrySink->AddLine(D2D1::Point2F(fPercent, SPEED_MULTIPLE_MAX_VALUE));
   pRectangleGeometrySink->AddLine(D2D1::Point2F(fPercent, 0.f));
   pRectangleGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
   pRectangleGeometrySink->Close();

   float fRectangleArea = 1.f;
   hr = pRectangleGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fRectangleArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   ComPtr<ID2D1PathGeometry> pUnionGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pUnionGeometry);
   ID2D1GeometrySink* pUnionGeometrySink = nullptr;
   hr = pUnionGeometry->Open(&pUnionGeometrySink);

   hr = pCurrentGeometry->CombineWithGeometry(pRectangleGeometry.Get(), D2D1_COMBINE_MODE_INTERSECT, NULL, pUnionGeometrySink);
   pUnionGeometrySink->Close();

   float fIntersetctArea = 1.f;
   hr = pUnionGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fIntersetctArea);

   if (FAILED(hr))
   {
      return m_fDuration;
   }

   float fDecodeTime = m_fDuration * fIntersetctArea / fCurrentArea;

   return fDecodeTime;
}
