/*
* PARSEC HEADER: e_relist.h
*/

#ifndef _E_RELIST_H_
#define _E_RELIST_H_

// forward decls --------------------------------------------------------------
//
class E_SimShipState;
class E_SimPlayerInfo;


//FIXME: this should be merged with DEBUG.C

// easy access to the singleton -----------------------------------------------
//
#define TheMemoryManager	UTL_MemoryManager::GetMemoryManager()

// set these wisely
#define MEMMAN_HASH_TABLE_SIZE			1001
#define HASH_POINTER_ADDRESS(x)			( ((size_t)(x)) % MEMMAN_HASH_TABLE_SIZE )

// struct defining a entry in the memory manager ------------------------------
//
class UTL_MemEntry
{
protected:
	UTL_MemEntry*		m_pNext;
	void*				m_p;
	const char*			m_descr1;
	const char*			m_descr2;
public:
	UTL_MemEntry( void* p = NULL, const char* descr1 = NULL, const char* descr2 = NULL ) : 
	  m_pNext( NULL ),
	  m_p( p ),
	  m_descr1( descr1 ),
	  m_descr2( descr2 )
	{
	}

	friend class UTL_MemoryManager;
};

// memory manager -------------------------------------------------------------
//
class UTL_MemoryManager
{
protected:
	
	UTL_MemEntry* m_HashTable[ MEMMAN_HASH_TABLE_SIZE ];

	UTL_MemoryManager()  
	{
		for( int nEntry = 0; nEntry < MEMMAN_HASH_TABLE_SIZE; nEntry++ ) {
			m_HashTable[ nEntry ] = NULL;
		}
	}
	~UTL_MemoryManager() 
	{
		CheckLeaks();
	}
public:
	// SINGLETON pattern
	static UTL_MemoryManager* GetMemoryManager()
	{
		static UTL_MemoryManager _TheMemoryManager;
		return &_TheMemoryManager;
	}

	// add a pointer for tracking
	int AddTracking( void* p, const char* descr1, const char* descr2 )
	{
		//MSGOUT( "UTL_MemoryManager::AddTracking(): %x, %s, %s", p, descr1, descr2 );

		// get hash key 
		size_t htid = HASH_POINTER_ADDRESS( p );

		// search hash table for already existing entry
		for ( UTL_MemEntry* scan = m_HashTable[ htid ]; scan != NULL; scan = scan->m_pNext ) {
			if ( scan->m_p == p ) {
				// issue warning
				MSGOUT( "UTL_MemManager::AddTracking(): duplicate call for pointer %x (%s, %s).", p, descr1, descr2 );
				return FALSE;
			}
		}

		// create new entry & prepend to list
		UTL_MemEntry* newentry = new UTL_MemEntry( p, descr1, descr2 );
		newentry->m_pNext = m_HashTable[ htid ];
		m_HashTable[ htid ] = newentry;

		return TRUE;
	}
	
	// remove a pointer from tracking
	int RemoveTracking( void* p )
	{
		// get hash key 
		size_t htid = HASH_POINTER_ADDRESS( p );

		UTL_MemEntry* scan = m_HashTable[ htid ];
		if ( scan != NULL ) {
			// check for head removal
			if ( scan->m_p == p ) {
				m_HashTable[ htid ] = scan->m_pNext;
				//MSGOUT( "UTL_MemoryManager::RemoveTracking(): %x, %s, %s", scan->m_p, scan->m_descr1, scan->m_descr2 );
				delete scan;
				return TRUE;
			} 

			// setup iteration
			UTL_MemEntry* prev = scan;
			scan = scan->m_pNext;

			for ( ; scan != NULL; scan = scan->m_pNext ) {

				if ( scan->m_p == p ) {

					// unlink from list
					prev->m_pNext = scan->m_pNext;
					//MSGOUT( "UTL_MemoryManager::RemoveTracking(): %x, %s, %s", scan->m_p, scan->m_descr1, scan->m_descr2 );
					delete scan;
					return TRUE;
				}
			}
		}

		// issue warning
		MSGOUT( "UTL_MemManager::RemoveTracking(): tracking entry for pointer %x not found.", p );
		return FALSE;
	}

	// dump all pointers we still track
	void CheckLeaks()
	{
		for( int nEntry = 0; nEntry < MEMMAN_HASH_TABLE_SIZE; nEntry++ ) {
			
			for( UTL_MemEntry* scan = m_HashTable[ nEntry ]; scan != NULL; scan = scan->m_pNext ) {
				MSGOUT( "UTL_MemManager::CheckLeaks(): leaked pointer %x (%s, %s)", scan->m_p, scan->m_descr1, scan->m_descr2 );
			}
		}
	}
};

// class holding a remote event list ------------------------------------------
//
class E_REList
{
protected:
	char*	m_data;
	char*	m_CurPos;
	size_t  m_Avail;
	size_t	m_nMaxSize;

	// reference counting stuff
	int		m_nRefCount;
	~E_REList();
	E_REList();
	E_REList( size_t size );
public:

	// reference counting stuff
	static E_REList* CreateAndAddRef( size_t size ) 
	{ 
		E_REList* relist = new E_REList; 
		relist->AddRef(); 
#ifdef PARSEC_DEBUG
		TheMemoryManager->AddTracking( (void*)relist, __FILE__, "E_REList::CreateAndAddRef()" );
#endif // PARSEC_DEBUG
		relist->Init( size ); 
		return relist; 
	}

