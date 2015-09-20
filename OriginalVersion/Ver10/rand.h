/*
 * An easy but acceptable random function
 */
 
extern unsigned next_rand;

inline void srand( const unsigned seed )
{
    next_rand = seed;
}

inline unsigned rand()
{
    next_rand = next_rand * 1103515245U + 12345;
    return (unsigned)(next_rand / 65536) % 32768;
}

/*
inline unsigned random( const unsigned max )
{
	return rand() % (max + 1);
}
*/
