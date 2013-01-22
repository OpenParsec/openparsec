/*
* PARSEC HEADER: UTL_List.h
*/

#ifndef _UTL_LIST_H_
#define _UTL_LIST_H_

#include <stdio.h>


// type for data in the list --------------------------------------------------
//
typedef void* UTL_listdata;

// struct defining one entry in the UTL_List ----------------------------------
//
template <class T>
class UTL_listentry_s
{
public:
	T					m_data;
	UTL_listentry_s*	m_pPrev;
	UTL_listentry_s*	m_pNext;
};

// class for holding a single linked list -------------------------------------
//
template <class T>
class UTL_List {
protected:
	UTL_listentry_s<T>*	m_pHead;
	UTL_listentry_s<T>*	m_pTail;
	int					m_nNumEntries;

	// unlinkg an entry from the list
	void _Unlink( UTL_listentry_s<T>* node )
	{
		ASSERT( node != NULL );

		// unlink from list
		if ( node->m_pNext != NULL ) {
			node->m_pNext->m_pPrev = node->m_pPrev;
		}
		if ( node->m_pPrev != NULL ) {
			node->m_pPrev->m_pNext = node->m_pNext;
		}

		// correct head/tail pointers
		if ( node->m_pPrev == NULL ) {
			ASSERT( node == m_pHead );
			m_pHead = node->m_pNext;
		}
		if ( node->m_pNext == NULL ) {
			ASSERT( node == m_pTail );
			m_pTail = node->m_pPrev;
		}

		m_nNumEntries--;
	}

public:
	UTL_List()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_nNumEntries = 0;
	}

	~UTL_List()
	{
		Clear();
	}

	// clear all entries in the list
	void Clear()
	{
		UTL_listentry_s<T>* next;
		for( UTL_listentry_s<T>* node = m_pHead; node != NULL; ) {
			next = node->m_pNext;
			delete node;
			node = next;
		}
		m_nNumEntries = 0;
		m_pHead = NULL;
		m_pTail = NULL;
	}
	void RemoveAll()	{ Clear(); }

	// append an entry at the tail of the list
	UTL_listentry_s<T>* AppendTail( T _data )
	{
		UTL_listentry_s<T>* newentry = new UTL_listentry_s<T>;
		newentry->m_data  = _data;
		newentry->m_pPrev = m_pTail;
		newentry->m_pNext = NULL;
		if ( m_pTail != NULL ) {
			m_pTail->m_pNext = newentry;
			m_pTail = newentry;
		} else {
			ASSERT( m_pHead == NULL );
			m_pHead = newentry;
			m_pTail = newentry;
		}

		m_nNumEntries++;

		return newentry;
	}

	// append an entry at the head of the list
	UTL_listentry_s<T>* AppendHead( T _data )
	{
		UTL_listentry_s<T>* newentry = new UTL_listentry_s<T>;
		newentry->m_data  = _data;
		newentry->m_pPrev = NULL;
		newentry->m_pNext = m_pHead;
		if ( m_pHead != NULL ) {
			m_pHead->m_pPrev = newentry;
			m_pHead = newentry;
		} else {
			ASSERT( m_pTail == NULL );
			m_pHead = newentry;
			m_pTail = newentry;
		}

		m_nNumEntries++;

		return newentry;
	}

	// return the head entry of the list
	UTL_listentry_s<T>* GetHead() const { return m_pHead; }

	// return the tail entry of the list
	UTL_listentry_s<T>* GetTail() const { return m_pTail; }

	// find an entry with a specific data
	UTL_listentry_s<T>* Find( T& data ) const
	{
		for( UTL_listentry_s<T>* node = m_pHead; node != NULL; ) {
			if ( node->m_data == data ) {
				return node;
			}
			node = node->m_pNext;
		}
		return NULL;
	}

	

	// remove an entry specified by its data
	//FIXME: evt. introduce key into UTL_listentry_s<T>
	bool_t Remove( T& _data )
	{
		UTL_listentry_s<T>* node = Find( _data );
		if ( node != NULL ) {
			_Unlink( node );
			delete node;
			return true;
		}

		return false;
	}

	// dump contents
	void Dump() const
	{
		int nEntry = 0;
		printf( "# of entries: %d", m_nNumEntries );
		for( UTL_listentry_s<T>* node = m_pHead; node != NULL; ) {
			printf( "%03d: adress: %8x, data: %8x prev: %8x next: %8x\n", nEntry, (int)node, (int)node->m_data, (int)node->m_pPrev, (int)node->m_pNext );
			node = node->m_pNext;
			nEntry++;
		}
	}

	// return the # of entries
	int GetNumEntries() const
	{
		return m_nNumEntries;
	}

	// return the entry at a specified index
	UTL_listentry_s<T>* GetEntryAtIndex( int nIndex ) const
	{
		// TODO: MINGW32 fails on this for some reason... might have to look into why...
		//ASSERT( ( nIndex >= 0 ) && ( nIndex < m_nNumEntries ) );
		int nEntry = 0;
		for( UTL_listentry_s<T>* node = m_pHead; node != NULL; ) {
			
			if ( nEntry == nIndex ) {
				return node;
			}

			node = node->m_pNext;
			nEntry++;
		}

		return NULL;
	}

	// remove entry from list, returns data in entry
	T RemoveHead()
	{
		ASSERT( GetHead() != NULL );
		return RemoveEntry( GetHead() );
	}

	// remove tail entry from list, returns data in entry
	T RemoveTail()
	{
		ASSERT( GetTail() != NULL );
		return RemoveEntry( GetTail() );
	}

	// remove entry from list, returns data in entry
	T RemoveEntry( UTL_listentry_s<T>* entry )
	{
		ASSERT( entry != NULL );
		_Unlink( entry );
		T data = entry->m_data;
		delete entry;
		return data;
	}

	// call a function for each entry in the list, stop iteration upon fuction failing
	void ForEach( int (*procfunc)( UTL_listentry_s<T>* node, void* param ), void* param )
	{
		for( UTL_listentry_s<T>* node = m_pHead; node != NULL; ) {

			// call the processing function
			if ( (*procfunc) (node, param ) == FALSE ) {
				break;
			}

			node = node->m_pNext;
		}
	}
};

/*
template <class T>
class UTL_ListWalker
{
protected:
	void*	m_pData;
public:
	UTL_ListWalker( )
	virtual void Callback( UTL_listentry_s<T>* entry ) = 0;
};

*/
#endif // _UTL_LIST_H_

