#ifndef __BITSET_H__
#define __BITSET_H__

#ifndef _UV
#define _UV(x) (1 << (x))
#endif

#ifndef _SETBIT
#define _SETBIT(x, y) ((x) |= _UV(y))
#endif

#ifndef _CLRBIT
#define _CLRBIT(x, y) ((x) &= ~_UV(y))
#endif

#endif
