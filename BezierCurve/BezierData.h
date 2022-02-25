#pragma once

class CBezierPoint;

class CBezierData
{
public:
   CBezierData();
   ~CBezierData();

   void SetPointRadius(const float fRadius);

   void SetBezierStartPoint(const CPoint point);

   void UpdateBezierPoint(const CRect rcClient);

   BOOL LButtonDown(const CPoint point);
   void MouseMove(const CPoint point);
   void LButtonUp();

   void ResetPointState();

   void CreateBezierGeometry(ID2D1GeometrySink* pSink);

   void DrawBezierPoints(Microsoft::WRL::ComPtr<ID2D1DeviceContext> pContext, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillBrush, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pOutlineBrush);

   void DrawCalculateBezierPoints(Microsoft::WRL::ComPtr<ID2D1DeviceContext> pContext, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillBrush);

private:
   void CalculatePoints();

   D2D1_POINT_2F                      m_ptStartPoint;
   std::vector<CBezierPoint*>         m_vtPoints;

   std::vector<D2D1_POINT_2F>         m_vtCalculatePoints;

   float m_fPointRadius;

   int m_nSelectIndex;
};
