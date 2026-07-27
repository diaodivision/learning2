#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "FogOfWarTypes.h"
#include "FogOfWarShaderTypes.ush"

class FFogOfWarComputeShader : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFogOfWarComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FFogOfWarComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// 标量参数
		SHADER_PARAMETER(FVector2f, GroundOrigin)
		SHADER_PARAMETER(FVector2f, GroundSize)
		SHADER_PARAMETER(FIntPoint, TextureSize)
		SHADER_PARAMETER(float, SmoothStepThreshold)
		SHADER_PARAMETER(float, SmoothStepDistanceThreshold)

		// 数组大小参数
		SHADER_PARAMETER(uint32, NumVisionStart)
		SHADER_PARAMETER(uint32, NumVision)
		SHADER_PARAMETER(uint32, NumRadiusSq)
		SHADER_PARAMETER(uint32, NumDirection)
		SHADER_PARAMETER(uint32, NumWorldHeghtBuffer)

		// 结构化缓冲区SHADER_PARAMETER_RDG_BUFFER_SRV
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FIntPoint>, VisionStartPosBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FVector2f>, VisionBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<int32>, RadiusSqBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FVector2f>, DirectionBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FHeightData>, WorldHeightBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, WorldHeightTexture)

		// 输出纹理
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		//const FPermutationDomain PermutationVector(Parameters.PermutationId);
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADS_X"), FogOfWarConst::kThreadsX);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), FogOfWarConst::kThreadsY);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), FogOfWarConst::kThreadsZ);
	}
};