	void AddRef()	{ m_nRefCount++; }
	void Release()	
	{ 
		m_nRefCount--; 
		if ( m_nRefCount == 0 ) {
#ifdef PARSEC_DEBUG
			TheMemoryManager->RemoveTracking( (void*)this );
#endif // PARSEC_DEBUG
			delete this; 
		}
	}
	
	// init the remote event list
	void Init( size_t size );

	// return the internal data of the remote event list
	RE_Header*	GetData() { return (RE_Header*)m_data;	}

	// return the size of the RE list
	size_t GetSize() { return (size_t)( m_CurPos - m_data ); }

	// clear remote event list
	int Clear();
		
	// append a remote event list
	int AppendList( RE_Header* relist );

	// append a E_REList
	int AppendList( E_REList* relist );

	// append a remote event
	size_t AppendEvent( RE_Header* re, size_t size );
	
	// check whether we have any remote events in the list
	int HasEvents()
	{
		return ( ( (RE_Header*) m_data)->RE_Type != RE_EMPTY );
	}
	
	// write to an external remote event list 
	size_t WriteTo( RE_Header* dst, size_t maxsize, int allow_truncate );

	// check if enough space in RE_List for specified remote event
	int RmEvAllowed( int re_type );
	
	// dump contents of RE list
	void Dump();

	// allocate space for a specific RE
	RE_Header* NET_Allocate( int retype );

	// insert state sync remote event
	int RmEvStateSync( byte statekey, byte stateval );

	// append a RE_OwnerSection event
	int NET_Append_RE_OwnerSection( int ownerid );

	// insert command info into remote event list
	int NET_Append_RE_CommandInfo( const char* commandstring );

	// append a RE_PlayerAndShipStatus event
	//FIXME: GAMECODE
	//FIXME: consolidate naming with RmEv<remote-event-name>
	int NET_Append_RE_PlayerAndShipStatus( int nClientID, E_SimPlayerInfo* pSimPlayerInfo, E_SimShipState* pSimShipState, refframe_t CurRefFrame, bool_t bUpdatePropsOnly );

	// append a RE_GameState
	//FIXME: GAMECODE
	int NET_Append_RE_GameState();

	// append a RE_KillStats event
	//FIXME: GAMECODE
	int NET_Append_RE_KillStats();

	// append a RE_CreateLaser event
	//FIXME: GAMECODE
	int NET_Append_RE_CreateLaser( const LaserObject* laserpo );

    // append a RE_CreateMissile event
    //FIXME: GAMECODE
    int NET_Append_RE_CreateMissile( const MissileObject *missilepo, dword targetobjid );
     
    // Append a RE_CreateExtra event
    int NET_Append_RE_CreateExtra( const ExtraObject *extrapo );
	
    // append a RE_KillOjbect event
	//FIXME: GAMECODE
	int NET_Append_RE_KillObject( dword objectid, byte listno );

	// append a RE_CreateExtra2 event
	//FIXME: GAMECODE
	int NET_Append_RE_CreateExtra2( const ExtraObject *extrapo );

	// append a RE_IPv4ServerInfo event
	int NET_Append_RE_IPv4ServerInfo( node_t* node, word nServerID, int xpos, int ypos, word flags );

	// append a RE_ServerLinkInfo event
	int NET_Append_RE_ServerLinkInfo( word nServerID_1, word nServerID_2, word flags );

	// append a RE_Generic 
	int NET_Append_RE_Generic( word flags, dword HostObjId, dword TargetId, dword padding );

	// append a RE_MapObject
	int NET_Append_RE_MapObject( int map_objectid, char* name, int xpos, int ypos, int w, int h, char* texname );

    //Append a Particle RE (energy fields, megashield...etc)
    int NET_Append_RE_ParticleObject( int type, const Vertex3& origin );
    
	// determine the size of a remote-event list
	static size_t DetermineListSize( RE_Header* relist )
	{
		ASSERT( relist != NULL );
		
		size_t lsize = 0;
		
		// process remote event list
		while ( relist->RE_Type != RE_EMPTY ) {
			
			// sum up size of all remote events
			size_t resize = RmEvGetSize( relist );
			lsize += resize;
			
			// advance to next event
			ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == resize ) );
			relist = (RE_Header *) ( (char *) relist + resize );
		}
		
		// include size of RE list termination
		lsize += sizeof( dword );

		return lsize;
	}

	// determine size of remote event
	static size_t RmEvGetSize( RE_Header *relist );

	// determine size of remote event from type
	static size_t RmEvGetSizeFromType( byte retype );

	// check whether the remote event list is well formed
	static int IsWellFormed( RE_Header *relist );

	// return the max. size of the RE list that can fit in one external packet
	static size_t GetMaxSizeInPacket();

	// validate the RE according to all bounds
	static int ValidateRE( RE_Header* relist, size_t size );

protected:
};


#define NET_RmEvList_GetSize		E_REList::DetermineListSize
#define NET_RmEvList_IsWellFormed	E_REList::IsWellFormed


#endif // _E_RELIST_H_
