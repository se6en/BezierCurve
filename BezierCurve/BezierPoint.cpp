#include "pch.h"
#include "BezierPoint.h"

CBezierPoint::CBezierPoint()
{
}

CBezierPoint::CBezierPoint(D2D1_POINT_2F& ctrlPoint1, D2D1_POINT_2F& knotPoint, D2D1_POINT_2F& ctrlPoint2)
{
   m_ptControlPoint1.bSelect = FALSE;
   m_ptControlPoint1.point = ctrlPoint1;

   m_ptKnotPoint.bSelect = FALSE;
   m_ptKnotPoint.point = knotPoint;

   m_ptControlPoint2.bSelect = FALSE;
   m_ptControlPoint2.point = ctrlPoint2;
}

CBezierPoint::~CBezierPoint()
{

}

BOOL CBezierPoint::PtInD2DRect(const CPoint point, D2D1_RECT_F& rcD2D)
{
   D2D1_POINT_2F ptD2D = D2D1::Point2F(static_cast<float>(point.x), static_cast<float>(point.y));

   return (ptD2D.x >= rcD2D.left && ptD2D.x <= rcD2D.right && ptD2D.y >= rcD2D.top && ptD2D.y <= rcD2D.bottom);
}

BOOL CBezierPoint::PtOnControlPoint1(CPoint point)
{
   D2D1_RECT_F rcPoint = D2D1::RectF(m_ptControlPoint1.point.x - m_fPointRadius, m_ptControlPoint1.point.y - m_fPointRadius,
      m_ptControlPoint1.point.x + m_fPointRadius, m_ptControlPoint1.point.y + m_fPointRadius);

   return PtInD2DRect(point, rcPoint);
}

BOOL CBezierPoint::PtOnKnotPoint(CPoint point)
{
   D2D1_RECT_F rcPoint = D2D1::RectF(m_ptKnotPoint.point.x - m_fPointRadius, m_ptKnotPoint.point.y - m_fPointRadius,
      m_ptKnotPoint.point.x + m_fPointRadius, m_ptKnotPoint.point.y + m_fPointRadius);

   return PtInD2DRect(point, rcPoint);
}

BOOL CBezierPoint::PtOnControlPoint2(CPoint point)
{
   D2D1_RECT_F rcPoint = D2D1::RectF(m_ptControlPoint2.point.x - m_fPointRadius, m_ptControlPoint2.point.y - m_fPointRadius,
      m_ptControlPoint2.point.x + m_fPointRadius, m_ptControlPoint2.point.y + m_fPointRadius);

   return PtInD2DRect(point, rcPoint);
}

void CBezierPoint::SelectControlPoint1(BOOL bSelect)
{
   m_ptControlPoint1.bSelect = bSelect;
}

void CBezierPoint::SelectKnotPoint(BOOL bSelect)
{
   m_ptKnotPoint.bSelect = bSelect;
}

void CBezierPoint::SelectControlPoint2(BOOL bSelect)
{
   m_ptControlPoint2.bSelect = bSelect;
}

void CBezierPoint::ResetPointState()
{
   m_ptControlPoint1.bSelect = FALSE;
   m_ptKnotPoint.bSelect = FALSE;
   m_ptControlPoint2.bSelect = FALSE;
}

void CBezierPoint::MovePoint(CPoint point)
{
   if (m_ptControlPoint1.bSelect)
   {
      m_ptControlPoint1.point = D2D1::Point2F(static_cast<float>(point.x), static_cast<float>(point.y));
      return;
   }

   if (m_ptKnotPoint.bSelect)
   {
      m_ptKnotPoint.point = D2D1::Point2F(static_cast<float>(point.x), static_cast<float>(point.y));
      return;
   }

   if (m_ptControlPoint2.bSelect)
   {
      m_ptControlPoint2.point = D2D1::Point2F(static_cast<float>(point.x), static_cast<float>(point.y));
      return;
   }
}

void CBezierPoint::SetPointRadius(const float fRadius)
{
   m_fPointRadius = fRadius;
}

float CBezierPoint::GetPointRadius() const
{
   return m_fPointRadius;
}

D2D1_POINT_2F CBezierPoint::GetControlPoint1() const
{
   return m_ptControlPoint1.point;
}

D2D1_POINT_2F CBezierPoint::GetKnotPoint() const
{
   return m_ptKnotPoint.point;
}

D2D1_POINT_2F CBezierPoint::GetControlPoint2() const
{
   return m_ptControlPoint2.point;
}

void CBezierPoint::DrawPoints(ComPtr<ID2D1DeviceContext> pContext, ComPtr<ID2D1SolidColorBrush> pFillBrush, ComPtr<ID2D1SolidColorBrush> pOutlineBrush)
{
   pContext->FillEllipse(D2D1::Ellipse(m_ptControlPoint1.point, m_fPointRadius, m_fPointRadius), pFillBrush.Get());

   if (m_ptControlPoint1.bSelect)
   {
      pContext->DrawEllipse(D2D1::Ellipse(m_ptControlPoint1.point, m_fPointRadius, m_fPointRadius), pOutlineBrush.Get());
   }

   pContext->FillEllipse(D2D1::Ellipse(m_ptKnotPoint.point, m_fPointRadius, m_fPointRadius), pFillBrush.Get());

   if (m_ptKnotPoint.bSelect)
   {
      pContext->DrawEllipse(D2D1::Ellipse(m_ptKnotPoint.point, m_fPointRadius, m_fPointRadius), pOutlineBrush.Get());
   }

   pContext->FillEllipse(D2D1::Ellipse(m_ptControlPoint2.point, m_fPointRadius, m_fPointRadius), pFillBrush.Get());

   if (m_ptControlPoint2.bSelect)
   {
      pContext->DrawEllipse(D2D1::Ellipse(m_ptControlPoint2.point, m_fPointRadius, m_fPointRadius), pOutlineBrush.Get());
   }


}