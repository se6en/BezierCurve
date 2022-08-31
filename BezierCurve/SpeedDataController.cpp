#include "pch.h"
#include "SpeedDataItemList.h"
#include "SpeedDataController.h"
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

static constexpr auto SEEK_PERCENT_CALCULATE_TOLERANCE = 0.001f;
static constexpr auto FLOAT_TYPE_TOLERANCE = 0.000001f;

static constexpr auto TIME_TOLERANCE = 0.016667f;

CSpeedDataController::CSpeedDataController()
{
   m_pDataItemList = new CSpeedDataItemList();
}

CSpeedDataController::~CSpeedDataController()
{
   if (m_pDataItemList != nullptr)
   {
      delete m_pDataItemList;
      m_pDataItemList = nullptr;
   }
}

CSpeedDataController& CSpeedDataController::GetInstance()
{
   static CSpeedDataController instance;
   return instance;
}

void CSpeedDataController::Init(float const& duration)
{
   m_fDuration = duration;
   m_fCurrentDuration = duration;

   m_pDataItemList->RemoveAll();

   CSpeedDataItem* pItem = new CSpeedDataItem(0.f, 0.f, 1.0f, 1.0f);
   m_pDataItemList->AddItem(pItem);

   pItem = new CSpeedDataItem(0.25f, 0.25f, 1.0f, 1.0f);
   m_pDataItemList->AddItem(pItem);

   pItem = new CSpeedDataItem(0.5f, 0.5f, 1.0f, 1.0f);
   m_pDataItemList->AddItem(pItem);

   pItem = new CSpeedDataItem(0.75f, 0.75f, 1.0f, 1.0f);
   m_pDataItemList->AddItem(pItem);

   pItem = new CSpeedDataItem(1.0f, 1.0f, 1.0f, 1.0f);
   m_pDataItemList->AddItem(pItem);
}

void CSpeedDataController::SetDuration(float const& fDuration)
{
   m_fDuration = fDuration;
}

float CSpeedDataController::GetDuration() const
{
   return m_fDuration;
}

BOOL CSpeedDataController::IsAllowToAddItem(float fTimePercent, CRect rcClient)
{
   int nAddXValue = FLOAT_TO_INT(PercentToXValue(fTimePercent, rcClient));

   int nTolerance = FLOAT_TO_INT(SPEED_POINT_INTERIOR_MARGIN + 2 * SPEED_POINT_RADIUS_X);

   // find from the first one to the first item who's percent is bigger than current one
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);

      float fItemPercent = pItem->GetTimePercentValue();
      int nItemXValue = FLOAT_TO_INT(PercentToXValue(fItemPercent, rcClient));

      if (fItemPercent > fTimePercent)
      {
         break;
      }
   }

   return TRUE;
}

void CSpeedDataController::AddItem(float fTimePercent, float fSpeedValue)
{

}

float CSpeedDataController::PercentToXValue(float const& fPercent, CRect rcClient)
{
   return INT_TO_FLOAT(rcClient.Width()) * fPercent;
}

float CSpeedDataController::XValueToPercent(int nXValue, CRect rcClient)
{
   int nValue = max(nXValue, rcClient.left);
   nValue = min(nValue, rcClient.right);

   return INT_TO_FLOAT(nValue) / INT_TO_FLOAT(rcClient.Width());
}

float CSpeedDataController::SpeedValueToYValue(float const& fSpeed, CRect rcClient)
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

float CSpeedDataController::YValueToSpeedValue(int nYValue, CRect rcClient)
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

