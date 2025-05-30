//---------------------------------------------------------------------------

#ifndef TimerH
#define TimerH
#include <windows.h>
typedef DWORD64  	   ticks_t;
typedef DWORD		   unsafe_ticks_t;

#define UNSAFE_TICKS_LENGTH	0x100000000ULL

/**
 * Singleton timer class.
 *
 * Can output 64bit time in milliseconds.
 * Uses terms:
 * - Unsafe ticks - milliseconds stored as 32 bit unsigned integer
 *   Those tend to overflow every ~47 days, thus considered unsafe
 * - Safe ticks - milliseconds stored as 64 bit unsigned integer
 */
class Timer {
public:
	/**
	 * Returns instance of Timer
	 */
	static Timer *Instance();

	/**
	 * Initializes timer.
	 *
	 * @param time is current unsafe ticks value.
	 */
	void Initialize(ticks_t time);

	/**
	 * Updates current time given unsae ticks value.
	 * Correctly handles overflows.
	 *
	 * @returns time delta to last call
	 */
	unsafe_ticks_t Update(unsafe_ticks_t time);

	/**
	 * Returns current safe ticks value.
	 */
	ticks_t GetTime();

	/**
	 * Returns current frame number
	 */
	ticks_t GetFrame();

protected:
	/**
	 * Protected constructor
	 */
	Timer();

private:
	static Timer	*m_Instance;

	ticks_t		m_Ticks;
	unsafe_ticks_t	m_UnsafeTicks;
	ticks_t		m_Frame;
};

//---------------------------------------------------------------------------
#endif
