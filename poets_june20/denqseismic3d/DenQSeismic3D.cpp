// SPDX-License-Identifier: BSD-2-Clause
#include "DenQSeismic3D.h"

#include <tinsel.h>
#include <POLite.h>

typedef PThread<
		DenQSeismic3DDevice,
		DenQSeismic3DState, // State
		None, // Edge label
		DenQSeismic3DMessage // Message
	> HeatThread;

int main()
{
	// Point thread structure at base of thread's heap
	HeatThread* thread = (HeatThread*) tinselHeapBaseSRAM();

	// Invoke interpreter
	thread->run();

	return 0;
}