void CSpeedDataController::UpdateCurrentDuration(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory)
{
   if (pD2DFactory == nullptr || m_pDataItemList == nullptr)
   {
      return;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return;
   }

   m_fCurrentDuration = 0.f;

   POSITION pos = m_pDataItemList->GetHeadPosition();
   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      CSpeedDataItem* pPrevItem = pItem->GetPrevItem();

      if (pPrevItem == nullptr)
      {
         // beginning point
         continue;
      }

      ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
      HRESULT hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
      if (FAILED(hr))
      {
         break;
      }

      ID2D1GeometrySink* pCurrentSink = nullptr;
      hr = pCurrentGeometry->Open(&pCurrentSink);
      if (FAILED(hr))
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F(0.f/*pPrevItem->GetTimePercentValue()*/, pPrevItem->GetRightSpeedValue());
      D2D1_POINT_2F ptEnd = D2D1::Point2F(pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue(), pItem->GetLeftSpeedValue());
      float fDistance = (ptEnd.x - ptStart.x) / 2.f;

      D2D1_POINT_2F ptFirstContrl = ptStart;
      ptFirstContrl.x += fDistance;

      D2D1_POINT_2F ptSecondContrl = ptEnd;
      ptSecondContrl.x -= fDistance;

      pCurrentSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);
      pCurrentSink->AddLine(ptStart);
      pCurrentSink->AddBezier(D2D1::BezierSegment(ptFirstContrl, ptSecondContrl, ptEnd));
      pCurrentSink->AddLine(D2D1::Point2F(ptEnd.x, 0.f));
      pCurrentSink->EndFigure(D2D1_FIGURE_END_CLOSED);
      pCurrentSink->Close();

      float fCurrentSegmentArea = 1.f;
      hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentSegmentArea);
      if (FAILED(hr))
      {
         break;
      }

      float fTimePercentOffset = pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue();
      float fTimeOffset = fTimePercentOffset * m_fDuration;
      float fSegmentTime = fTimePercentOffset * 1.f / fCurrentSegmentArea;
      fSegmentTime *= fTimeOffset;

      m_fCurrentDuration += fSegmentTime;
   }
   //// 1. create original geometry
   //ComPtr<ID2D1PathGeometry> pBaseGeometry = nullptr;
   //HRESULT hr = pD2DFactory->CreatePathGeometry(&pBaseGeometry);
   //if (FAILED(hr))
   //{
   //   return;
   //}

   //ID2D1GeometrySink* pBaseSink = nullptr;
   //hr = pBaseGeometry->Open(&pBaseSink);
   //if (FAILED(hr))
   //{
   //   return;
   //}

   //if (FAILED(CreateVirtualBaseGeometry(pBaseSink)))
   //{
   //   return;
   //}

   //float fOriginArea = 1.f;
   //hr = pBaseGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fOriginArea);

   //if (FAILED(hr))
   //{
   //   return;
   //}

   //// 2. create current geometry
   //float fCurrentArea = GetCurveAreaValue(pD2DFactory);

   //if (FAILED(fCurrentArea <= 0.f))
   //{
   //   return;
   //}

   //ASSERT(fOriginArea > 0.f && fCurrentArea > 0.f);

   //m_fCurrentDuration = m_fDuration * fOriginArea / fCurrentArea;
}

void CSpeedDataController::UpdateElementPercentValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      CSpeedDataItem* pPrevItem = pItem->GetPrevItem();

      if (pPrevItem == nullptr)
      {
         // beginning point
         pItem->SetElementPercentValue(0.f);
         continue;
      }

      ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
      HRESULT hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
      if (FAILED(hr))
      {
         break;
      }

      ID2D1GeometrySink* pCurrentSink = nullptr;
      hr = pCurrentGeometry->Open(&pCurrentSink);
      if (FAILED(hr))
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F(pPrevItem->GetTimePercentValue(), pPrevItem->GetRightSpeedValue());
      D2D1_POINT_2F ptEnd = D2D1::Point2F(pItem->GetTimePercentValue(), pItem->GetLeftSpeedValue());
      float fDistance = (ptEnd.x - ptStart.x) / 2.f;

      D2D1_POINT_2F ptFirstContrl = ptStart;
      ptFirstContrl.x += fDistance;

      D2D1_POINT_2F ptSecondContrl = ptEnd;
      ptSecondContrl.x -= fDistance;

      pCurrentSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);
      pCurrentSink->AddLine(ptStart);
      pCurrentSink->AddBezier(D2D1::BezierSegment(ptFirstContrl, ptSecondContrl, ptEnd));
      pCurrentSink->AddLine(D2D1::Point2F(ptEnd.x, 0.f));
      pCurrentSink->EndFigure(D2D1_FIGURE_END_CLOSED);
      pCurrentSink->Close();

      float fCurrentSegmentArea = 1.f;
      hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentSegmentArea);
      if (FAILED(hr))
      {
         break;
      }

      float fTimePercentOffset = pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue();
      float fTimeOffset = fTimePercentOffset * m_fDuration;
      float fElementTimePercentOffset = fTimePercentOffset * 1.f / fCurrentSegmentArea;
      fElementTimePercentOffset *= fTimeOffset;

      float fElementPercentOffset = fElementTimePercentOffset / m_fCurrentDuration;

      float fElemenmtPercent = pPrevItem->GetElementPercentValue() + fElementPercentOffset;

      pItem->SetElementPercentValue(fElemenmtPercent);
   }
}

