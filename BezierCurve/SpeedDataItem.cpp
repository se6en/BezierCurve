#include "pch.h"
#include "SpeedDataItem.h"

CSpeedDataItem::CSpeedDataItem()
{

}

CSpeedDataItem::CSpeedDataItem(float fTimePercent, float fLeftValue, float fRightValue)
{
   m_fTimePercentValue = fTimePercent;
   m_fLeftSpeedValue = fLeftValue;
   m_fRightSpeedValue = fRightValue;
}

CSpeedDataItem::~CSpeedDataItem()
{

}

void CSpeedDataItem::SetTimePercentValue(float fTimePercent)
{
   m_fTimePercentValue = fTimePercent;
}

float CSpeedDataItem::GetTimePercentValue()
{
   return m_fTimePercentValue;
}

void CSpeedDataItem::SetLeftSpeedValue(float fSpeedValue)
{
   m_fLeftSpeedValue = fSpeedValue;
}

float CSpeedDataItem::GetLeftSpeedValue()
{
   return m_fLeftSpeedValue;
}

void CSpeedDataItem::SetRightSpeedValue(float fSpeedValue)
{
   m_fRightSpeedValue = fSpeedValue;
}

float CSpeedDataItem::GetRightSpeedValue()
{
   return m_fRightSpeedValue;
}

void CSpeedDataItem::SetPrevItem(CSpeedDataItem* pPrevItem)
{
   m_pPrevItem = pPrevItem;
}

CSpeedDataItem* CSpeedDataItem::GetPrevItem()
{
   return m_pPrevItem;
}

void CSpeedDataItem::SetNextItem(CSpeedDataItem* pNextItem)
{
   m_pNextItem = pNextItem;
}

CSpeedDataItem* CSpeedDataItem::GetNextItem()
{
   return m_pNextItem;
}

void CSpeedDataItem::SelectLeftItem(BOOL bSelect)
{
   m_bLeftSelect = bSelect;
}

BOOL CSpeedDataItem::IsLeftItemSelect()
{
   return m_bLeftSelect;
}

void CSpeedDataItem::SelectRightItem(BOOL bSelect)
{
   m_bRightSelect = bSelect;
}

BOOL CSpeedDataItem::IsRightItemSelect()
{
   return m_bRightSelect;
}

CSpeedDataItemController::CSpeedDataItemController()
   : m_pFirstItem(nullptr),
   m_pLastItem(nullptr),
   m_nItemCount(0)
{
}

CSpeedDataItemController::~CSpeedDataItemController()
{
   RemoveAll();
}

int CSpeedDataItemController::GetItemCount()
{
   return m_nItemCount;
}

void CSpeedDataItemController::AddItem(CSpeedDataItem* pItem, CSpeedDataItem* pPrevItem)
{
   if (pItem == nullptr)
   {
      return;
   }

   CSpeedDataItem* pAddPrevItem = pPrevItem == nullptr ? m_pLastItem : pPrevItem;

   CSpeedDataItem* pNextItem = pAddPrevItem ? pAddPrevItem->GetNextItem() : nullptr;

   // update previous item and next item pointer
   if (pAddPrevItem != nullptr)
   {
      pAddPrevItem->SetNextItem(pItem);
   }

   if (pNextItem != nullptr)
   {
      pNextItem->SetPrevItem(pItem);
   }

   pItem->SetPrevItem(pAddPrevItem);
   pItem->SetNextItem(pNextItem);

   if (m_pLastItem == pAddPrevItem)
   {
      m_pLastItem = pItem;
   }

   if (m_pFirstItem == nullptr)
   {
      m_pFirstItem = pItem;
   }

   ++m_nItemCount;
}

void CSpeedDataItemController::AddItemBefore(CSpeedDataItem* pItem, CSpeedDataItem* pNextItem)
{
   if (pItem == nullptr)
   {
      return;
   }

   if (pNextItem == nullptr)
   {
      pNextItem = m_pFirstItem;
   }

   CSpeedDataItem* pPrevItem = pNextItem ? pNextItem->GetPrevItem() : nullptr;

   // update previous item and next item pointer
   if (pNextItem != nullptr)
   {
      pNextItem->SetPrevItem(pItem);
   }

   if (pPrevItem != nullptr)
   {
      pPrevItem->SetNextItem(pItem);
   }

   pItem->SetPrevItem(pPrevItem);
   pItem->SetNextItem(pNextItem);

   if (m_pLastItem == nullptr)
   {
      m_pLastItem = pItem;
   }

   if (m_pFirstItem == pNextItem)
   {
      m_pFirstItem = pItem;
   }

   ++m_nItemCount;
}

void CSpeedDataItemController::RemoveItem(CSpeedDataItem* pItem)
{
   if (pItem == nullptr)
   {
      return;
   }

   if (pItem->GetPrevItem())
   {
      pItem->GetPrevItem()->SetNextItem(pItem->GetNextItem());
   }

   if (pItem->GetNextItem())
   {
      pItem->GetNextItem()->SetPrevItem(pItem->GetPrevItem());
   }

   if (m_pFirstItem == pItem)
   {
      m_pFirstItem = pItem->GetNextItem();
   }

   if (m_pLastItem == pItem)
   {
      m_pLastItem = pItem->GetPrevItem();
   }

   m_nItemCount--;

   delete pItem;
}

void CSpeedDataItemController::RemoveAll()
{
   for (CSpeedDataItem* pItem = m_pFirstItem; pItem != nullptr;)
   {
      CSpeedDataItem* pNextItem = pItem->GetNextItem();

      delete pItem;

      pItem = pNextItem;
   }

   m_pFirstItem = nullptr;
   m_pLastItem = nullptr;
}

CSpeedDataItem* CSpeedDataItemController::GetFistItem() const
{
   return m_pFirstItem;
}

CSpeedDataItem* CSpeedDataItemController::GetLastItem() const
{
   return m_pLastItem;
}

POSITION CSpeedDataItemController::GetHeadPosition() const
{
   return (POSITION)m_pFirstItem;
}

CSpeedDataItem* CSpeedDataItemController::GetNextItem(POSITION& position) const
{
   CSpeedDataItem* pItem = (CSpeedDataItem*)position;

   position = (POSITION)pItem->GetNextItem();

   return pItem;
}
