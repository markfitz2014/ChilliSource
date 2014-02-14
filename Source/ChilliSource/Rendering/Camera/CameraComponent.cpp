/*
*  CameraComponent.cpp
*  moFlo
*
*  Created by Scott Downie on 12/10/2010.
*  Copyright 2010 Tag Games. All rights reserved.
*
*/

#include <ChilliSource/Rendering/Camera/CameraComponent.h>
#include <ChilliSource/Core/Main/Screen.h>
#include <ChilliSource/Core/Math/MathUtils.h>
#include <ChilliSource/Core/Entity/Entity.h>
#include <ChilliSource/Core/Main/ApplicationEvents.h>
#include <ChilliSource/Core/Main/Screen.h>

namespace moFlo
{
	namespace Rendering
	{
		DEFINE_NAMED_INTERFACE(CCameraComponent);
		//------------------------------------------------------
		/// Constructor
		///
		/// @param Camera description setting the FOV etc
		//------------------------------------------------------
		CCameraComponent::CCameraComponent(const CameraDescription &inCamDesc) 
			: mDesc(inCamDesc), mbProjectionCacheValid(false)
		{
			EnableViewportRotationWithScreen(mDesc.bShouldRotateToScreen);
			EnableViewportResizeWithScreen(mDesc.bShouldResizeToScreen);
			mDesc.IsOrthographic ? CalculateOrthographicMatrix() : CalculatePerspectiveMatrix();
		}
		//----------------------------------------------------------
		/// Is A
		///
		/// Returns if it is of the type given
		/// @param Comparison Type
		/// @return Whether the class matches the comparison type
		//----------------------------------------------------------
		bool CCameraComponent::IsA(moFlo::Core::InterfaceIDType inInterfaceID) const
		{
			return (inInterfaceID == CCameraComponent::InterfaceID);
		}
		//----------------------------------------------------------
		/// Set Look At
		///
		/// Set the camera orientation, target and position
		/// @param Position
		/// @param Look target
		/// @param Up direction
		//----------------------------------------------------------
		void CCameraComponent::SetLookAt(const Core::CVector3& invPos, const Core::CVector3& invTarget, const Core::CVector3& invUp)
		{
            Core::CEntity * pParent(GetEntityOwner());

            MOFLOW_ASSERT((pParent!=NULL), "CameraComponent has no parent entity");
            
            pParent->Transform().SetLookAt(invPos, invTarget, invUp);
		}
		//------------------------------------------------------
		/// Unproject
		///
		/// Project from a point in screen space to a ray in
		/// world space
		/// @param Point in screen space with orientation
		/// @return Ray in world space with camera view direction
		//------------------------------------------------------
		Core::Ray CCameraComponent::Unproject(const Core::CVector2 &invScreenPos)
		{
            Core::CMatrix4x4 matProj = (GetView() * GetProjection()).Inverse();
            
            Core::CVector2 vScreenSize(Core::CScreen::GetOrientedDimensions());
			//Normalise the screen space co-ordinates into clip space
			f32 nx = ((2.0f * (invScreenPos.x/vScreenSize.x)) - 1.0f);
			f32 ny = ((2.0f * (invScreenPos.y/vScreenSize.y)) - 1.0f);
			
			Core::CVector4 vNear(nx, ny, -1.0f, 1.0f);
            Core::CVector4 vFar(nx,ny, 1.0f, 1.0f);
            vNear = vNear * matProj;
            vFar = vFar * matProj;
            
            Core::Ray cRay;
            
            cRay.vOrigin = vNear/vNear.w;
            cRay.vDirection = vFar/vFar.w - cRay.vOrigin;
            
            cRay.fLength = cRay.vDirection.Length();
            cRay.vDirection /= cRay.fLength;
            
            return cRay;
		}
		//------------------------------------------------------
		/// Project
		///
		/// Convert from a point in world space to a point in
		/// screen space
		//------------------------------------------------------
		Core::CVector2 CCameraComponent::Project(const Core::CVector3 &invWorldPos)
		{
			//Convert the world space position to clip space
			Core::CMatrix4x4 matToClip = (GetView() * GetProjection());
			Core::CVector4 vScreenPos = Core::CVector4(invWorldPos, 1.0f) * matToClip;
			
            Core::CVector2 vScreenSize(Core::CScreen::GetOrientedDimensions());
			
			// Normalize co-ordinates
			vScreenPos.x = vScreenPos.x / vScreenPos.w;
			vScreenPos.y = vScreenPos.y / vScreenPos.w;

			//Convert from clip space to screen space
			vScreenPos.x = (vScreenSize.x * 0.5f)* vScreenPos.x + vScreenSize.x * 0.5f;
			vScreenPos.y = (vScreenSize.y * 0.5f)* vScreenPos.y + vScreenSize.y * 0.5f;

			//Return 2D screen space co-ordinates
			return (Core::CVector2)vScreenPos;
		}
		//----------------------------------------------------------
		/// Use Orthographic View
		///
		/// Switch between ortho and perspective
		/// @param On or off
		//----------------------------------------------------------
		void CCameraComponent::UseOrthographicView(bool inbOrthoEnabled)
		{
			mDesc.IsOrthographic = inbOrthoEnabled;

			mbProjectionCacheValid = false;
		}
        //----------------------------------------------------------
        /// Is Orthographic View
        ///
        /// @return On or off
        //----------------------------------------------------------
        bool CCameraComponent::IsOrthographicView() const
        {
            return mDesc.IsOrthographic;
        }
		//----------------------------------------------------------
		/// Set Viewport Size
		///
		/// @param Vector containing width and height
		//----------------------------------------------------------
		void CCameraComponent::SetViewportSize(const Core::CVector2 &invSize)
		{
			mDesc.vViewSize = invSize;
			mbProjectionCacheValid = false;
		}
		//----------------------------------------------------------
		/// Set Viewport Size
		///
		/// @param Width 
		/// @param Height
		//----------------------------------------------------------
		void CCameraComponent::SetViewportSize(u32 inudwWidth, u32 inudwHeight)
		{
			mDesc.vViewSize.x = (f32)inudwWidth;
			mDesc.vViewSize.y = (f32)inudwHeight;
			mbProjectionCacheValid = false;
		}
		//----------------------------------------------------------
		/// Get Viewport Size
		///
		/// @param Vector containing width and height
		//----------------------------------------------------------
		const Core::CVector2& CCameraComponent::GetViewportSize()
		{ 
			return mDesc.vViewSize; 
		}	
		//------------------------------------------------------
		/// Calculate Perspective Matrix (Normalized)
		///
		/// 
		//------------------------------------------------------
		void CCameraComponent::CalculatePerspectiveMatrix()
		{
			f32 Top = mDesc.fNearClipping * (f32)tanf(mDesc.fFOV * Core::kPI / 360.0f);
			f32 Bottom = -Top;
			f32 Left = Bottom * mDesc.fAspect;
			f32 Right = Top * mDesc.fAspect;	
			f32 Depth = mDesc.fFarClipping - mDesc.fNearClipping;

			f32 A = (Right + Left)/(Right - Left);
			f32 B = (Top + Bottom)/(Top - Bottom);
			f32 C = (mDesc.fFarClipping + mDesc.fNearClipping)/(Depth);
			f32 D = (2.0f * mDesc.fFarClipping * mDesc.fNearClipping)/(Depth);
			f32 F = (2.0f * mDesc.fNearClipping)/(Top - Bottom);
			f32 G = (2.0f * mDesc.fNearClipping)/(Right - Left);

			mmatProj = Core::CMatrix4x4
				(
				G, 0, 0, 0, 
				0, F, 0, 0, 
				A, B, -C, -1, 
				0, 0, -D, 0
				);

			mbProjectionCacheValid = true;
		}
		//------------------------------------------------------
		/// Calculate Orthographic Matrix
		///
		/// 
		//------------------------------------------------------
		void CCameraComponent::CalculateOrthographicMatrix()
		{
			mmatOrthoProj = Core::CMatrix4x4::CreateOrthoMatrix(mDesc.vViewSize.x, mDesc.vViewSize.y, mDesc.fNearClipping, mDesc.fFarClipping);
			mbProjectionCacheValid = true;
		}
		//------------------------------------------------------
		/// Set Viewport Orientation
		///
		/// Rotate the view matrix of this camera to match the 
		/// screen orientation
		/// @param Screen orientation flag
		//------------------------------------------------------
		void CCameraComponent::SetViewportOrientation(Core::ScreenOrientation ineOrientation)
		{
			//Save the camera's new orientation
			mViewOrientation = ineOrientation;

			//Invalidate our view projection
			mbProjectionCacheValid = false;
		}
		//------------------------------------------------------
		/// Get Orthographic Projection 
		///
		/// @return Orthographic projection matrix
		//------------------------------------------------------
		const Core::CMatrix4x4& CCameraComponent::GetOrthoProjection() const
		{
			return mmatOrthoProj;
		}
		//------------------------------------------------------
		/// Get Projection 
		///
		/// If the transform has changed we recalculate the
		/// projection matrix 
		/// 
		/// @return Projection matrix
		//------------------------------------------------------
		const Core::CMatrix4x4& CCameraComponent::GetProjection() 
		{
			if(!mbProjectionCacheValid)
			{
				//Update our projection matrix
				//If we are using a perspective matrix we will also
				//need the orthographic matrix for screen space overlays
				CalculateOrthographicMatrix();
				if(mDesc.IsOrthographic) 
				{
					mmatProj = mmatOrthoProj;
				} 
				else 
				{
					CalculatePerspectiveMatrix();
				}
			}

			return mmatProj;
		}
		//------------------------------------------------------
		/// Get View 
		///
		/// @return View matrix
		//------------------------------------------------------
		const Core::CMatrix4x4& CCameraComponent::GetView()
		{
			if(mpEntityOwner)
			{
				mmatView = mpEntityOwner->Transform().GetWorldTransform().Inverse();
			}

			return mmatView;
		}
		//------------------------------------------------------
		/// Get Frustum Pointer
		///
		/// @return Pointer to camera frustum
		//------------------------------------------------------
		const Core::CFrustum* CCameraComponent::GetFrustumPtr() const
		{
			return &mFrustum;
		}
		//------------------------------------------------------
		/// Update Frustum
		///
		/// Recalculate frustum planes
		//------------------------------------------------------
		void CCameraComponent::UpdateFrustum()
		{
			Core::CMatrix4x4::Multiply(&GetView(), &GetProjection(), &mmatViewProj);

			//Re-calculate the clip planes
			mFrustum.CalculateClippingPlanes(mmatViewProj);
		}
		//------------------------------------------------------
		/// Billboard
		///
		/// Orientate the given matrix to face the cameras
		/// view vector
		//------------------------------------------------------
		void CCameraComponent::Billboard(const Core::CMatrix4x4& inmatBillboarded, Core::CMatrix4x4& outmatBillboarded)
		{
            const Core::CMatrix4x4 matView = GetView();
            Core::CMatrix4x4::Multiply(&inmatBillboarded, &matView, &outmatBillboarded);
            
			outmatBillboarded.m[12] = inmatBillboarded.m[12];
			outmatBillboarded.m[13] = inmatBillboarded.m[13];
			outmatBillboarded.m[14] = inmatBillboarded.m[14];
		}
		//------------------------------------------------------
		/// Set Field Of View
		///
		/// @param Viewing angle in degrees
		//------------------------------------------------------
		void CCameraComponent::SetFieldOfView(const f32 infFOVDegress)
		{
			mDesc.fFOV = infFOVDegress;

			mbProjectionCacheValid = false;
		}
		//------------------------------------------------------
		/// Set Aspect Ratio
		///
		/// @param Aspect Ratio (Viewport width/viewport height)
		//------------------------------------------------------
		void CCameraComponent::SetAspectRatio(const f32 infAspectRatio)
		{
			mDesc.fAspect = infAspectRatio;

			mbProjectionCacheValid = false;
		}
		//------------------------------------------------------
		/// Set Near Clipping
		///
		/// @param Near Z clipping distance in view space
		//------------------------------------------------------
		void CCameraComponent::SetNearClipping(const f32 infNear)
		{
			mDesc.fNearClipping = infNear;

			mbProjectionCacheValid = false;
		}
		//------------------------------------------------------
		/// Set Far Clipping
		///
		/// @param Far Z clipping distance in view space
		//------------------------------------------------------
		void CCameraComponent::SetFarClipping(const f32 infFar)
		{
			mDesc.fFarClipping = infFar;

			mbProjectionCacheValid = false;
		}
		//------------------------------------------------------
		/// Get Field Of View
		///
		/// @return Viewing angle in degrees
		//------------------------------------------------------
		const f32 CCameraComponent::GetFieldOfView() const
		{
			return mDesc.fFOV;
		}
		//------------------------------------------------------
		/// Get Aspect Ratio
		///
		/// @return Aspect Ratio (Viewport width/viewport height)
		//------------------------------------------------------
		const f32 CCameraComponent::GetAspectRatio() const
		{
			return mDesc.fAspect;
		}
		//------------------------------------------------------
		/// Get Near Clipping
		///
		/// @return Near Z clipping distance in view space
		//------------------------------------------------------
		const f32 CCameraComponent::GetNearClipping() const
		{
			return mDesc.fNearClipping;
		}
		//------------------------------------------------------
		/// Get Far Clipping
		///
		/// @return Far Z clipping distance in view space
		//------------------------------------------------------
		const f32 CCameraComponent::GetFarClipping() const
		{
			return mDesc.fFarClipping;
		}
		//------------------------------------------------------
		/// Set Clear Colour
		///
		/// @param Render buffer clear colour
		//------------------------------------------------------
		void CCameraComponent::SetClearColour(const Core::CColour &inCol)
		{
			mDesc.ClearCol = inCol;
		}
		//------------------------------------------------------
		/// Get Clear Colour
		///
		/// @return Render buffer clear colour
		//------------------------------------------------------
		const Core::CColour& CCameraComponent::GetClearColour() const
		{
			return mDesc.ClearCol;
		}
        //--------------------------------------------------------------------------------------------------
        /// Get Opaque Sort Predicate
        ///
        /// @return Gets the currently set opaque sort predicate for this scene
        //--------------------------------------------------------------------------------------------------
        RendererSortPredicatePtr CCameraComponent::GetOpaqueSortPredicate() const
        {
            return mpOpaqueSortPredicate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Transparent Sort Predicate
        ///
        /// @return Gets the currently set opaque sort predicate for this scene
        //--------------------------------------------------------------------------------------------------
        RendererSortPredicatePtr CCameraComponent::GetTransparentSortPredicate() const
        {
            return mpTransparentSortPredicate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Set Opaque Sort Predicate
        ///
        /// @return Opaque sort predicate to use for this scene
        //--------------------------------------------------------------------------------------------------
        void CCameraComponent::SetOpaqueSortPredicate(const RendererSortPredicatePtr & inpPredicate)
        {
            mpOpaqueSortPredicate = inpPredicate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Set Transparent Sort Predicate
        ///
        /// @param Transparent sort predicate to use for this scene
        //--------------------------------------------------------------------------------------------------
        void CCameraComponent::SetTransparentSortPredicate(const RendererSortPredicatePtr & inpPredicate)
        {
            mpTransparentSortPredicate = inpPredicate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Culling Predicate
        ///
        /// @param Culling predicate to use for this camera
        //--------------------------------------------------------------------------------------------------
        CullingPredicatePtr CCameraComponent::GetCullingPredicate() const
        {
            return IsOrthographicView() ? mpOrthographicCulling : mpPerspectiveCulling;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Perspective Culling Predicate
        ///
        /// @param Culling predicate to use for this camera in perpective mode
        //--------------------------------------------------------------------------------------------------
        void CCameraComponent::SetPerspectiveCullingPredicate(const CullingPredicatePtr & inpPredicate)
        {
            mpPerspectiveCulling = inpPredicate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get orthographic Culling Predicate
        ///
        /// @param Culling predicate to use for this camera in orthographic mode
        //--------------------------------------------------------------------------------------------------
        void CCameraComponent::SetOrthographicCullingPredicate(const CullingPredicatePtr & inpPredicate)
        {
            mpOrthographicCulling = inpPredicate;
        }        
		//------------------------------------------------------
		/// Enable Viewport Rotation with Screen
		///
		/// @param Whether the viewport should rotate when
		/// the screen rotates
		//-----------------------------------------------------
		void CCameraComponent::EnableViewportRotationWithScreen(bool inbEnable)
		{
			inbEnable ? Core::CApplicationEvents::GetScreenOrientationChangedEvent() += Core::ApplicationScreenOrientationDelegate(this, &CCameraComponent::SetViewportOrientation) :
						Core::CApplicationEvents::GetScreenOrientationChangedEvent() -= Core::ApplicationScreenOrientationDelegate(this, &CCameraComponent::SetViewportOrientation);
		}
		//------------------------------------------------------
		/// Enable Viewport Resize with Screen
		///
		/// @param Whether the viewport should resize when
		/// the screen resizes
		//-----------------------------------------------------
		void CCameraComponent::EnableViewportResizeWithScreen(bool inbEnable)
		{
			inbEnable ? Core::CApplicationEvents::GetScreenResizedEvent() += Core::ApplicationScreenResizeDelegate(this, &CCameraComponent::SetViewportSize) :
						Core::CApplicationEvents::GetScreenResizedEvent() -= Core::ApplicationScreenResizeDelegate(this, &CCameraComponent::SetViewportSize);
		}
		//-----------------------------------------------------
		/// Destructor
		///
		//-----------------------------------------------------
		CCameraComponent::~CCameraComponent()
		{
			if(mDesc.bShouldResizeToScreen)
			{
				EnableViewportResizeWithScreen(false);
			}
			if(mDesc.bShouldRotateToScreen)
			{
				EnableViewportRotationWithScreen(false);
			}
		}
	}
}
