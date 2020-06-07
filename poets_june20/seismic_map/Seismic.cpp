// SPDX-License-Identifier: BSD-2-Clause
#include "Seismic.h"

#include <tinsel.h>
#include <POLite.h>

typedef PThread<
		SeismicDevice,
		SeismicState, // State
		int32_t, // Edge label
		SeismicMessage // Message
	> HeatThread;

int main()
{
	// Point thread structure at base of thread's heap
	HeatThread* thread = (HeatThread*) tinselHeapBaseSRAM();

	// Invoke interpreter
	thread->run();

	return 0;
}
