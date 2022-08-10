#pragma once

using namespace Microsoft::WRL;

class CSpeedDataItemController;

class CSpeedData
{
public:
   CSpeedData();
   virtual ~CSpeedData();

   void Init(float const& duration);

   void SetDuration(float const& fDuration);
   float GetDuration() const;

   void ResetDataSelectState();

   BOOL SelectDataItem(CPoint point, CRect rcClient);

   BOOL AnyItemSelect();

   void MoveDataItem(CPoint point, CRect rcClient);

   void AddItem(float fTimePercent, float fSpeedValue);

   HRESULT CreateSpeedCurveGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedCurvePointLineGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedDataSelectPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient);

   HRESULT CreateVirtualBaseGeometry(ID2D1GeometrySink* pSink);
   HRESULT CreateVirtualCurrentGeometry(ID2D1GeometrySink* pSink);
   float GetCurrentDuration(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory);

private:
   float PercentToXValue(float const& fPercent, CRect rcClient);
   float XValueToPercent(int nXValue, CRect rcClient);

   float SpeedValueToYValue(float const& fSpeed, CRect rcClient);
   float YValueToSpeedValue(int nYValue, CRect rcClient);

   float m_fDuration;

   CSpeedDataItemController* m_pDataItemController;
};