#pragma once
#include "pch.h"

#include "Runnable.h"

class FThreadHeartBeat : public FRunnable {
public:
	static void Init();
};