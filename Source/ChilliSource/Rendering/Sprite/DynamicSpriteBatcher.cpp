/*
 *  SpriteBatch.cpp
 *  moFlo
 *
 *  Created by Scott Downie on 06/10/2010.
 *  Copyright 2010 Tag Games. All rights reserved.
 *
 */

#include <ChilliSource/Rendering/Sprite/DynamicSpriteBatcher.h>
#include <ChilliSource/Rendering/Base/RenderSystem.h>

#include <ChilliSource/Core/Math/MathUtils.h>

#ifdef DEBUG_STATS
#include <ChilliSource/Debugging/Base/DebugStats.h>
#endif

namespace ChilliSource
{
	namespace Rendering
	{	
        const u32 kudwMaxSpritesInDynamicBatch = 2048;
        
		//-------------------------------------------------------
		/// Constructor
		///
		/// Default
		//-------------------------------------------------------
		DynamicSpriteBatch::DynamicSpriteBatch(RenderSystem* inpRenderSystem) 
        : mudwCurrentRenderSpriteBatch(0), mudwSpriteCommandCounter(0)
		{
            for(u32 i=0; i<kudwNumBuffers; ++i)
            {
                mpBatch[i] = new SpriteBatch(kudwMaxSpritesInDynamicBatch, inpRenderSystem, BufferUsage::k_dynamic);
            }
            
            maRenderCommands.reserve(50);
		}
        //-------------------------------------------------------
        /// Render
        ///
        /// Batch the sprite to be rendered later. Track the 
        /// render commands so that the correct subset of the 
        /// mesh buffer can be flushed and the correct material
        /// applied
        ///
        /// @param Render system
        /// @param Sprite data to batch
        //-------------------------------------------------------
		void DynamicSpriteBatch::Render(RenderSystem* inpRenderSystem, const SpriteComponent::SpriteData& inpSprite, const Core::Matrix4x4 * inpTransform)
		{
            //If we exceed the capacity of the buffer then we will be forced to flush it
            if(maSpriteCache.size() >= kudwMaxSpritesInDynamicBatch)
            {
                ForceRender(inpRenderSystem);
            }
            
            //As all the contents of the mesh buffer have the same vertex format we can push sprites into the buffer
            //regardless of the material. However we cannot render the buffer in a single draw call we must render
            //subsets of the buffer based on materials
            if(mpLastMaterial && mpLastMaterial != inpSprite.pMaterial)
            {
                ForceCommandChange();
            }
            
            maSpriteCache.push_back(inpSprite);
            if(inpTransform)
            {
                for(u32 i = 0; i < kudwVertsPerSprite; i++)
                    Core::Matrix4x4::Multiply(&inpSprite.sVerts[i].vPos, inpTransform, &maSpriteCache.back().sVerts[i].vPos);
            }
            mpLastMaterial = inpSprite.pMaterial;
            ++mudwSpriteCommandCounter;
		}
        //-------------------------------------------------------
        /// Force Command Change
        ///
        /// Force a render command change so that subsequent
        /// additons to the buffer will not be drawn in this call
        //-------------------------------------------------------
		void DynamicSpriteBatch::ForceCommandChange()
		{
            if(!maSpriteCache.empty())
            {
                maRenderCommands.resize(maRenderCommands.size() + 1);
                
                RenderCommand &sLastCommand = maRenderCommands.back();
                
                //Make a copy of the material that this batch will use
                sLastCommand.Material = *(mpLastMaterial.get());
                //The offset of the indices for this batch
                sLastCommand.udwOffset = ((maSpriteCache.size() - mudwSpriteCommandCounter) * kudwIndicesPerSprite) * sizeof(s16);
                //The number of indices in this batch
                sLastCommand.udwStride = mudwSpriteCommandCounter * kudwIndicesPerSprite;
                mudwSpriteCommandCounter = 0;
            }
		}
        //-------------------------------------------------------
        /// Force Render
        ///
        /// Force the currently batched sprites to be rendered
        /// regardless of whether the batch is full
        ///
        /// @param Render system
        //-------------------------------------------------------
        void DynamicSpriteBatch::ForceRender(RenderSystem* inpRenderSystem)
        {
            if(!maSpriteCache.empty())
            {
#ifdef DEBUG_STATS
                if(!maRenderCommands.empty())
                {
                    RenderCommand &sLastCommand = maRenderCommands.back();
                    if(sLastCommand.Material.IsTransparent())
                    {
                        DebugStats::AddToEvent("Sprites_Trans", (u32)maSpriteCache.size());
                    }
                    else
                    {
                        DebugStats::AddToEvent("Sprites", (u32)maSpriteCache.size());
                    }
                }
#endif
                //Close off the batch
                ForceCommandChange();
                //Copy geometry into the mesh buffer and render
                BuildAndFlushBatch(inpRenderSystem);
            }
        }
        //----------------------------------------------------------
        /// Build and Flush Batch
        ///
        /// Map the batch into the mesh buffer and present the
        /// contents. This will then swap the active buffer
        /// so that it can be filled while the other one is 
        /// rendering
        /// 
        /// @param Render system
        //----------------------------------------------------------
		void DynamicSpriteBatch::BuildAndFlushBatch(RenderSystem* inpRenderSystem)
        {
            if(!maSpriteCache.empty())
            {
                //Build the next buffer
                mpBatch[mudwCurrentRenderSpriteBatch]->Build(&maSpriteCache);
                maSpriteCache.clear();
            }   
            
            //Loop round all the render commands and draw the sections of the buffer with the correct material
            for(std::vector<RenderCommand>::iterator it = maRenderCommands.begin(); it != maRenderCommands.end(); ++it)
            {
                //Render the last filled buffer
                mpBatch[mudwCurrentRenderSpriteBatch]->Render(inpRenderSystem, it->Material, it->udwOffset, it->udwStride);
            }
                
            maRenderCommands.clear();
                
            //Swap the buffers
            mudwCurrentRenderSpriteBatch = Core::MathUtils::Wrap(++mudwCurrentRenderSpriteBatch, 0u, kudwBufferArrayBounds);
		}
        //----------------------------------------------------------
        /// Destructor
        //----------------------------------------------------------
        DynamicSpriteBatch::~DynamicSpriteBatch()
        {
            for(u32 i=0; i<kudwNumBuffers; ++i)
            {
                delete mpBatch[i];
            }
        }
	}
}

