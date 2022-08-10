#pragma once

class CSpeedDataItem
{
public:
   CSpeedDataItem();
   CSpeedDataItem(float fTimePercent, float fLeftValue, float fRightValue);
   virtual ~CSpeedDataItem();

   void SetTimePercentValue(float fTimePercent);
   float GetTimePercentValue();

   void SetLeftSpeedValue(float fSpeedValue);
   float GetLeftSpeedValue();

   void SetRightSpeedValue(float fSpeedValue);
   float GetRightSpeedValue();

   void SetPrevItem(CSpeedDataItem* pPrevItem);
   CSpeedDataItem* GetPrevItem();

   void SetNextItem(CSpeedDataItem* pNextItem);
   CSpeedDataItem* GetNextItem();

   void SelectLeftItem(BOOL bSelect);
   BOOL IsLeftItemSelect();

   void SelectRightItem(BOOL bSelect);
   BOOL IsRightItemSelect();

private:
   BOOL m_bLeftSelect;
   BOOL m_bRightSelect;

   CSpeedDataItem* m_pPrevItem;
   CSpeedDataItem* m_pNextItem;

   float m_fTimePercentValue = 0.f;

   float m_fLeftSpeedValue = 1.f;
   float m_fRightSpeedValue = 1.f;
};

class CSpeedDataItemController
{
public:
   CSpeedDataItemController();
   virtual ~CSpeedDataItemController();

   int GetItemCount();

   void AddItem(CSpeedDataItem* pItem, CSpeedDataItem* pPrevItem = nullptr);
   void AddItemBefore(CSpeedDataItem* pItem, CSpeedDataItem* pNexItem);

   void RemoveItem(CSpeedDataItem* pItem);
   void RemoveAll();

   CSpeedDataItem* GetFistItem() const;
   CSpeedDataItem* GetLastItem() const;

   POSITION GetHeadPosition() const;
   CSpeedDataItem* GetNextItem(POSITION& position) const;

private:
   CSpeedDataItem* m_pFirstItem;
   CSpeedDataItem* m_pLastItem;

   int m_nItemCount;
};
