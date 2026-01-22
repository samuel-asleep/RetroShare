# Libretroshare Wiki Implementation - Missing Features

**Repository:** https://github.com/samuel-asleep/libretroshare  
**Current Commit:** aa6a40abe8f4a091e9349eb895daf5cea3e385a5  
**Related Issue:** RetroShare/RetroShare#3107

---

## Overview

This document specifies the missing backend implementations needed in the libretroshare repository to complete issue #3107 "Wiki Todos V2". The samuel-asleep/libretroshare fork is correctly configured in .gitmodules and needs these additions.

---

## Required Implementation #1: Specific Wiki Event Codes (Todo 2)

### Current State
The event system exists but only has generic codes:
- `UPDATED_SNAPSHOT` (0x01)
- `UPDATED_COLLECTION` (0x02)

**File:** `src/retroshare/rswiki.h` (lines 68-72)

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

## Required Implementation #2: Moderator System (Todo 6)

### Current State
- No moderator data structures exist
- No moderator API methods
- No validation logic for moderator permissions
- RsWikiCollection struct is minimal (only has mDescription, mCategory, mHashTags)

**Files to modify:**
- `src/retroshare/rswiki.h` - Add moderator API interface
- `src/rsitems/rswikiitems.h` - Add moderator data structures
- `src/rsitems/rswikiitems.cc` - Add serialization for moderator data
- `src/services/p3wiki.h` - Add moderator method declarations
- `src/services/p3wiki.cc` - Implement moderator logic and validation

### Design Specification

Follow the forums-style moderator pattern as discussed in issue comments:

#### 2.1 Data Structures

**In `src/rsitems/rswikiitems.h`**, add:

```cpp
// Moderator entry with termination date support
struct RsWikiModeratorEntry {
    RsGxsId mModeratorId;           // GXS ID of the moderator
    rstime_t mAddedTime;            // When moderator was added
    rstime_t mTerminationTime;      // 0 = active, >0 = terminated at this time
    
    RsWikiModeratorEntry() : mAddedTime(0), mTerminationTime(0) {}
};

// Add to RsGxsWikiCollectionItem class:
// Inside RsGxsWikiCollectionItem, the RsWikiCollection will need moderator list
```

**In `src/retroshare/rswiki.h`**, update RsWikiCollection:

```cpp
struct RsWikiCollection: RsGxsGenericGroupData
{
    std::string mDescription;
    std::string mCategory;
    std::string mHashTags;
    std::vector<RsWikiModeratorEntry> mModerators;  // NEW: Moderator list
};
```

#### 2.2 Serialization

**In `src/rsitems/rswikiitems.cc`**, update the `serial_process` method for `RsGxsWikiCollectionItem`:

```cpp
void RsGxsWikiCollectionItem::serial_process(RsGenericSerializer::SerializeJob j, RsGenericSerializer::SerializeContext& ctx)
{
    RsGxsGrpItem::serial_process(j, ctx);
    
    RS_SERIAL_PROCESS(collection.mDescription);
    RS_SERIAL_PROCESS(collection.mCategory);
    RS_SERIAL_PROCESS(collection.mHashTags);
    RS_SERIAL_PROCESS(collection.mModerators);  // NEW: Serialize moderator list
}
```

#### 2.3 API Methods

**In `src/retroshare/rswiki.h`**, add to the RsWiki interface:

```cpp
class RsWiki: public RsGxsIfaceHelper
{
public:
    // ... existing methods ...
    
    /* Moderator Management */
    
    /**
     * Add a moderator to a wiki collection
     * @param groupId The wiki group ID
     * @param moderatorId The GXS ID to add as moderator
     * @return true if successful
     */
    virtual bool addModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId) = 0;
    
    /**
     * Remove a moderator from a wiki collection
     * Sets termination time instead of deleting to maintain history
     * @param groupId The wiki group ID
     * @param moderatorId The GXS ID to remove
     * @return true if successful
     */
    virtual bool removeModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId) = 0;
    
    /**
     * Get list of moderators for a wiki collection
     * @param groupId The wiki group ID
     * @param moderators Output vector of moderator entries
     * @param activeOnly If true, only return active moderators (termination time = 0)
     * @return true if successful
     */
    virtual bool getModerators(const RsGxsGroupId& groupId, 
                               std::vector<RsWikiModeratorEntry>& moderators,
                               bool activeOnly = true) = 0;
    
    /**
     * Check if a GXS ID is an active moderator for a wiki
     * @param groupId The wiki group ID
     * @param gxsId The GXS ID to check
     * @return true if the ID is an active moderator
     */
    virtual bool isModerator(const RsGxsGroupId& groupId, const RsGxsId& gxsId) = 0;
};
```

#### 2.4 Implementation

**In `src/services/p3wiki.h`**, add method declarations:

```cpp
class p3Wiki: public RsGenExchange, public RsWiki
{
public:
    // ... existing methods ...
    
    /* Moderator Management - Override from RsWiki */
    virtual bool addModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId) override;
    virtual bool removeModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId) override;
    virtual bool getModerators(const RsGxsGroupId& groupId, 
                               std::vector<RsWikiModeratorEntry>& moderators,
                               bool activeOnly = true) override;
    virtual bool isModerator(const RsGxsGroupId& groupId, const RsGxsId& gxsId) override;
    
protected:
    // Helper for validation
    bool isActiveModerator(const RsWikiCollection& collection, const RsGxsId& gxsId, rstime_t atTime) const;
};
```

**In `src/services/p3wiki.cc`**, implement the methods:

