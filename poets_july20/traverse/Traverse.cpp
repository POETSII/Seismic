// SPDX-License-Identifier: BSD-2-Clause
#include "Traverse.h"

#include <tinsel.h>
#include <POLite.h>

#include <tinsel.h>
#include <POLite.h>

typedef PThread<
		SSSPDevice,
		SSSPState, // State
		int32_t, // Edge label
		SSSPMessage // Message
	> HeatThread;

int main()
{
	// Point thread structure at base of thread's heap
	HeatThread* thread = (HeatThread*) tinselHeapBaseSRAM();

	// Invoke interpreter
	thread->run();

	return 0;
}
