#include "pch.h"
#include "BezierPoint.h"
#include "BezierData.h"

static constexpr float DEFAULT_POINT_DADIUS = 5.f;

CBezierData::CBezierData()
   :m_nSelectIndex(-1),
   m_fPointRadius(DEFAULT_POINT_DADIUS)
{

}

CBezierData::~CBezierData()
{
   m_vtPoints.clear();
   m_vtCalculatePoints.clear();
}

void CBezierData::SetPointRadius(const float fRadius)
{
   m_fPointRadius = fRadius;
}

void CBezierData::SetBezierStartPoint(const CPoint point)
{
   m_ptStartPoint = D2D1::Point2F(static_cast<float>(point.x), static_cast<float>(point.y));
}

void CBezierData::UpdateBezierPoint(const CRect rcClient)
{
   m_vtPoints.clear();
   CBezierPoint* point = nullptr;

   D2D1_POINT_2F ptControlPrevRight, ptKnot, ptControlCurLeft;
   // first point
   ptControlPrevRight = D2D1::Point2F(static_cast<float>(rcClient.Width() / 8), static_cast<float>(3 * rcClient.Height() / 8));
   ptKnot = D2D1::Point2F(static_cast<float>(rcClient.Width() / 4), static_cast<float>(rcClient.Height() / 4));
   ptControlCurLeft = D2D1::Point2F(static_cast<float>(rcClient.Width() / 8), static_cast<float>(rcClient.Height() / 4));

   point = new CBezierPoint(ptControlPrevRight, ptKnot, ptControlCurLeft);
   point->SetPointRadius(m_fPointRadius);

   m_vtPoints.emplace_back(point);

   // second point
   ptControlPrevRight = D2D1::Point2F(static_cast<float>(3 * rcClient.Width() / 8), static_cast<float>(rcClient.Height() / 4));
   ptKnot = D2D1::Point2F(static_cast<float>(rcClient.Width() / 2), static_cast<float>(rcClient.Height() / 2));
   ptControlCurLeft = D2D1::Point2F(static_cast<float>(3 * rcClient.Width() / 8), static_cast<float>(3 * rcClient.Height() / 8));

   point = new CBezierPoint(ptControlPrevRight, ptKnot, ptControlCurLeft);
   point->SetPointRadius(m_fPointRadius);

   m_vtPoints.emplace_back(point);

   // third point
   ptControlPrevRight = D2D1::Point2F(static_cast<float>(5 * rcClient.Width() / 8), static_cast<float>(5 * rcClient.Height() / 8));
   ptKnot = D2D1::Point2F(static_cast<float>(3 * rcClient.Width() / 4), static_cast<float>(3 * rcClient.Height() / 4));
   ptControlCurLeft = D2D1::Point2F(static_cast<float>(5 * rcClient.Width() / 8), static_cast<float>(3 * rcClient.Height() / 4));

   point = new CBezierPoint(ptControlPrevRight, ptKnot, ptControlCurLeft);
   point->SetPointRadius(m_fPointRadius);

   m_vtPoints.emplace_back(point);

   // forth point
   ptControlPrevRight = D2D1::Point2F(static_cast<float>(7 * rcClient.Width() / 8), static_cast<float>(5 * rcClient.Height() / 8));
   ptKnot = D2D1::Point2F(static_cast<float>(rcClient.Width()), static_cast<float>(rcClient.Height() / 2));
   ptControlCurLeft = D2D1::Point2F(static_cast<float>(7 * rcClient.Width() / 8), static_cast<float>(rcClient.Height() / 2));

   point = new CBezierPoint(ptControlPrevRight, ptKnot, ptControlCurLeft);
   point->SetPointRadius(m_fPointRadius);

   m_vtPoints.emplace_back(point);

   CalculatePoints();
}

BOOL CBezierData::LButtonDown(const CPoint point)
{
   for (size_t i = 0; i < m_vtPoints.size(); i++)
   {
      if (m_vtPoints[i]->PtOnControlPoint1(point))
      {
         m_vtPoints[i]->SelectControlPoint1(TRUE);
         m_nSelectIndex = static_cast<int>(i);
         return TRUE;
      }

      if (m_vtPoints[i]->PtOnKnotPoint(point))
      {
         m_vtPoints[i]->SelectKnotPoint(TRUE);
         m_nSelectIndex = static_cast<int>(i);
         return TRUE;
      }

      if (m_vtPoints[i]->PtOnControlPoint2(point))
      {
         m_vtPoints[i]->SelectControlPoint2(TRUE);
         m_nSelectIndex = static_cast<int>(i);
         return TRUE;
      }
   }

   return FALSE;
}

