#ifndef __IDMANAGER_H
#define __IDMANAGER_H

/**
\addtogroup cross-layers
\{
\addtogroup IDManager
\{
*/

#include "opendefs.h"

//#define DAGROOT //if I am the master
#define SLAVENUMBER 1// 1  2 3
//=========================== define ==========================================

#define ACTION_YES      'Y'
#define ACTION_NO       'N'
#define ACTION_TOGGLE   'T'

static const uint8_t linklocalprefix[] = 
{
   0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//=========================== typedef =========================================

BEGIN_PACK

typedef struct {
   bool          isDAGroot;
   uint8_t       myPANID[2];
   uint8_t       my16bID[2];
   uint8_t       my64bID[8];
   uint8_t       myPrefix[8];
} debugIDManagerEntry_t;

END_PACK

//=========================== module variables ================================

typedef struct {
   bool          isDAGroot;
   uint8_t       offset_address;//added by zyx
   uint8_t		 all_slave_mumbers;//added by zyx
   open_addr_t   myPANID;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPrefix;
   open_addr_t	 myParentID;//added by zyx
   bool          slotSkip;
   uint8_t       joinKey[16];
   asn_t         joinAsn;
} idmanager_vars_t;

//=========================== prototypes ======================================

void         idmanager_init(void);
bool         idmanager_getIsDAGroot(void);
uint8_t		 idmanager_getmy_offsetaddress(void);
void         idmanager_setIsDAGroot(bool newRole);
bool         idmanager_getIsSlotSkip(void);
open_addr_t* idmanager_getMyID(uint8_t type);
open_addr_t* idmanager_getMyParentID(void);

owerror_t    idmanager_setMyID(open_addr_t* newID);
bool         idmanager_isMyAddress(open_addr_t* addr);
void         idmanager_triggerAboutRoot(void);
void         idmanager_setJoinKey(uint8_t *key);
void         idmanager_setJoinAsn(asn_t *asn);
void         idmanager_getJoinKey(uint8_t **pKey);

bool         debugPrint_id(void);
bool         debugPrint_joined(void);


/**
\}
\}
*/

#endif
