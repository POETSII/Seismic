// SPDX-License-Identifier: BSD-2-Clause
#include "DenQSeismic.h"

#include <tinsel.h>
#include <POLite.h>

typedef PThread<
		DenQSeismicDevice,
		DenQSeismicState, // State
		None, // Edge label
		DenQSeismicMessage // Message
	> HeatThread;

int main()
{
	// Point thread structure at base of thread's heap
	HeatThread* thread = (HeatThread*) tinselHeapBaseSRAM();

	// Invoke interpreter
	thread->run();

	return 0;
}