void CSpeedDataController::UpdateDynamicValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory)
{
   // udate duration first
   UpdateCurrentDuration(pD2DFactory);

   UpdateElementPercentValue(pD2DFactory);
}

void CSpeedDataController::ResetDataSelectState()
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      
      pItem->SelectLeftItem(FALSE);
      pItem->SelectRightItem(FALSE);
   }
}

BOOL CSpeedDataController::AnyItemSelect()
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);

      if (pItem->IsLeftItemSelect() || pItem->IsRightItemSelect())
      {
         return TRUE;
      }
   }

   return FALSE;
}

BOOL CSpeedDataController::SelectDataItem(CPoint point, CRect rcClient)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);

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

void CSpeedDataController::MoveDataItem(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, CPoint point, CRect rcClient)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);

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

   UpdateDynamicValue(pD2DFactory);
}

HRESULT CSpeedDataController::CreateSpeedCurveGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemList == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
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

HRESULT CSpeedDataController::CreateSpeedCurvePointLineGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemList == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
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

HRESULT CSpeedDataController::CreateSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemList == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemList->GetHeadPosition();
   
   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
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

HRESULT CSpeedDataController::CreateSpeedDataSelectPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemList == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);

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

HRESULT CSpeedDataController::CreateElementSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient)
{
   if (pSink == nullptr || m_pDataItemList == nullptr)
   {
      return E_NOTIMPL;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return E_NOTIMPL;
   }

   pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      CSpeedDataItem* pNextItem = pItem->GetNextItem();
      if (pNextItem == nullptr)
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F();
      ptStart.x = PercentToXValue(pItem->GetElementPercentValue(), rcClient);
      ptStart.x += (SPEED_POINT_INTERIOR_MARGIN / 2.f);
      ptStart.y = INT_TO_FLOAT(rcClient.CenterPoint().y);
      ptStart.y -= SPEED_POINT_RADIUS_Y;

      pSink->BeginFigure(ptStart, D2D1_FIGURE_BEGIN_FILLED);

      pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptStart.x, ptStart.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
         0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

      pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

      D2D1_POINT_2F ptEnd = D2D1::Point2F(PercentToXValue(pNextItem->GetElementPercentValue(), rcClient),
         INT_TO_FLOAT(rcClient.CenterPoint().y));
      ptEnd.x -= SPEED_POINT_INTERIOR_MARGIN / 2.f;
      ptEnd.y -= SPEED_POINT_RADIUS_Y;

      pSink->BeginFigure(ptEnd, D2D1_FIGURE_BEGIN_FILLED);

      pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(ptEnd.x, ptEnd.y + 2 * SPEED_POINT_RADIUS_Y), D2D1::SizeF(SPEED_POINT_RADIUS_X, SPEED_POINT_RADIUS_Y),
         0.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

      pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
   }

   pSink->Close();

   return S_OK;
}

HRESULT CSpeedDataController::CreateVirtualBaseGeometry(ID2D1GeometrySink* pSink)
{
   pSink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   pSink->AddLine(D2D1::Point2F(0.f, 1.f));
   pSink->AddLine(D2D1::Point2F(1.f, 1.f));
   pSink->AddLine(D2D1::Point2F(1.f, 0.f));
   pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
 
   pSink->Close();

   return S_OK;
}

HRESULT CSpeedDataController::CreateVirtualCurrentGeometry(ID2D1GeometrySink* pSink)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
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

