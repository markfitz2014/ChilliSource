//
//  The MIT License (MIT)
//
//  Copyright (c) 2016 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include <ChilliSource/Rendering/Base/RenderCommandCompiler.h>

#include <ChilliSource/Core/Threading/TaskScheduler.h>
#include <ChilliSource/Rendering/Base/CameraRenderPassGroup.h>
#include <ChilliSource/Rendering/Base/RenderPass.h>
#include <ChilliSource/Rendering/Base/TargetRenderPassGroup.h>
#include <ChilliSource/Rendering/Target/RenderTargetGroup.h>

namespace ChilliSource
{
    namespace
    {
        /// Calculates whether a Camera Render Pass Group contains at least one render pass
        /// object.
        ///
        /// @param cameraRenderPassGroup
        ///     The camera render pass group.
        ///
        /// @return Whether or not there is at least one object.
        ///
        bool ContainsRenderPassObject(const CameraRenderPassGroup& cameraRenderPassGroup) noexcept
        {
            for (const auto& renderPass : cameraRenderPassGroup.GetRenderPasses())
            {
                if (renderPass.GetRenderPassObjects().size() > 0)
                {
                    return true;
                }
            }
            
            return false;
        }
        
        /// Calculates the number of render command lists required to process the given list
        /// of target render pass groups, and pre and post command lists.
        ///
        /// @param targetRenderPassGroups
        ///     The list of target render pass groups.
        /// @param preRenderCommandList
        ///     The render command list that should be inserted at the beginning of the queue if
        ///     the list contains any commands.
        /// @param postRenderCommandList
        ///     The render command list that should be inserted at the end of the queue if
        ///     the list contains any commands.
        ///
        /// @return The number of render command lists required.
        ///
        u32 CalcNumRenderCommandLists(const std::vector<TargetRenderPassGroup>& targetRenderPassGroups, const RenderCommandList* preRenderCommandList, const RenderCommandList* postRenderCommandList) noexcept
        {
            u32 count = 0;
            
            if (preRenderCommandList->GetOrderedList().size() > 0)
            {
                ++count;
            }
            
            for (const auto& targetRenderPassGroup : targetRenderPassGroups)
            {
                ++count; // target setup
                
                for (const auto& cameraRenderPassGroup : targetRenderPassGroup.GetRenderCameraGroups())
                {
                    for (const auto& renderPass : cameraRenderPassGroup.GetRenderPasses())
                    {
                        if (renderPass.GetRenderPassObjects().size() > 0)
                        {
                            ++count;
                        }
                    }
                }
                
                ++count; // target cleanup
            }
            
            if (postRenderCommandList->GetOrderedList().size() > 0)
            {
                ++count;
            }
            
            return count;
        }
        
        /// Adds a new begin command or begin with target group command, depending on whether or
        /// not a RenderTargetGroup exists.
        ///
        /// @param targetRenderPassGroup
        ///     The target render pass group.
        /// @param renderCommandList
        ///     The render command list to add the command to.
        ///
        void AddBeginCommand(const TargetRenderPassGroup& targetRenderPassGroup, RenderCommandList* renderCommandList) noexcept
        {
            if (targetRenderPassGroup.GetRenderTargetGroup())
            {
                renderCommandList->AddBeginWithTargetGroupCommand(targetRenderPassGroup.GetRenderTargetGroup(), targetRenderPassGroup.GetClearColour());
            }
            else
            {
                renderCommandList->AddBeginCommand(targetRenderPassGroup.GetResolution(), targetRenderPassGroup.GetClearColour());
            }
        }
        
        /// Adds a new apply light command to the list for the given render pass.
        ///
        /// @param renderPass
        ///     The render pass.
        /// @param renderCommandList
        ///     The render command list to add the command to.
        ///
        void AddApplyLightCommand(const RenderPass& renderPass, RenderCommandList* renderCommandList) noexcept
        {
            switch (renderPass.GetLightType())
            {
                case RenderPass::LightType::k_none:
                {
                    renderCommandList->AddApplyAmbientLightCommand(Colour::k_black);
                    break;
                }
                case RenderPass::LightType::k_ambient:
                {
                    const auto& ambientLight = renderPass.GetAmbientLight();
                    renderCommandList->AddApplyAmbientLightCommand(ambientLight.GetColour());
                    break;
                }
                case RenderPass::LightType::k_directional:
                {
                    const auto& directionalLight = renderPass.GetDirectionalLight();
                    auto viewProj = Matrix4::Inverse(directionalLight.GetLightWorldMatrix()) * directionalLight.GetLightProjectionMatrix();
                    
                    const auto& shadowMapTarget = directionalLight.GetShadowMapTarget();
                    const RenderTexture* shadowMapTexture = nullptr;
                    if (shadowMapTarget)
                    {
                        shadowMapTexture = shadowMapTarget->GetDepthTarget();
                        CS_ASSERT(shadowMapTexture, "Shadow map target must have depth texture.");
                    }
                    
                    renderCommandList->AddApplyDirectionalLightCommand(directionalLight.GetColour(), directionalLight.GetDirection(), viewProj, directionalLight.GetShadowTolerance(), shadowMapTexture);
                    break;
                }
                case RenderPass::LightType::k_point:
                {
                    const auto& pointLight = renderPass.GetPointLight();
                    renderCommandList->AddApplyPointLightCommand(pointLight.GetColour(), pointLight.GetPosition(), pointLight.GetAttenuation());
                    break;
                }
                default:
                {
                    CS_LOG_FATAL("Invalid light type.");
                    break;
                }
            }
        }
        
