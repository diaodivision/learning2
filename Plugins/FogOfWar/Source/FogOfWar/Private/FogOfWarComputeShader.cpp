#include "FogOfWarComputeShader.h"

IMPLEMENT_GLOBAL_SHADER(
	FFogOfWarComputeShader,
	"/FogOfWar/Private/FogOfWar.usf",
	"MainCS",
	SF_Compute
);