HRESULT CSpeedDataController::CreateVirtualElementGeometry(ID2D1GeometrySink* pSink)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      CSpeedDataItem* pNextItem = pItem->GetNextItem();
      if (pNextItem == nullptr)
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F();
      ptStart.x = pItem->GetElementPercentValue();
      ptStart.y = pItem->GetRightSpeedValue();

      pSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);

      pSink->AddLine(ptStart);

      pSink->AddBezier(D2D1::BezierSegment(ptStart, ptStart, ptStart));

      D2D1_POINT_2F ptSecondPoint = D2D1::Point2F(pNextItem->GetElementPercentValue(), pNextItem->GetLeftSpeedValue());

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

float CSpeedDataController::GetCurveAreaValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory)
{
   ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
   if (FAILED(hr))
   {
      return 0.f;
   }

   ID2D1GeometrySink* pCurrentSink = nullptr;
   hr = pCurrentGeometry->Open(&pCurrentSink);
   if (FAILED(hr))
   {
      return 0.f;
   }

   if (FAILED(CreateVirtualCurrentGeometry(pCurrentSink)))
   {
      return 0.f;
   }

   float fCurrentArea = 1.f;
   hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentArea);

   if (FAILED(hr))
   {
      return 0.f;
   }

   return fCurrentArea;
}

float CSpeedDataController::GetCurrentDuration()
{
   return m_fCurrentDuration;
}

float CSpeedDataController::GetRealPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent)
{
   if (pD2DFactory == nullptr || m_pDataItemList == nullptr)
   {
      return 0.f;
   }

   if (m_pDataItemList->GetItemCount() <= 0)
   {
      return 0.f;
   }

   // 1. create original geometry
   ComPtr<ID2D1PathGeometry> pBaseGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pBaseGeometry);
   if (FAILED(hr))
   {
      return 0.f;
   }

   ID2D1GeometrySink* pBaseSink = nullptr;
   hr = pBaseGeometry->Open(&pBaseSink);
   if (FAILED(hr))
   {
      return 0.f;
   }

   if (FAILED(CreateVirtualBaseGeometry(pBaseSink)))
   {
      return 0.f;
   }

   float fOriginArea = 1.f;
   hr = pBaseGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fOriginArea);

   if (FAILED(hr))
   {
      return 0.f;
   }

   // 2. create current geometry
   ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
   if (FAILED(hr))
   {
      return 0.f;
   }

   ID2D1GeometrySink* pCurrentSink = nullptr;
   hr = pCurrentGeometry->Open(&pCurrentSink);
   if (FAILED(hr))
   {
      return 0.f;
   }

   if (FAILED(CreateVirtualCurrentGeometry(pCurrentSink)))
   {
      return 0.f;
   }

   float fCurrentArea = 1.f;
   hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentArea);

   if (FAILED(hr))
   {
      return 0.f;
   }

   ASSERT(fOriginArea > 0.f && fCurrentArea > 0.f);

   float fCurrentDuration = m_fDuration * fOriginArea / fCurrentArea;

   D2D1_RECT_F fRectBezierBounds = D2D1::RectF();
   D2D1_MATRIX_3X2_F matrixTransform = D2D1::Matrix3x2F::Identity();

   if (FAILED(pCurrentGeometry->GetBounds(nullptr, &fRectBezierBounds)))
   {
      return 0.f;
   }

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
      return 0.f;
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
      return 0.f;
   }

   return fIntersetctArea / fCurrentArea;
}

float CSpeedDataController::CalculateDecodeTime(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent)
{
   //float fRealPercent = GetRealPercent(pD2DFactory, fPercent);

   float fDecodeTime = m_fDuration * fPercent;

   return fDecodeTime;
}

float CSpeedDataController::GetSeekTime(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent)
{
   float fSeekTime = m_fCurrentDuration * fPercent;

   return fSeekTime;
}

