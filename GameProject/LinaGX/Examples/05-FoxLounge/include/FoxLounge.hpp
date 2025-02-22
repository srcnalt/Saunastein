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

#ifndef EXAMPLE_HPP
#define EXAMPLE_HPP

#include "App.hpp"
#include "LinaGX/LinaGX.hpp"
#include "Camera.hpp"
#include "Utility.hpp"

namespace LinaGX
{
    namespace Examples
    {

        struct PerFrameData
        {
            LinaGX::CommandStream* graphicsStream         = nullptr;
            LinaGX::CommandStream* transferStream         = nullptr;
            uint64                 transferSemaphoreValue = 0;
            uint16                 transferSemaphore      = 0;
            uint16                 globalSet              = 0;

            uint32 rscCameraData        = 0;
            uint32 rscCameraDataGPU     = 0;
            uint8* rscCameraDataMapping = nullptr;

            uint32 rscSceneData         = 0;
            uint8* rscSceneDataMapping  = nullptr;
            uint32 rscObjDataCPU        = 0;
            uint32 rscObjDataGPU        = 0;
            uint8* rscObjDataCPUMapping = nullptr;

            int32 boundShader = -1;
        };

        struct Texture2D
        {
            std::string                        path      = "";
            uint32                             gpuHandle = 0;
            std::vector<LinaGX::TextureBuffer> allLevels;
        };

        struct Material
        {
            std::string                                         name = "";
            GPUMaterialData                                     gpuMat;
            std::unordered_map<LinaGX::GLTFTextureType, uint32> textureIndices;
            uint16                                              descriptorSets[FRAMES_IN_FLIGHT];
            uint32                                              stagingResources[FRAMES_IN_FLIGHT];
            uint32                                              gpuResources[FRAMES_IN_FLIGHT];
        };

        struct RenderTarget
        {
            std::vector<RenderPassColorAttachment> colorAttachments;
            RenderPassDepthStencilAttachment       depthStencilAttachment;
        };

        struct Pass
        {
            RenderTarget renderTargets[FRAMES_IN_FLIGHT];
            uint16       descriptorSets[FRAMES_IN_FLIGHT];
            uint16       additionalDescriptorSets[FRAMES_IN_FLIGHT];
        };

        struct MeshPrimitive
        {
            GPUBuffer vtxBuffer;
            GPUBuffer indexBuffer;
            uint32    materialIndex = 0;
            uint32    indexCount    = 0;
        };

        struct WorldObject
        {
            glm::mat4                  globalMatrix;
            glm::mat4                  invBindMatrix;
            std::string                name      = "";
            bool                       isSkinned = false;
            bool                       hasMesh   = false;
            std::vector<MeshPrimitive> primitives;
            bool                       isSkyCube = false;
        };

        class Example : public App
        {
        public:
            virtual void Initialize() override;
            virtual void Shutdown() override;
            virtual void OnTick() override;
            virtual void OnRender() override;
            virtual void OnWindowResized(uint32 w, uint32 h);

        private:
            void ConfigureInitializeLinaGX();
            void CreateMainWindow();
            void SetupStreams();
            void LoadTexture(const char* path, uint32 id);
            void SetupTextures();
            void SetupSamplers();
            void SetupMaterials();
            void LoadAndParseModels();
            void SetupShaders();
            void SetupGlobalResources();
            void SetupPipelineLayouts();
            void SetupGlobalDescriptorSet();
            void CreatePasses(bool isFirst, uint32 customWidth, uint32 customHeight);
            void DestroyPasses(bool isShutdown);

            void SetupPass(PassType pass, const std::vector<LinaGX::TextureDesc>& renderTargetDescriptions, bool hasDepth, const LinaGX::TextureDesc& depthDescription, const LinaGX::DescriptorSetDesc& descriptorDesc, bool isSwapchain = false);

            void BeginPass(uint32 frameIndex, PassType pass, uint32 width = 0, uint32 height = 0, uint32 viewIndex = 0, uint32 depthViewIndex = 0);
            void EndPass(uint32 frameIndex);
            void DrawObjects(uint32 frameIndex, uint16 flags, Shader shader, bool bindMaterials = true);
            void DrawCube(uint32 frameIndex);
            void DrawFullscreenQuad(uint32 frameIndex);
            void BindShader(uint32 frameIndex, uint32 target);
            void BindPassSet(uint32 frameIndex, PipelineLayoutType layout, uint16 set, uint32 offset, bool useDynamicOffset);
            void ReflectionPass(uint32 frameIndex);
            void CollectPassBarrier(uint32 frameIndex, PassType pass, LinaGX::TextureState target, uint32 srcAccess, uint32 dstAccess, bool collectDepth = false);
            void ExecPassBarriers(LinaGX::CommandStream* stream, uint32 srcStage, uint32 dstStage);
            void TransferGlobalData(uint32 frameIndex);
            void BindGlobalSet(uint32 frameIndex);
            void BindConstants(uint32 frameIndex, uint32 c1, uint32 c2);
            void DeferredRenderScene(uint32 frameIndex, uint16 drawObjFlags, uint32 cameraDataIndex, uint32 width, uint32 height);
            void IrradiancePrefilterBRDF();
            void DrawShadowmap(uint32 frameIndex);
            void Bloom(uint32 frameIndex);
            void SSAOGeometry(uint32 frameIndex, uint32 width, uint32 height);

        private:
            LinaGX::Instance*      m_lgx       = nullptr;
            LinaGX::Window*        m_window    = nullptr;
            uint8                  m_swapchain = 0;
            PerFrameData           m_pfd[FRAMES_IN_FLIGHT];
            std::vector<Texture2D> m_textures;

            std::vector<uint32>      m_samplers;
            std::vector<uint16>      m_shaders;
            std::vector<WorldObject> m_worldObjects;
            std::vector<Material>    m_materials;
            std::vector<Pass>        m_passes;

            uint32 m_skyboxIndexCount = 0;
            Camera m_camera;

            std::vector<uint16>                 m_pipelineLayouts;
            std::vector<LinaGX::TextureBarrier> m_passBarriers;

            glm::vec4             m_lightPos         = {};
            uint32                m_sceneCubemap     = 0;
            uint32                m_ssaoDataCPU      = 0;
            uint32                m_ssaoDataGPU      = 0;
            uint32                m_ssaoNoiseTexture = 0;
            LinaGX::TextureBuffer m_ssaoNoiseBuffer  = {};
            uint32                m_windowX          = 0;
            uint32                m_windowY          = 0;
        };

    } // namespace Examples
} // namespace LinaGX

#endif
