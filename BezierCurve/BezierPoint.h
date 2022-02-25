#pragma once

using namespace Microsoft::WRL;

struct CBezierPointData
{
   CBezierPointData()
   {

   };

   CBezierPointData(D2D1_POINT_2F& pt, BOOL bSel)
      :point(pt),
      bSelect(bSel)
   {

   }

   BOOL bSelect;
   D2D1_POINT_2F point;
};

class CBezierPoint
{
public:
   CBezierPoint();
   CBezierPoint(D2D1_POINT_2F& ctrlPoint1, D2D1_POINT_2F& knotPoint, D2D1_POINT_2F& ctrlPoint2);
   ~CBezierPoint();

   BOOL PtOnControlPoint1(CPoint point);
   BOOL PtOnKnotPoint(CPoint point);
   BOOL PtOnControlPoint2(CPoint point);

   void SelectControlPoint1(BOOL bSelect);
   void SelectKnotPoint(BOOL bSelect);
   void SelectControlPoint2(BOOL bSelect);

   void ResetPointState();

   void MovePoint(CPoint point);

   void SetPointRadius(const float fRadius);
   float GetPointRadius() const;

   D2D1_POINT_2F GetControlPoint1() const;
   D2D1_POINT_2F GetKnotPoint() const;
   D2D1_POINT_2F GetControlPoint2() const;

   void DrawPoints(ComPtr<ID2D1DeviceContext> pContext, ComPtr<ID2D1SolidColorBrush> pFillBrush, ComPtr<ID2D1SolidColorBrush> pOutlineBrush);

private:
   BOOL PtInD2DRect(const CPoint point, D2D1_RECT_F& rcD2D);

private:
   CBezierPointData          m_ptControlPoint1;
   CBezierPointData          m_ptKnotPoint;
   CBezierPointData          m_ptControlPoint2;

   float m_fPointRadius;
};