float CSpeedDataController::CalculateElementPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent)
{
   POSITION pos = m_pDataItemList->GetHeadPosition();

   while (pos != nullptr)
   {
      CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
      CSpeedDataItem* pPrevItem = pItem->GetPrevItem();

      if (pPrevItem == nullptr)
      {
         continue;
      }

      float fCurTimePercent = pItem->GetTimePercentValue();
      float fPrevTimePercent = pPrevItem->GetTimePercentValue();

      if (fPercent > fCurTimePercent)
      {
         continue;
      }

      if (fabsf(fPercent - fCurTimePercent) <= FLOAT_TYPE_TOLERANCE)
      {
         return pItem->GetElementPercentValue();
      }

      if (fabsf(fPercent - fPrevTimePercent) <= FLOAT_TYPE_TOLERANCE)
      {
         return pPrevItem->GetElementPercentValue();
      }

      ComPtr<ID2D1PathGeometry> pSegmentGeometry = nullptr;
      HRESULT hr = pD2DFactory->CreatePathGeometry(&pSegmentGeometry);
      if (FAILED(hr))
      {
         break;
      }

      ID2D1GeometrySink* pSegmentSink = nullptr;
      hr = pSegmentGeometry->Open(&pSegmentSink);
      if (FAILED(hr))
      {
         break;
      }

      D2D1_POINT_2F ptStart = D2D1::Point2F(0.f, pPrevItem->GetRightSpeedValue());
      D2D1_POINT_2F ptEnd = D2D1::Point2F(pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue(), pItem->GetLeftSpeedValue());
      float fDistance = (ptEnd.x - ptStart.x) / 2.f;

      D2D1_POINT_2F ptFirstContrl = ptStart;
      ptFirstContrl.x += fDistance;

      D2D1_POINT_2F ptSecondContrl = ptEnd;
      ptSecondContrl.x -= fDistance;

      pSegmentSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);
      pSegmentSink->AddLine(ptStart);
      pSegmentSink->AddBezier(D2D1::BezierSegment(ptFirstContrl, ptSecondContrl, ptEnd));
      pSegmentSink->AddLine(D2D1::Point2F(ptEnd.x, 0.f));
      pSegmentSink->EndFigure(D2D1_FIGURE_END_CLOSED);
      pSegmentSink->Close();

      float fSegmentArea = 1.f;
      hr = pSegmentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fSegmentArea);
      if (FAILED(hr))
      {
         ASSERT(FALSE);
      }

      /*ComPtr<ID2D1RectangleGeometry> pCurrentGeometry = nullptr;
      hr = pD2DFactory->CreateRectangleGeometry(D2D1::RectF(0.f, 0.f, fPercent - pPrevItem->GetTimePercentValue(), SPEED_MULTIPLE_MAX_VALUE), &pCurrentGeometry);*/
      ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
      hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
      ComPtr<ID2D1GeometrySink> pCurrentGeometrySink = nullptr;
      hr = pCurrentGeometry->Open(&pCurrentGeometrySink);

      pCurrentGeometrySink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
      pCurrentGeometrySink->AddLine(D2D1::Point2F(0.f, SPEED_MULTIPLE_MAX_VALUE));
      pCurrentGeometrySink->AddLine(D2D1::Point2F(fPercent - pPrevItem->GetTimePercentValue(), SPEED_MULTIPLE_MAX_VALUE));
      pCurrentGeometrySink->AddLine(D2D1::Point2F(fPercent - pPrevItem->GetTimePercentValue(), 0.f));
      pCurrentGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
      pCurrentGeometrySink->Close();

      ComPtr<ID2D1PathGeometry> pUnionGeometry = nullptr;
      hr = pD2DFactory->CreatePathGeometry(&pUnionGeometry);
      ComPtr<ID2D1GeometrySink> pUnionGeometrySink = nullptr;
      hr = pUnionGeometry->Open(&pUnionGeometrySink);

      hr = pCurrentGeometry->CombineWithGeometry(pSegmentGeometry.Get(), D2D1_COMBINE_MODE_INTERSECT, NULL, pUnionGeometrySink.Get());
      pUnionGeometrySink->Close();

      float fIntersectArea = 1.f;
      hr = pUnionGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fIntersectArea);

      float fIntersectPercent = fIntersectArea / fSegmentArea;

      float fElementPercent = pPrevItem->GetElementPercentValue();
      fElementPercent += (pItem->GetElementPercentValue() - pPrevItem->GetElementPercentValue()) * fIntersectPercent;

      return fElementPercent;
   }

   return 1.f;
}

