// SPDX-License-Identifier: BSD-2-Clause
#include "QSeismic.h"

#include <tinsel.h>
#include <POLite.h>

typedef PThread<
		QSeismicDevice,
		QSeismicState, // State
		None, // Edge label
		QSeismicMessage // Message
	> HeatThread;

int main()
{
	// Point thread structure at base of thread's heap
	HeatThread* thread = (HeatThread*) tinselHeapBaseSRAM();

	// Invoke interpreter
	thread->run();

	return 0;
}
