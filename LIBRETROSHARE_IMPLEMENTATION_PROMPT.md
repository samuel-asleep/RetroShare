# Libretroshare Wiki Implementation - Remaining Features

**Repository:** https://github.com/samuel-asleep/libretroshare  
**Current Commit:** ed77137e05332398ce37e7906ba9dc890a783dff  
**Related Issue:** RetroShare/RetroShare#3107

---

## Overview

This document specifies the remaining backend implementation needed in the libretroshare repository to complete issue #3107 "Wiki Todos V2". The samuel-asleep/libretroshare fork is correctly configured in .gitmodules.

**Recent Progress:**
- ✅ **Todo 6 Backend Complete** (commit 6f69b681): Moderator management fully implemented
- ✅ **Todo 3 Backend APIs** (commit daedbe63): Content fetching APIs for merge operations added
- ⚠️ **Todo 2 Incomplete**: Specific event codes still missing

---

## Required Implementation: Specific Wiki Event Codes (Todo 2)

**Status:** ⚠️ **INCOMPLETE** - Only generic UPDATED_* events exist

### Current State
The event system exists but only has generic codes:
- `UPDATED_SNAPSHOT` (0x01)
- `UPDATED_COLLECTION` (0x02)

**File:** `src/retroshare/rswiki.h` (lines 68-72)

**What's Already Done:**
- ✅ Event structure `RsGxsWikiEvent` properly extends `RsEvent`
- ✅ Event emission working via `rsEvents->postEvent()` in `p3wiki.cc`
- ✅ Basic event dispatching to GUI

### Required Changes

#### 1.1 Update Event Enum in `src/retroshare/rswiki.h`

Expand the `RsWikiEventCode` enum to include:

```cpp
enum class RsWikiEventCode : uint8_t
{
    UPDATED_SNAPSHOT          = 0x01,
    UPDATED_COLLECTION        = 0x02,
    NEW_SNAPSHOT              = 0x03,  // NEW
    NEW_COLLECTION            = 0x04,  // NEW
    SUBSCRIBE_STATUS_CHANGED  = 0x05,  // NEW
    NEW_COMMENT               = 0x06   // NEW
};
```

#### 1.2 Update Event Emission Logic in `src/services/p3wiki.cc`

Modify the `notifyChanges()` method (currently lines 59-81) to:

1. **Distinguish between NEW and UPDATED messages:**
   - Check if the message/group already existed before this change
   - Use `NEW_SNAPSHOT` for first-time page creation
   - Use `UPDATED_SNAPSHOT` for page edits
   - Use `NEW_COLLECTION` for new wiki group creation
   - Use `UPDATED_COLLECTION` for wiki group updates

2. **Add logic for NEW_COMMENT events:**
   - Detect when a comment is added vs updated
   - Emit `NEW_COMMENT` event with appropriate group and message IDs

3. **Add SUBSCRIBE_STATUS_CHANGED events:**
   - Hook into subscription change notifications
   - Emit event when user subscribes/unsubscribes to a wiki group

**Example implementation approach:**
```cpp
void p3Wiki::notifyChanges(std::vector<RsGxsNotify*>& changes)
{
    if (rsEvents) {
        RsEventType wikiEventType = (RsEventType)rsEvents->getDynamicEventType("GXS_WIKI");
        
        for(auto change : changes) {
            std::shared_ptr<RsGxsWikiEvent> event = std::make_shared<RsGxsWikiEvent>(wikiEventType);
            event->mWikiGroupId = change->mGroupId;
            
            if (auto msgChange = dynamic_cast<RsGxsMsgChange*>(change)) {
                // Determine if new or updated
                // Check msgChange->msgChangeMap to see if message IDs are new
                
                if (isNewMessage(msgChange)) {
                    // Check message type
                    if (isCommentMessage(msgChange)) {
                        event->mWikiEventCode = RsWikiEventCode::NEW_COMMENT;
                    } else {
                        event->mWikiEventCode = RsWikiEventCode::NEW_SNAPSHOT;
                    }
                } else {
                    event->mWikiEventCode = RsWikiEventCode::UPDATED_SNAPSHOT;
                }
            } else if (auto grpChange = dynamic_cast<RsGxsGroupChange*>(change)) {
                if (isNewGroup(grpChange)) {
                    event->mWikiEventCode = RsWikiEventCode::NEW_COLLECTION;
                } else {
                    event->mWikiEventCode = RsWikiEventCode::UPDATED_COLLECTION;
                }
            }
            
            rsEvents->postEvent(event);
            delete change;
        }
    } else {
        for(auto change : changes) delete change;
    }
    changes.clear();
}
```

**Note:** You'll need to implement helper methods like `isNewMessage()`, `isCommentMessage()`, and `isNewGroup()` to check the change type properly.

---

## ✅ COMPLETED: Moderator System (Todo 6)

**Status:** ✅ **IMPLEMENTED** in commit 6f69b681

### What Was Implemented

The complete moderator system is now functional in libretroshare:

#### API Methods (src/retroshare/rswiki.h, lines 144-176)
- ✅ `addModerator(grpId, moderatorId)` - Add moderator to wiki collection
- ✅ `removeModerator(grpId, moderatorId)` - Remove moderator from wiki collection  
- ✅ `getModerators(grpId, moderators)` - Get list of moderators
- ✅ `isActiveModerator(grpId, authorId, editTime)` - Check moderator status

#### Implementation (src/services/p3wiki.cc/h)
- ✅ All moderator methods implemented in p3Wiki class
- ✅ Network-wide message validation logic
- ✅ Self-edit permission + moderator permission checking
- ✅ Moderator list serialization in RsGxsWikiCollectionItem

#### What Remains
- ❌ GUI integration in WikiGroupDialog (RetroShare main repo, not libretroshare)
- ❌ Add/remove moderator UI buttons
- ❌ Moderator list display widget

**Note:** The backend work for Todo 6 is complete. Only GUI work remains in the main RetroShare repository.

---

## ✅ COMPLETED: Content Fetching APIs (Todo 3 Backend)

**Status:** ✅ **IMPLEMENTED** in commit daedbe63

### What Was Implemented

Content fetching APIs for merge operations:

#### API Methods (src/retroshare/rswiki.h, lines 178-195)
- ✅ `getSnapshotContent(snapshotId, content)` - Fetch single page content
- ✅ `getSnapshotsContent(snapshotIds, contents)` - Bulk fetch multiple pages

#### Implementation (src/services/p3wiki.cc/h)
- ✅ Token-based async requests implemented
- ✅ Efficient bulk fetching for merge operations
- ✅ Returns content mapped by snapshot message ID

#### What Remains
- ❌ GUI integration in WikiEditDialog to use these APIs (RetroShare main repo)
- ❌ Actual diff/merge algorithms in GUI
- ❌ Merge UI logic to fetch and combine content

**Note:** The backend support for Todo 3 is ready. GUI needs to integrate these APIs for actual merging.

---

## Required Implementation #2: Moderator System (Todo 6)

**Status:** ❌ **MOVED TO GUI** - Backend complete, see above

This section is no longer needed as the moderator backend is fully implemented. See "COMPLETED: Moderator System" section above.
### Integration Tests
1. Create wiki group and verify NEW_COLLECTION event
2. Add page and verify NEW_SNAPSHOT event
3. Edit page and verify UPDATED_SNAPSHOT event
4. Add comment and verify NEW_COMMENT event
5. Subscribe/unsubscribe and verify SUBSCRIBE_STATUS_CHANGED event
6. Add moderator and verify they can edit any page
7. Remove moderator and verify they can only self-edit
8. Verify non-moderators can create edits but not republish

### Network Tests
1. Verify moderator validation works across network peers
2. Verify terminated moderators are kept in history
3. Verify backward compatibility with old wiki groups

---

## Implementation Order

**Phase 1:** Event codes (Todo 2) - THE ONLY REMAINING BACKEND WORK
   - Update enum in rswiki.h
   - Implement event classification in p3wiki.cc
   - Test event emission

**Phase 2-5:** ALREADY COMPLETE
   - ✅ Moderator data structures (Todo 6) - Done in commit 6f69b681
   - ✅ Moderator API (Todo 6) - Done in commit 6f69b681
   - ✅ Validation logic (Todo 6) - Done in commit 6f69b681
   - ✅ Content fetching APIs (Todo 3) - Done in commit daedbe63

---

## File Summary

**Files to modify for Todo 2 (only remaining backend work):**
1. `src/retroshare/rswiki.h` - Add 4 new event codes to enum
2. `src/services/p3wiki.cc` - Update notifyChanges() to classify events

**Files already modified (complete):**
- ✅ `src/retroshare/rswiki.h` - Moderator API, content fetching API
- ✅ `src/rsitems/rswikiitems.cc` - Moderator serialization
- ✅ `src/services/p3wiki.h` - Moderator and content API declarations  
- ✅ `src/services/p3wiki.cc` - Full moderator and content API implementation

**Estimated effort for Todo 2:** 2-4 hours for experienced developer

---

## Notes

- **Todo 6 (Moderators):** Complete backend implementation exists in commit 6f69b681
  - Follows forums-style moderator pattern as discussed in issue #3107
  - Network-wide validation ensures consistent moderation
  - Only GUI work remains (WikiGroupDialog in main RetroShare repo)

- **Todo 3 (Content Fetching):** Backend APIs ready in commit daedbe63
  - `getSnapshotContent()` and `getSnapshotsContent()` available
  - GUI needs to integrate these for actual merge operations
  - WikiEditDialog merge logic needs to use the new APIs

- **Todo 2 (Event Codes):** Only backend work remaining
  - Need to distinguish NEW vs UPDATED states
  - Add SUBSCRIBE_STATUS_CHANGED and NEW_COMMENT events
  - Improves UI responsiveness to specific changes

---

**Implementation Target:** samuel-asleep/libretroshare repository  
**Current Status:** 2 of 3 backend todos complete, 1 remaining (event codes)  
**After Event Code Implementation:** All backend work will be complete
**Remaining Work:** GUI integration in main RetroShare repository