float CSpeedDataController::CalculateSeekPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, Microsoft::WRL::ComPtr<ID2D1PathGeometry> pPathGeometry, float const& fDuration,
   const float& fDesiredValue, float fStart, float fEnd)
{
   float fStartValue = fStart;
   float fEndValue = fEnd;
   float fTestValue = fStart + (fEnd - fStart) / 2.f;

   ComPtr<ID2D1PathGeometry> pTestGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pTestGeometry);
   ComPtr<ID2D1GeometrySink> pTestGeometrySink = nullptr;
   hr = pTestGeometry->Open(&pTestGeometrySink);

   pTestGeometrySink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   pTestGeometrySink->AddLine(D2D1::Point2F(0.f, SPEED_MULTIPLE_MAX_VALUE));
   pTestGeometrySink->AddLine(D2D1::Point2F(fTestValue, SPEED_MULTIPLE_MAX_VALUE));
   pTestGeometrySink->AddLine(D2D1::Point2F(fTestValue, 0.f));
   pTestGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
   pTestGeometrySink->Close();

   float fTestArea = 1.f;
   hr = pTestGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fTestArea);

   ComPtr<ID2D1PathGeometry> pUnionGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pUnionGeometry);
   ComPtr<ID2D1GeometrySink> pUnionGeometrySink = nullptr;
   hr = pUnionGeometry->Open(&pUnionGeometrySink);

   hr = pTestGeometry->CombineWithGeometry(pPathGeometry.Get(), D2D1_COMBINE_MODE_INTERSECT, NULL, pUnionGeometrySink.Get());
   pUnionGeometrySink->Close();

   float fIntersetctArea = 1.f;
   hr = pUnionGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fIntersetctArea);

   float fCalculateTime = m_fDuration * fTestValue * fTestValue / fIntersetctArea;

   if (fabsf(fCalculateTime - fDesiredValue) <= TIME_TOLERANCE)
   {
      return fTestValue;
   }

   pTestGeometry = nullptr;
   pTestGeometrySink = nullptr;
   pUnionGeometry = nullptr;
   pUnionGeometrySink = nullptr;

   if (fCalculateTime > fDesiredValue)
   {
      fEndValue = fTestValue;
   }
   else if (fCalculateTime < fDesiredValue)
   {
      fStartValue =  fTestValue;
   }

   if (fabsf(fEndValue - fStartValue) < SEEK_PERCENT_CALCULATE_TOLERANCE)
   {
      return fStartValue + (fEndValue - fStartValue) / 2.f;
   }

   return CalculateSeekPercent(pD2DFactory, pPathGeometry, fDuration, fDesiredValue, fStartValue, fEndValue);
}