void CBezierData::MouseMove(const CPoint point)
{
   if (m_nSelectIndex == -1)
   {
      return;
   }

   m_vtPoints[m_nSelectIndex]->MovePoint(point);

   CalculatePoints();
}

void CBezierData::LButtonUp()
{
   /*if (m_nSelectIndex != -1)
   {
      m_vtPoints[m_nSelectIndex]->ResetPointState();
   }*/
}

void CBezierData::ResetPointState()
{
   if (m_nSelectIndex != -1)
   {
      m_vtPoints[m_nSelectIndex]->ResetPointState();
   }
}

void CBezierData::CreateBezierGeometry(ID2D1GeometrySink* pSink)
{
   if (pSink == nullptr)
   {
      return;
   }

   //pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

   pSink->BeginFigure(m_ptStartPoint, /*D2D1_FIGURE_BEGIN_HOLLOW*/D2D1_FIGURE_BEGIN_FILLED);

   for (auto pPointData : m_vtPoints)
   {
      pSink->AddBezier(D2D1::BezierSegment(pPointData->GetControlPoint1(), pPointData->GetControlPoint2(), pPointData->GetKnotPoint()));
   }

   //pSink->AddLine(m_ptStartPoint);

   pSink->EndFigure(D2D1_FIGURE_END_OPEN);

   pSink->Close();
}

void CBezierData::DrawBezierPoints(ComPtr<ID2D1DeviceContext> pContext, ComPtr<ID2D1SolidColorBrush> pFillBrush, ComPtr<ID2D1SolidColorBrush> pOutlineBrush)
{
   //// draw first line
   //if (!m_vtPoints.empty())
   //{
   //   pContext->DrawLine(m_ptStartPoint, m_vtPoints[0]->GetControlPoint1(), pOutlineBrush.Get());
   //}

   // draw start point
   pContext->FillEllipse(D2D1::Ellipse(m_ptStartPoint, m_fPointRadius, m_fPointRadius), pFillBrush.Get());

   // draw lines
   D2D1_POINT_2F ptPrev = m_ptStartPoint;

   for (size_t i = 0; i < m_vtPoints.size(); i++)
   {
      pContext->DrawLine(ptPrev, m_vtPoints[i]->GetControlPoint1(), pOutlineBrush.Get(), 2.0F);
      pContext->DrawLine(m_vtPoints[i]->GetKnotPoint(), m_vtPoints[i]->GetControlPoint2(), pOutlineBrush.Get(), 2.0F);

      ptPrev = m_vtPoints[i]->GetKnotPoint();
   }

   for (auto pPointData : m_vtPoints)
   {
      pPointData->DrawPoints(pContext, pFillBrush, pOutlineBrush);
   }
}

void CBezierData::CalculatePoints()
{
   if (m_vtPoints.empty())
   {
      return;
   }

   m_vtCalculatePoints.clear();
   // calculate points based on P_t = (1-t)^3 * P_0 + 3 * t * (1-t)^2 * P_1 + 3 * t^2 * (1-t) * P_2 + t^3 * P_3
   D2D1_POINT_2F pt0 = m_ptStartPoint;
   D2D1_POINT_2F pt1 = m_vtPoints[0]->GetControlPoint1();
   D2D1_POINT_2F pt2 = m_vtPoints[0]->GetControlPoint2();
   D2D1_POINT_2F pt3 = m_vtPoints[0]->GetKnotPoint();

   for (float fValue = 0.f; fValue < 1.f;)
   {
      D2D1_POINT_2F ptCalculate;
      ptCalculate.x = powf((1.f - fValue), 3.f) * pt0.x + 3 * fValue * powf((1.f - fValue), 2.f) * pt1.x + 3 * powf(fValue, 2.f) * (1 - fValue) * pt2.x + powf(fValue, 3.f) * pt3.x;
      ptCalculate.y = powf((1.f - fValue), 3.f) * pt0.y + 3 * fValue * powf((1.f - fValue), 2.f) * pt1.y + 3 * powf(fValue, 2.f) * (1 - fValue) * pt2.y + powf(fValue, 3.f) * pt3.y;

      m_vtCalculatePoints.emplace_back(ptCalculate);

      fValue += 0.05f;
   }
}

void CBezierData::DrawCalculateBezierPoints(Microsoft::WRL::ComPtr<ID2D1DeviceContext> pContext, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillBrush)
{
   if (m_vtCalculatePoints.empty())
   {
      return;
   }

   for (auto point : m_vtCalculatePoints)
   {
      pContext->FillEllipse(D2D1::Ellipse(point, m_fPointRadius, m_fPointRadius), pFillBrush.Get());
   }
}