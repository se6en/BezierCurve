#pragma once
using namespace Microsoft::WRL;

class SpecializedSink : public ID2D1SimplifiedGeometrySink
{
public:
   SpecializedSink();

   STDMETHOD_(ULONG, AddRef)(THIS)
   {
      return InterlockedIncrement(reinterpret_cast<LONG volatile*>(&m_cRef));
   }

   STDMETHOD_(ULONG, Release)(THIS)
   {
      ULONG cRef = static_cast<ULONG>(
         InterlockedDecrement(reinterpret_cast<LONG volatile*>(&m_cRef)));

      if (0 == cRef)
      {
         delete this;
      }

      return cRef;
   }

   STDMETHOD(QueryInterface)(THIS_ REFIID iid, void** ppvObject)
   {
      HRESULT hr = S_OK;

      if (__uuidof(IUnknown) == iid)
      {
         *ppvObject = static_cast<IUnknown*>(this);
         AddRef();
      }
      else if (__uuidof(ID2D1SimplifiedGeometrySink) == iid)
      {
         *ppvObject = static_cast<ID2D1SimplifiedGeometrySink*>(this);
         AddRef();
      }
      else
      {
         *ppvObject = NULL;
         hr = E_NOINTERFACE;
      }

      return hr;
   }

   STDMETHOD_(void, AddBeziers)(const D2D1_BEZIER_SEGMENT* beziers,
      UINT beziersCount)
   {
      OutputDebugString(_T("\n SpecializedSink AddBeziers with: \n"));
      for (UINT i = 0; i < beziersCount; i++)
      {
         CString strPoints;
         strPoints.Format(_T("\n point1: %.0f, %.0f; point2:  %.0f, %.0f; point3:  %.0f, %.0f \n"), beziers[i].point1.x, beziers[i].point1.y, beziers[i].point2.x, beziers[i].point2.y, beziers[i].point3.x, beziers[i].point3.y);
         OutputDebugString(strPoints);
      }
   }

   STDMETHOD_(void, AddLines)(const D2D1_POINT_2F* points, UINT pointsCount)
   {
      OutputDebugString(_T("\n SpecializedSink AddLines with: \n"));
      for (UINT i = 0; i < pointsCount; i++)
      {
         m_vecPoints.emplace_back(points[i]);

         CString strPoints;
         strPoints.Format(_T("\n point: %.0f, %.0f \n"), points[i].x, points[i].y);
         OutputDebugString(strPoints);
      }
      // Customize this method to meet your specific data needs.
   }

   STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint,
      D2D1_FIGURE_BEGIN figureBegin)
   {
      m_vecPoints.emplace_back(startPoint);

      OutputDebugString(_T("\n SpecializedSink BeginFigure with: \n"));

      CString strPoints;
      strPoints.Format(_T("\n point: %.0f, %.0f \n"), startPoint.x, startPoint.y);
      OutputDebugString(strPoints);
   }

   STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd)
   {
   }

   STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode)
   {
   }

   STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags)
   {
   }

   STDMETHOD(Close)()
   {
      return S_OK;
   }

   size_t GetIntersectPointsCount()
   {
      return m_vecPoints.size();
   }

   std::vector<D2D1_POINT_2F>* GetIntersectPoins()
   {
      return &m_vecPoints;
   }

private:
   UINT m_cRef;

   std::vector<D2D1_POINT_2F> m_vecPoints;
};