        /// Compiles the render commands for the given render pass. The render pass must contain
        /// render pass objects otherwise this will assert.
        ///
        /// @param renderPass
        ///     The render pass.
        /// @param renderCommandList
        ///     The render command list to add the commands to.
        ///
        void CompileRenderCommandsForPass(const RenderPass& renderPass, RenderCommandList* renderCommandList) noexcept
        {
            AddApplyLightCommand(renderPass, renderCommandList);
            
            const auto& renderPassObjects = renderPass.GetRenderPassObjects();
            CS_ASSERT(renderPassObjects.size() > 0, "Cannot compile a pass with no objects.");
            
            const RenderMaterial* currentMaterial = nullptr;
            const RenderMesh* currentStaticMesh = nullptr;
            const RenderDynamicMesh* currentDynamicMesh = nullptr;
            
            for (const auto& renderPassObject : renderPassObjects)
            {
                if (renderPassObject.GetRenderMaterial() != currentMaterial)
                {
                    currentMaterial = renderPassObject.GetRenderMaterial();
                    currentStaticMesh = nullptr;
                    
                    renderCommandList->AddApplyMaterialCommand(currentMaterial);
                }
                
                switch (renderPassObject.GetType())
                {
                    case RenderPassObject::Type::k_static:
                    {
                        if (renderPassObject.GetRenderMesh() != currentStaticMesh)
                        {
                            currentStaticMesh = renderPassObject.GetRenderMesh();
                            currentDynamicMesh = nullptr;
                            
                            renderCommandList->AddApplyMeshCommand(currentStaticMesh);
                        }
                        break;
                    }
                    case RenderPassObject::Type::k_dynamic:
                    {
                        if (renderPassObject.GetRenderDynamicMesh() != currentDynamicMesh)
                        {
                            currentStaticMesh = nullptr;
                            currentDynamicMesh = renderPassObject.GetRenderDynamicMesh();
                            
                            renderCommandList->AddApplyDynamicMeshCommand(currentDynamicMesh);
                        }
                        break;
                    }
                    default:
                        CS_LOG_FATAL("Invalid RenderPassObject type.");
                        break;
                }
                
                renderCommandList->AddRenderInstanceCommand(renderPassObject.GetWorldMatrix());
            }
        }
    }
    
    //------------------------------------------------------------------------------
    RenderCommandBufferCUPtr RenderCommandCompiler::CompileRenderCommands(const TaskContext& taskContext, const std::vector<TargetRenderPassGroup>& targetRenderPassGroups,
                                                                          std::vector<RenderDynamicMeshUPtr> renderDynamicMeshes, RenderCommandListUPtr preRenderCommandList,
                                                                          RenderCommandListUPtr postRenderCommandList) noexcept
    {
        u32 numLists = CalcNumRenderCommandLists(targetRenderPassGroups, preRenderCommandList.get(), postRenderCommandList.get());
        RenderCommandBufferUPtr renderCommandBuffer(new RenderCommandBuffer(numLists, std::move(renderDynamicMeshes)));
        std::vector<Task> tasks;
        u32 currentList = 0;
        
        if (preRenderCommandList->GetOrderedList().size() > 0)
        {
            *renderCommandBuffer->GetRenderCommandList(currentList++) = std::move(*preRenderCommandList);
        }
        
        for (const auto& targetRenderPassGroup : targetRenderPassGroups)
        {
            AddBeginCommand(targetRenderPassGroup, renderCommandBuffer->GetRenderCommandList(currentList++));
                
            for (const auto& cameraRenderPassGroup : targetRenderPassGroup.GetRenderCameraGroups())
            {
                if (ContainsRenderPassObject(cameraRenderPassGroup))
                {
                    const auto& camera = cameraRenderPassGroup.GetCamera();
                    renderCommandBuffer->GetRenderCommandList(currentList)->AddApplyCameraCommand(camera.GetWorldMatrix().GetTranslation(), camera.GetViewProjectionMatrix());
                    
                    for (const auto& renderPass : cameraRenderPassGroup.GetRenderPasses())
                    {
                        if (renderPass.GetRenderPassObjects().size() > 0)
                        {
                            auto renderCommandList = renderCommandBuffer->GetRenderCommandList(currentList++);
                            tasks.push_back([=, &renderPass](const TaskContext& innerTaskContext)
                            {
                                CompileRenderCommandsForPass(renderPass, renderCommandList);
                            });
                        }
                    }
                }
            }
            
            renderCommandBuffer->GetRenderCommandList(currentList++)->AddEndCommand();
        }
        
        if (postRenderCommandList->GetOrderedList().size() > 0)
        {
            *renderCommandBuffer->GetRenderCommandList(currentList++) = std::move(*postRenderCommandList);
        }
        
        if (tasks.size() > 0)
        {
            taskContext.ProcessChildTasks(tasks);
        }
        
        return std::move(renderCommandBuffer);
    }
}