```cpp
bool p3Wiki::addModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId)
{
    // 1. Fetch the collection
    // 2. Check if caller has admin rights
    // 3. Check if moderator already exists
    // 4. If exists and terminated, reactivate (set termination time to 0)
    // 5. If doesn't exist, add new entry with current time
    // 6. Update the collection
    // 7. Return success/failure
}

bool p3Wiki::removeModerator(const RsGxsGroupId& groupId, const RsGxsId& moderatorId)
{
    // 1. Fetch the collection
    // 2. Check if caller has admin rights
    // 3. Find the moderator entry
    // 4. Set termination time to current time (don't delete entry)
    // 5. Update the collection
    // 6. Return success/failure
}

bool p3Wiki::getModerators(const RsGxsGroupId& groupId, 
                           std::vector<RsWikiModeratorEntry>& moderators,
                           bool activeOnly)
{
    // 1. Fetch the collection
    // 2. Copy moderator list to output vector
    // 3. If activeOnly, filter out entries with termination time > 0
    // 4. Return success/failure
}

bool p3Wiki::isModerator(const RsGxsGroupId& groupId, const RsGxsId& gxsId)
{
    // 1. Fetch the collection
    // 2. Check if gxsId is in moderator list
    // 3. Check if termination time is 0 (active)
    // 4. Return true/false
}

bool p3Wiki::isActiveModerator(const RsWikiCollection& collection, const RsGxsId& gxsId, rstime_t atTime) const
{
    for (const auto& mod : collection.mModerators) {
        if (mod.mModeratorId == gxsId) {
            // Check if moderator was active at the given time
            if (mod.mAddedTime <= atTime && 
                (mod.mTerminationTime == 0 || mod.mTerminationTime > atTime)) {
                return true;
            }
        }
    }
    return false;
}
```

#### 2.5 Validation Logic

Add validation in message processing to enforce moderator rules:

**In `src/services/p3wiki.cc`**, override or modify message validation:

```cpp
// This should be added to the message validation logic
// The exact location depends on how RsGenExchange validates messages

bool p3Wiki::validateWikiMessage(RsGxsMsgItem* msgItem)
{
    // Get the message and group info
    RsGxsWikiSnapshotItem* snapshot = dynamic_cast<RsGxsWikiSnapshotItem*>(msgItem);
    if (!snapshot) {
        return true; // Not a snapshot, use default validation
    }
    
    // Get the collection for this message
    RsWikiCollection collection;
    // ... fetch collection using msgItem->meta.mGroupId ...
    
    // Rule 1: Self-edits are always allowed
    RsGxsId authorId = msgItem->meta.mAuthorId;
    RsGxsId originalAuthorId = getOriginalAuthorId(msgItem->meta.mOrigMsgId);
    
    if (authorId == originalAuthorId) {
        return true; // Self-edit allowed
    }
    
    // Rule 2: Non-self edits require active moderator status
    rstime_t messageTime = msgItem->meta.mPublishTs;
    
    if (isActiveModerator(collection, authorId, messageTime)) {
        return true; // Active moderator can edit
    }
    
    // Rule 3: Reject edits that don't meet criteria
    std::cerr << "p3Wiki: Rejecting message - author is not original author or active moderator" << std::endl;
    return false;
}
```

#### 2.6 Backward Compatibility

Ensure that wikis created before the moderator system still work:

- Empty moderator list = allow all edits (backward compatible behavior)
- Only enforce moderator rules if the moderator list is non-empty
- Serialization must handle missing moderator field gracefully

---

## Testing Requirements

### Unit Tests
1. Test event code emission for all 6 event types
2. Test moderator add/remove/get operations
3. Test moderator validation logic
4. Test serialization/deserialization with moderators

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

1. **Phase 1:** Event codes (Todo 2)
   - Update enum in rswiki.h
   - Implement event classification in p3wiki.cc
   - Test event emission

2. **Phase 2:** Moderator data structures (Todo 6)
   - Add RsWikiModeratorEntry
   - Update RsWikiCollection
   - Add serialization
   - Test data persistence

3. **Phase 3:** Moderator API (Todo 6)
   - Implement add/remove/get methods
   - Add isModerator helper
   - Test API functionality

4. **Phase 4:** Validation logic (Todo 6)
   - Implement message validation
   - Test moderator permissions
   - Test network-wide validation

5. **Phase 5:** Integration & testing
   - Run all tests
   - Verify backward compatibility
   - Performance testing

---

## File Summary

**Files to modify:**
1. `src/retroshare/rswiki.h` - Event codes, moderator API interface, RsWikiCollection structure
2. `src/rsitems/rswikiitems.h` - RsWikiModeratorEntry structure
3. `src/rsitems/rswikiitems.cc` - Moderator serialization
4. `src/services/p3wiki.h` - Moderator method declarations
5. `src/services/p3wiki.cc` - Event logic, moderator implementation, validation

**Estimated effort:** 2-3 days for experienced developer

---

## Notes

- This implementation follows the forums-style moderator pattern as discussed in issue #3107 comments by @csoler
- The moderator system is independent from publish rights, allowing more flexible management
- Termination dates allow for audit history while removing moderator permissions
- Network-wide validation ensures consistent moderation across all peers
- Empty moderator lists maintain backward compatibility with existing wikis

---

**Implementation Target:** samuel-asleep/libretroshare repository  
**After Implementation:** Update RetroShare GUI to use new moderator API  
**Related GUI Work:** WikiGroupDialog moderator management UI (separate from this backend work)