float CSpeedDataController::CalculateSeekPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fElementPercent)
{
   ComPtr<ID2D1PathGeometry> pElementGeometry = nullptr;
   HRESULT hr = pD2DFactory->CreatePathGeometry(&pElementGeometry);
   if (FAILED(hr))
   {
      return 0.f;
   }

   ID2D1GeometrySink* pElementGeometrySink = nullptr;
   hr = pElementGeometry->Open(&pElementGeometrySink);
   if (FAILED(hr))
   {
      return 0.f;
   }

   if (FAILED(CreateVirtualElementGeometry(pElementGeometrySink)))
   {
      return 0.f;
   }

   float fWholeArea = 1.f;
   hr = pElementGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fWholeArea);

   if (FAILED(hr))
   {
      return 0.f;
   }

   ComPtr<ID2D1PathGeometry> pTestGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pTestGeometry);
   ComPtr<ID2D1GeometrySink> pTestGeometrySink = nullptr;
   hr = pTestGeometry->Open(&pTestGeometrySink);

   pTestGeometrySink->BeginFigure(D2D1::Point2F(0.f, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   pTestGeometrySink->AddLine(D2D1::Point2F(0.f, SPEED_MULTIPLE_MAX_VALUE));
   pTestGeometrySink->AddLine(D2D1::Point2F(fElementPercent, SPEED_MULTIPLE_MAX_VALUE));
   pTestGeometrySink->AddLine(D2D1::Point2F(fElementPercent, 0.f));
   pTestGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
   pTestGeometrySink->Close();

   float fTestArea = 1.f;
   hr = pTestGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fTestArea);

   ComPtr<ID2D1PathGeometry> pUnionGeometry = nullptr;
   hr = pD2DFactory->CreatePathGeometry(&pUnionGeometry);
   ComPtr<ID2D1GeometrySink> pUnionGeometrySink = nullptr;
   hr = pUnionGeometry->Open(&pUnionGeometrySink);

   hr = pTestGeometry->CombineWithGeometry(pElementGeometry.Get(), D2D1_COMBINE_MODE_INTERSECT, NULL, pUnionGeometrySink.Get());
   pUnionGeometrySink->Close();

   float fIntersetctArea = 0.f;
   hr = pUnionGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fIntersetctArea);

   if (FAILED(hr))
   {
      return 0.f;
   }

   return fIntersetctArea / fWholeArea;
   
   //POSITION pos = m_pDataItemList->GetHeadPosition();

   //while (pos != nullptr)
   //{
   //   CSpeedDataItem* pItem = m_pDataItemList->GetNextItem(pos);
   //   CSpeedDataItem* pPrevItem = pItem->GetPrevItem();

   //   if (pPrevItem == nullptr)
   //   {
   //      continue;
   //   }

   //   float fCurTimePercent = pItem->GetElementPercentValue();
   //   float fPrevTimePercent = pPrevItem->GetElementPercentValue();

   //   if (fElementPercent > fCurTimePercent)
   //   {
   //      continue;
   //   }

   //   if (fabsf(fElementPercent - fCurTimePercent) <= FLOAT_TYPE_TOLERANCE)
   //   {
   //      return pItem->GetTimePercentValue();
   //   }

   //   if (fabsf(fElementPercent - fPrevTimePercent) <= FLOAT_TYPE_TOLERANCE)
   //   {
   //      return pPrevItem->GetTimePercentValue();
   //   }

   //   ComPtr<ID2D1PathGeometry> pCurrentGeometry = nullptr;
   //   HRESULT hr = pD2DFactory->CreatePathGeometry(&pCurrentGeometry);
   //   if (FAILED(hr))
   //   {
   //      break;
   //   }

   //   ID2D1GeometrySink* pCurrentSink = nullptr;
   //   hr = pCurrentGeometry->Open(&pCurrentSink);
   //   if (FAILED(hr))
   //   {
   //      break;
   //   }

   //   D2D1_POINT_2F ptStart = D2D1::Point2F(0.f/*pPrevItem->GetTimePercentValue()*/, pPrevItem->GetRightSpeedValue());
   //   D2D1_POINT_2F ptEnd = D2D1::Point2F(pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue(), pItem->GetLeftSpeedValue());
   //   float fDistance = (ptEnd.x - ptStart.x) / 2.f;

   //   D2D1_POINT_2F ptFirstContrl = ptStart;
   //   ptFirstContrl.x += fDistance;

   //   D2D1_POINT_2F ptSecondContrl = ptEnd;
   //   ptSecondContrl.x -= fDistance;

   //   pCurrentSink->BeginFigure(D2D1::Point2F(ptStart.x, 0.f), D2D1_FIGURE_BEGIN_FILLED);
   //   pCurrentSink->AddLine(ptStart);
   //   pCurrentSink->AddBezier(D2D1::BezierSegment(ptFirstContrl, ptSecondContrl, ptEnd));
   //   pCurrentSink->AddLine(D2D1::Point2F(ptEnd.x, 0.f));
   //   pCurrentSink->EndFigure(D2D1_FIGURE_END_CLOSED);
   //   pCurrentSink->Close();

   //   float fCurrentSegmentArea = 1.f;
   //   hr = pCurrentGeometry->ComputeArea(D2D1::Matrix3x2F::Identity(), &fCurrentSegmentArea);
   //   if (FAILED(hr))
   //   {
   //      break;
   //   }

   //   float fSeekPercent = fElementPercent - fPrevTimePercent;
   //   float fSeekTime = m_fCurrentDuration * fSeekPercent;

   //   float fCurrentDuration = m_fDuration * (pItem->GetTimePercentValue() - pPrevItem->GetTimePercentValue());

   //   float fCalculatePercent = CalculateSeekPercent(pD2DFactory, pCurrentGeometry, fCurrentDuration, fSeekTime, 0.f, ptEnd.x);

   //   return pPrevItem->GetTimePercentValue() + fCalculatePercent;
   //}

   //return 1.f;
}
