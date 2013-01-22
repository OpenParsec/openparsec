/*
 * PARSEC HEADER: gd_host.h
 */

#ifndef _GD_HOST_H_
#define _GD_HOST_H_


// ----------------------------------------------------------------------------
// HOST BYTE ORDER/WORD SIZE RELATED MACROS                                   -
// ----------------------------------------------------------------------------


// endianness conversion functions --------------------------------------------
//
#define FORCESWAP_16(s)		( ((word)(s) >> 8) | ((word)(s) << 8) )
#define FORCESWAP_32(l)		( ( ((dword)(l)             ) << 24 ) |	\
							  ( ((dword)(l)             ) >> 24 ) |	\
							  ( ((dword)(l) & 0x0000ff00) << 8 )  |	\
							  ( ((dword)(l) & 0x00ff0000) >> 8 ) )

#ifdef SYSTEM_BIG_ENDIAN

	// little endian to host (and vice versa)
	#define SWAP_16(s)		FORCESWAP_16(s)
	#define SWAP_32(l)		FORCESWAP_32(l)

	// big endian to host (and vice versa)
	#define SWAP_16_BIG(s)	(s)
	#define SWAP_32_BIG(l)	(l)

#else

	// little endian to host (and vice versa)
	#define SWAP_16(s)		(s)
	#define SWAP_32(l)		(l)

	// big endian to host (and vice versa)
	#define SWAP_16_BIG(s)	FORCESWAP_16(s)
	#define SWAP_32_BIG(l)	FORCESWAP_32(l)

#endif // SYSTEM_BIG_ENDIAN


// conversion from seg:ofs address to linear address (DOS32 only) -------------
//
#define MK_LINEAR(seg,ofs)			((void*)((seg<<4)+ofs))


// lo and hi byte and word functions (intentionally not named like in win32) --
//
#ifndef LO_BYTE
	#define LO_BYTE(w)				((byte)(w))
#endif

#ifndef HI_BYTE
	#define HI_BYTE(w)				((byte)((w)>>8))
#endif

#ifndef LO_WORD
	#define LO_WORD(l)				((word)(l))
#endif

#ifndef HI_WORD
	#define HI_WORD(l)				((word)((l)>>16))
#endif

#ifndef MAKE_WORD
	#define MAKE_WORD(l,h)			(((byte)(l))|(((word)(byte)(h))<<8))
#endif

#ifndef MAKE_DWORD
	#define MAKE_DWORD(l,h)			(((word)(l))|(((dword)(word)(h))<<16))
#endif


#endif // _GD_HOST_H_


