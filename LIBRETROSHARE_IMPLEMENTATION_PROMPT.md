# Libretroshare Wiki - Remaining Backend Work

**Repository:** https://github.com/samuel-asleep/libretroshare  
**Current Commit:** ed77137e05332398ce37e7906ba9dc890a783dff  
**Related Issue:** RetroShare/RetroShare#3107

---

## Status Summary

### ✅ Completed Backend Work
- **Todo 6:** Complete moderator management system (commit 6f69b681)
- **Todo 3:** Content fetching APIs for merge operations (commit daedbe63)
- **Todo 8:** All CRUD operations and blocking APIs

### ⚠️ Remaining Backend Work
- **Todo 2:** Specific event codes (only remaining backend task)

---

## Todo 2: Add Specific Wiki Event Codes

**Current State:**  
Only generic event codes exist in `src/retroshare/rswiki.h` (lines 68-72)

**Required:** Add 4 new event codes to distinguish NEW from UPDATED events and add SUBSCRIBE_STATUS_CHANGED and NEW_COMMENT events.

### Implementation Required

**File:** `src/retroshare/rswiki.h` (lines 68-72)  
**Change:** Expand RsWikiEventCode enum from 2 to 6 codes

**File:** `src/services/p3wiki.cc` (lines 59-81)  
**Change:** Update notifyChanges() to classify event types (NEW vs UPDATED, comments vs snapshots)

### Event Codes Needed

```
NEW_SNAPSHOT (0x03) - First-time page creation
NEW_COLLECTION (0x04) - New wiki group creation  
SUBSCRIBE_STATUS_CHANGED (0x05) - User subscribed/unsubscribed
NEW_COMMENT (0x06) - New comment added
```

---

## Implementation Estimate

**Time:** 2-4 hours  
**Complexity:** Medium  
**Files:** 2 files to modify

---

## Note

This is the **only remaining backend work** for issue #3107. All other backend features are complete.
