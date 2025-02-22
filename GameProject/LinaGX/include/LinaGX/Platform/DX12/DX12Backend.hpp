/*
This file is a part of: LinaGX
https://github.com/inanevin/LinaGX

Author: Inan Evin
http://www.inanevin.com

The 2-Clause BSD License

Copyright (c) [2023-] Inan Evin

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once

#include "LinaGX/Core/Backend.hpp"
#include "LinaGX/Platform/DX12/DX12Common.hpp"
#include <atomic>

namespace D3D12MA
{
    class Allocator;
    class Allocation;
} // namespace D3D12MA

namespace LinaGX
{
    class DX12HeapStaging;
    class DX12HeapGPU;

    struct DX12Swapchain
    {
        Microsoft::WRL::ComPtr<IDXGISwapChain3> ptr          = NULL;
        bool                                    isValid      = false;
        bool                                    isActive     = true;
        bool                                    isFullscreen = false;
        Format                                  format       = Format::B8G8R8A8_UNORM;
        LINAGX_VEC<uint32>                      colorTextures;
        uint32                                  width       = 0;
        uint32                                  height      = 0;
        uint32                                  _imageIndex = 0;
        VSyncStyle                              vsync       = {};
    };

    struct DX12RootParamInfo
    {
        uint32         rootParameter  = 0;
        uint32         binding        = 0;
        uint32         set            = 0;
        uint32         elementSize    = 1;
        DescriptorType reflectionType = DescriptorType::UBO;
        bool           isWritable     = false;
    };

    struct DX12PipelineLayout
    {
        bool                                           isValid            = false;
        Microsoft::WRL::ComPtr<ID3D12RootSignature>    rootSig            = NULL;
        Microsoft::WRL::ComPtr<ID3D12CommandSignature> indirectIndexedSig = NULL;
        Microsoft::WRL::ComPtr<ID3D12CommandSignature> indirectDrawSig    = NULL;
        LINAGX_VEC<DX12RootParamInfo>                  rootParams;
        uint32                                         constantsSpace   = 0;
        uint32                                         constantsBinding = 0;
    };

    struct DX12Shader
    {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pso        = NULL;
        DX12PipelineLayout                          layout     = {};
        Topology                                    topology   = Topology::TriangleList;
        bool                                        isValid    = false;
        bool                                        isCompute  = false;
        bool                                        ownsLayout = false;
    };

    struct DX12Texture2D
    {
        LINAGX_VEC<DescriptorHandle> srvs = {};
        LINAGX_VEC<DescriptorHandle> dsvs = {};
        LINAGX_VEC<DescriptorHandle> rtvs = {};

        Microsoft::WRL::ComPtr<ID3D12Resource> rawRes             = NULL;
        uint64                                 requiredAlignment  = 0;
        D3D12MA::Allocation*                   allocation         = NULL;
        D3D12_RESOURCE_STATES                  state              = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        TextureDesc                            desc               = {};
        DXGI_FORMAT                            format             = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        uint32                                 bytesPerPixel      = 0;
        bool                                   isValid            = false;
        bool                                   isSwapchainTexture = false;
    };

    struct DX12Sampler
    {
        bool             isValid    = false;
        DescriptorHandle descriptor = {};
    };

    struct DX12RenderPassImage
    {
        uint32 txt         = 0;
        bool   isSwapchain = false;
    };

    struct DX12BoundDescriptorSet
    {
        uint16             handle  = 0;
        bool               isDirty = false;
        LINAGX_VEC<uint32> boundDynamicOffsets;
        uint32             setAllocIndex = 0;
    };

    struct DX12BoundConstant
    {
        uint8* data             = nullptr;
        uint32 offset           = 0;
        uint32 size             = 0;
        bool   usesStreamAlloc  = false;
        size_t linearAllocIndex = 0;
    };

    struct DX12MSAATargetInfo
    {
        ID3D12Resource* msaaTarget    = nullptr;
        ID3D12Resource* resolveTarget = nullptr;
        DXGI_FORMAT     format        = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
        TextureState    resolveState  = TextureState::ShaderRead;
        uint32          resolveView   = 0;
    };

    struct DX12CommandStream
    {
        bool                                                    isValid            = false;
        ID3D12RootSignature*                                    boundRootSignature = nullptr;
        uint32                                                  boundShader        = 0;
        CommandType                                             type               = CommandType::Graphics;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>          allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>      list;
        LINAGX_VEC<LINAGX_PAIR<uint32, uint64>>                 intermediateResources;
        LINAGX_VEC<LINAGX_PAIR<void*, uint64>>                  adjustedBuffers;
        LINAGX_VEC<LINAGX_PAIR<uint32, DX12BoundDescriptorSet>> boundDescriptorSets;
        DX12BoundConstant                                       boundConstants;
        CommandStream*                                          streamImpl = nullptr;
        LINAGX_VEC<DX12MSAATargetInfo>                          lastMSAATargets;
    };

    struct DX12PerFrameData
    {
    };

    struct DX12Resource
    {
        bool                                   isValid              = false;
        bool                                   isGPUWritable        = false;
        D3D12MA::Allocation*                   allocation           = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> cpuVisibleResource   = nullptr;
        ResourceHeap                           heapType             = ResourceHeap::StagingHeap;
        uint64                                 size                 = 0;
        bool                                   isMapped             = false;
        DescriptorHandle                       descriptor           = {};
        DescriptorHandle                       additionalDescriptor = {};
        D3D12_RESOURCE_STATES                  state                = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
    };

    struct DX12UserSemaphore
    {
        bool                                isValid = false;
        Microsoft::WRL::ComPtr<ID3D12Fence> ptr     = nullptr;
    };

    struct DX12DescriptorBinding
    {
        uint32                  binding              = 0;
        DescriptorBinding       lgxBinding           = {};
        DescriptorHandle        gpuPointer           = {};
        DescriptorHandle        additionalGpuPointer = {};
        LINAGX_VEC<ShaderStage> stages               = {};
    };

    struct DX12DescriptorSet
    {
        bool                                          isValid = false;
        LINAGX_VEC<LINAGX_VEC<DX12DescriptorBinding>> bindings;
        uint32                                        setAllocationCount = 1;
    };

    struct DX12Queue
    {
        bool                                            isValid = false;
        CommandType                                     type    = CommandType::Graphics;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>      queue;
        uint64                                          frameFenceValue = 0;
        LINAGX_VEC<uint64>                              storedFenceValues;
        LINAGX_VEC<Microsoft::WRL::ComPtr<ID3D12Fence>> frameFences;
        std::atomic_flag*                               inUse = nullptr;
    };

    class DX12Backend : public Backend
    {
    private:
        typedef void (DX12Backend::*CommandFunction)(uint8*, DX12CommandStream& stream);

    public:
        DX12Backend()
            : Backend(){};
        virtual ~DX12Backend(){};

        virtual uint16 CreateUserSemaphore() override;
        virtual void   DestroyUserSemaphore(uint16 handle) override;
        virtual void   WaitForUserSemaphore(uint16 handle, uint64 value) override;
        virtual uint8  CreateSwapchain(const SwapchainDesc& desc) override;
        virtual void   DestroySwapchain(uint8 handle) override;
        virtual void   RecreateSwapchain(const SwapchainRecreateDesc& desc) override;
        virtual void   SetSwapchainActive(uint8 swp, bool isActive) override;
        static bool    CompileShader(ShaderStage stage, const LINAGX_STRING& source, DataBlob& outBlob);
        virtual uint16 CreateShader(const ShaderDesc& shaderDesc) override;
        virtual void   DestroyShader(uint16 handle) override;
        virtual uint32 CreateTexture(const TextureDesc& desc) override;
        virtual void   DestroyTexture(uint32 handle) override;
        virtual uint32 CreateSampler(const SamplerDesc& desc) override;
        virtual void   DestroySampler(uint32 handle) override;
        virtual uint32 CreateResource(const ResourceDesc& desc) override;
        virtual void   MapResource(uint32 handle, uint8*& ptr) override;
        virtual void   UnmapResource(uint32 handle) override;
        virtual void   DestroyResource(uint32 handle) override;
        virtual uint16 CreateDescriptorSet(const DescriptorSetDesc& desc) override;
        virtual void   DestroyDescriptorSet(uint16 handle) override;
        virtual void   DescriptorUpdateBuffer(const DescriptorUpdateBufferDesc& desc) override;
        virtual void   DescriptorUpdateImage(const DescriptorUpdateImageDesc& desc) override;
        virtual uint16 CreatePipelineLayout(const PipelineLayoutDesc& desc) override;
        virtual void   DestroyPipelineLayout(uint16 layout) override;
        virtual uint32 CreateCommandStream(const CommandStreamDesc& desc) override;
        virtual void   SetCommandStreamImpl(uint32 handle, CommandStream* stream) override;
        virtual void   DestroyCommandStream(uint32 handle) override;
        virtual void   CloseCommandStreams(CommandStream** streams, uint32 streamCount) override;
        virtual void   SubmitCommandStreams(const SubmitDesc& desc) override;
        virtual uint8  CreateQueue(const QueueDesc& desc) override;
        virtual void   DestroyQueue(uint8 queue) override;
        virtual uint8  GetPrimaryQueue(CommandType type) override;

        void            DX12Exception(HrException e);
        ID3D12Resource* GetGPUResource(const DX12Resource& res);

        ID3D12Device* GetDevice()
        {
            return m_device.Get();
        }

    private:
        uint16 CreateFence();
        void   DestroyFence(uint16 handle);
        void   WaitForFences(ID3D12Fence* fence, uint64 frameFenceValue);
        void   BindDescriptorSets(DX12CommandStream& stream, DX12Shader& shader);
        void   BindConstants(DX12CommandStream& stream, DX12Shader& shader);
        void   IncreaseGraphicsFences();

    public:
        virtual bool Initialize() override;
        virtual void Shutdown() override;
        virtual void Join() override;
        virtual void StartFrame(uint32 frameIndex) override;
        virtual void Present(const PresentDesc& present) override;
        virtual void EndFrame() override;

    private:
        void CMD_BeginRenderPass(uint8* data, DX12CommandStream& stream);
        void CMD_EndRenderPass(uint8* data, DX12CommandStream& stream);
        void CMD_SetViewport(uint8* data, DX12CommandStream& stream);
        void CMD_SetScissors(uint8* data, DX12CommandStream& stream);
        void CMD_BindPipeline(uint8* data, DX12CommandStream& stream);
        void CMD_DrawInstanced(uint8* data, DX12CommandStream& stream);
        void CMD_DrawIndexedInstanced(uint8* data, DX12CommandStream& stream);
        void CMD_DrawIndexedIndirect(uint8* data, DX12CommandStream& stream);
        void CMD_DrawIndirect(uint8* data, DX12CommandStream& stream);
        void CMD_BindVertexBuffers(uint8* data, DX12CommandStream& stream);
        void CMD_BindIndexBuffers(uint8* data, DX12CommandStream& stream);
        void CMD_CopyResource(uint8* data, DX12CommandStream& stream);
        void CMD_CopyBufferToTexture2D(uint8* data, DX12CommandStream& stream);
        void CMD_CopyTexture2DToBuffer(uint8* data, DX12CommandStream& stream);
        void CMD_CopyTexture(uint8* data, DX12CommandStream& stream);
        void CMD_BindDescriptorSets(uint8* data, DX12CommandStream& stream);
        void CMD_BindConstants(uint8* data, DX12CommandStream& stream);
        void CMD_Dispatch(uint8* data, DX12CommandStream& stream);
        void CMD_ExecuteSecondaryStream(uint8* data, DX12CommandStream& stream);
        void CMD_Barrier(uint8* data, DX12CommandStream& stream);
        void CMD_Debug(uint8* data, DX12CommandStream& stream);
        void CMD_DebugBeginLabel(uint8* data, DX12CommandStream& stream);
        void CMD_DebugEndLabel(uint8* data, DX12CommandStream& stream);

    private:
        D3D12MA::Allocator*                        m_dx12Allocator = nullptr;
        static Microsoft::WRL::ComPtr<IDxcLibrary> s_idxcLib;
        Microsoft::WRL::ComPtr<IDXGIAdapter1>      m_adapter      = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Device>       m_device       = nullptr;
        Microsoft::WRL::ComPtr<IDXGIFactory4>      m_factory      = nullptr;
        bool                                       m_allowTearing = false;

        DX12HeapStaging*                                    m_rtvHeap         = nullptr;
        DX12HeapStaging*                                    m_bufferHeap      = nullptr;
        DX12HeapStaging*                                    m_textureHeap     = nullptr;
        DX12HeapStaging*                                    m_dsvHeap         = nullptr;
        DX12HeapStaging*                                    m_samplerHeap     = nullptr;
        IDList<uint8, DX12Swapchain>                        m_swapchains      = {10};
        IDList<uint16, DX12Shader>                          m_shaders         = {20};
        IDList<uint32, DX12Texture2D>                       m_textures        = {50};
        IDList<uint32, DX12CommandStream>                   m_cmdStreams      = {50};
        IDList<uint16, Microsoft::WRL::ComPtr<ID3D12Fence>> m_fences          = {20};
        IDList<uint32, DX12Resource>                        m_resources       = {100};
        IDList<uint16, DX12UserSemaphore>                   m_userSemaphores  = {20};
        IDList<uint32, DX12Sampler>                         m_samplers        = {100};
        IDList<uint16, DX12DescriptorSet>                   m_descriptorSets  = {20};
        IDList<uint8, DX12Queue>                            m_queues          = {5};
        IDList<uint16, DX12PipelineLayout>                  m_pipelineLayouts = {10};
        DX12HeapGPU*                                        m_gpuHeapBuffer   = nullptr;
        DX12HeapGPU*                                        m_gpuHeapSampler  = nullptr;

        LINAGX_VEC<LINAGX_PAIR<LINAGX_TYPEID, CommandFunction>> m_cmdFunctions;
        uint32                                                  m_currentFrameIndex    = 0;
        uint32                                                  m_currentImageIndex    = 0;
        uint32                                                  m_previousRefreshCount = 0;
        uint32                                                  m_previousPresentCount = 0;
        uint32                                                  m_glitchCount          = 0;

        LINAGX_VEC<DX12PerFrameData>                m_perFrameData;
        LINAGX_VEC<LINAGX_PAIR<CommandType, uint8>> m_primaryQueues;

        std::atomic<uint32> m_submissionPerFrame  = 0;
        std::atomic<bool>   m_graphicsFencesDirty = false;
    };

} // namespace LinaGX
