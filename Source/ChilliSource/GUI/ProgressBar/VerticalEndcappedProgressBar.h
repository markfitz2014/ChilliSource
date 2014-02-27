//
//  VerticalEndcappedProgressBar.h
//  iOSTemplate
//
//  Created by Thomas Kronberg on 12/02/2013.
//  Copyright (c) 2013 Tag Games. All rights reserved.
//

#ifndef _MOFLOW_GUI_DEFAULTS_VERTICALENDCAPPEDPROGRESSBAR_H_
#define _MOFLOW_GUI_DEFAULTS_VERTICALENDCAPPEDPROGRESSBAR_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/GUI/ProgressBar/ProgressBar.h>
#include <ChilliSource/GUI/Image/VerticalStretchableImage.h>

namespace ChilliSource
{
	namespace GUI
	{
		class VerticalEndcappedProgressBar : public ProgressBar
		{
		public:
            
			DECLARE_META_CLASS(VerticalEndcappedProgressBar)
            
			VerticalEndcappedProgressBar();
			VerticalEndcappedProgressBar(const Core::ParamDictionary& insParams);
            
			//------------------------------------------------------------------------
			/// Set Sprite Sheet
			///
			/// @param Sprite Sheet
			//------------------------------------------------------------------------
			void SetSpriteSheet(const Rendering::SpriteSheetSPtr& inpSpriteSheet);
			//------------------------------------------------------------------------
			/// Get Sprite Sheet
			///
			/// @return Sprite Sheet
			//------------------------------------------------------------------------
			const Rendering::SpriteSheetSPtr& GetSpriteSheet() const;
			//---------------------------------------------------------
			/// Set Base Fill Sprite Sheet Index ID
			///
			/// Set the "path" to the sprite sheet index IDs.
			/// e.g. For the following IDs:
			///			* "BLUE_PANEL_LEFT"
			///			* "BLUE_PANEL_CENTRE"
			///			* "BLUE_PANEL_RIGHT"
			///
			/// the base ID would be "BLUE_PANEL_"
			//---------------------------------------------------------
			void SetBaseFillSpriteSheetIndexID(const std::string& instrID);
			//---------------------------------------------------------
			/// Get Base Fill Sprite Sheet Index ID
			///
			/// Get the "path" to the sprite sheet index IDs.
			/// e.g. For the following IDs:
			///			* "BLUE_PANEL_LEFT"
			///			* "BLUE_PANEL_CENTRE"
			///			* "BLUE_PANEL_RIGHT"
			///
			/// the base ID would be "BLUE_PANEL_"
			//---------------------------------------------------------
			const std::string& GetBaseFillSpriteSheetIndexID() const;
			//---------------------------------------------------------
			/// Set Base Background Sprite Sheet Index ID
			///
			/// Set the "path" to the sprite sheet index IDs.
			/// e.g. For the following IDs:
			///			* "BLUE_PANEL_LEFT"
			///			* "BLUE_PANEL_CENTRE"
			///			* "BLUE_PANEL_RIGHT"
			///
			/// the base ID would be "BLUE_PANEL_"
			//---------------------------------------------------------
			void SetBaseBackgroundSpriteSheetIndexID(const std::string& instrID);
			//---------------------------------------------------------
			/// Get Base Background Sprite Sheet Index ID
			///
			/// Get the "path" to the sprite sheet index IDs.
			/// e.g. For the following IDs:
			///			* "BLUE_PANEL_LEFT"
			///			* "BLUE_PANEL_CENTRE"
			///			* "BLUE_PANEL_RIGHT"
			///
			/// the base ID would be "BLUE_PANEL_"
			//---------------------------------------------------------
			const std::string& GetBaseBackgroundSpriteSheetIndexID() const;
			//------------------------------------------------------------------------
			/// Set Fill Indices
			///
			/// @param Sprite sheet index of top patch
			/// @param Sprite sheet index of mid patch
			/// @param Sprite sheet index of bottom patch
			//------------------------------------------------------------------------
			void SetFillIndices(u32 inudwTop, u32 inudwMid, u32 inudwBottom);
			//------------------------------------------------------------------------
			/// Set Background Indices
			///
			/// @param Sprite sheet index of top patch
			/// @param Sprite sheet index of mid patch
			/// @param Sprite sheet index of bottom patch
			//------------------------------------------------------------------------
			void SetBackgroundIndices(u32 inudwTop, u32 inudwMid, u32 inudwBottom);
            //--------------------------------------------------------
            /// Enable Width From Image
            ///
            /// When this is enabled the image view's width will be
            /// based on the width of the cap
            ///
            /// @param Enable/disable
            //--------------------------------------------------------
            void EnableWidthFromImage(bool inbEnable);
			//--------------------------------------------------------
			/// Is Width From Image Enabled
			///
			/// When this is enabled the image view's size will be
			/// based on the size of the image
			///
			/// @return Whether the image view's size will be
			/// based on the size of the image
			//--------------------------------------------------------
			bool IsWidthFromImageEnabled() const;
			//------------------------------------------------------------------------
			/// Set Fill Colour
			///
			/// @param Colour of progress bar
			//------------------------------------------------------------------------
			void SetFillColour(const Core::Colour& inColour);
			//------------------------------------------------------------------------
			/// Get Fill Colour
			///
			/// @return Colour of progress bar
			//------------------------------------------------------------------------
			const Core::Colour& GetFillColour() const;
			//------------------------------------------------------------------------
			/// Set Background Colour
			///
			/// @param Colour of background
			//------------------------------------------------------------------------
			void SetBackgroundColour(const Core::Colour& inColour);
			//------------------------------------------------------------------------
			/// Get Background Colour
			///
			/// @return Colour of background
			//------------------------------------------------------------------------
			const Core::Colour& GetBackgroundColour() const;
			//------------------------------------------------------------------------
			/// Draw
			///
			/// Draw this view and any subviews
			///
			/// @param Renderer
			//------------------------------------------------------------------------
			virtual void Draw(Rendering::CanvasRenderer* inpCanvas);
            
		private:
            
			DECLARE_PROPERTY_A(Rendering::SpriteSheetSPtr, SpriteSheet, SetSpriteSheet, GetSpriteSheet);
            
			DECLARE_PROPERTY_A(std::string, BaseFillSpriteSheetIndexID, SetBaseFillSpriteSheetIndexID, GetBaseFillSpriteSheetIndexID);
			DECLARE_PROPERTY_A(std::string, BaseBackgroundSpriteSheetIndexID, SetBaseBackgroundSpriteSheetIndexID, GetBaseBackgroundSpriteSheetIndexID);
            
			DECLARE_PROPERTY_A(Core::Colour, FillColour, SetFillColour, GetFillColour);
			DECLARE_PROPERTY_A(Core::Colour, BackgroundColour, SetBackgroundColour, GetBackgroundColour);
			
            
            DECLARE_PROPERTY_A(bool, WidthFromImage, EnableWidthFromImage, IsWidthFromImageEnabled);
			
			VerticalStretchableImageSPtr mpFillImage;
			VerticalStretchableImageSPtr mpBGImage;
            
		};
	}
}

#endif