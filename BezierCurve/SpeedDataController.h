#pragma once

using namespace Microsoft::WRL;

class CSpeedDataItemList;
class CSpeedDataItem;

class CSpeedDataController
{
public:
   CSpeedDataController();
   virtual ~CSpeedDataController();

   static CSpeedDataController& GetInstance();

   void Init(float const& duration);

   void SetDuration(float const& fDuration);
   float GetDuration() const;

   void ResetDataSelectState();

   BOOL SelectDataItem(CPoint point, CRect rcClient);

   BOOL AnyItemSelect();

   void MoveDataItem(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, CPoint point, CRect rcClient);

   BOOL IsAllowToAddItem(float fTimePercent, CRect rcClient);

   void AddItem(float fTimePercent, float fSpeedValue);

   HRESULT CreateSpeedCurveGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedCurvePointLineGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient);
   HRESULT CreateSpeedDataSelectPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient);

   HRESULT CreateElementSpeedDataPointGeometry(ID2D1GeometrySink* pSink, CRect rcClient);

   HRESULT CreateVirtualBaseGeometry(ID2D1GeometrySink* pSink);
   HRESULT CreateVirtualCurrentGeometry(ID2D1GeometrySink* pSink);
   float GetCurrentDuration();

   HRESULT CreateVirtualElementGeometry(ID2D1GeometrySink* pSink);

   float CalculateDecodeTime(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent);
   float GetSeekTime(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent);

   float CalculateSeekPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fElementPercent);
   float CalculateSeekPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, Microsoft::WRL::ComPtr<ID2D1PathGeometry> pPathGeometry, float const& fDuration,
      const float& fDesiredValue, float fStart, float fEnd);

   float CalculateElementPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent);

   void UpdateDynamicValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory);

private:
   float PercentToXValue(float const& fPercent, CRect rcClient);
   float XValueToPercent(int nXValue, CRect rcClient);

   float SpeedValueToYValue(float const& fSpeed, CRect rcClient);
   float YValueToSpeedValue(int nYValue, CRect rcClient);

   float GetCurveAreaValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory);

   void UpdateCurrentDuration(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory);
   void UpdateElementPercentValue(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory);

   float GetRealPercent(Microsoft::WRL::ComPtr<ID2D1Factory1> pD2DFactory, float fPercent);

   float m_fDuration;

   float m_fCurrentDuration;

   CSpeedDataItemList* m_pDataItemList;
};