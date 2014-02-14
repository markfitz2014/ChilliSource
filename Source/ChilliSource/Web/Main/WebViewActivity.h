/*
 *  WebView.h
 *  iOSTemplate
 *
 *  Created by Scott Downie on 25/07/2011.
 *  Copyright 2011 Tag Games. All rights reserved.
 *
 */

#ifndef _MO_FLO_CORE_WEB_VIEW_ACTIVITY_H_
#define _MO_FLO_CORE_WEB_VIEW_ACTIVITY_H_

#include <ChilliSource/Core/Main/Activity.h>
#include <ChilliSource/Core/Main/GenericEvent.h>
#include <ChilliSource/Core/File/FileSystem.h>
#include <ChilliSource/Core/Math/UnifiedCoordinates.h>

namespace moFlo
{
	class IWebViewActivity : public IActivity
	{
	public:
		
		DECLARE_NAMED_INTERFACE(IWebViewActivity);
		//--------------------------------------------------------------
		/// Is A
		///
		/// @param Interface ID
		/// @param Whether activity is of given type
		//--------------------------------------------------------------
		bool IsA(Core::InterfaceIDType inID) const;
		//-----------------------------------------------
		/// Present
		/// 
		/// Show the web view and load the given
		/// URL
		///
		/// @param URL
		//-----------------------------------------------
		virtual void Present(const std::string& instrURL, f32 infDismissButtonScale = 0.075f) = 0;
		//-----------------------------------------------
		/// Present From File
		/// 
		/// Show the web view and load the given
		/// html file
		///
		/// @param HTML file name
		//-----------------------------------------------
		virtual void PresentFromFile(Core::STORAGE_LOCATION ineStorageLocation, const std::string& instrFile, f32 infDismissButtonScale = 0.075f) = 0;
		//-----------------------------------------------
		/// Present In External Browser
		///
		/// Opens up device default web browser and displays url
		///
		/// @param URL
		//-----------------------------------------------
		virtual void PresentInExternalBrowser(const std::string& instrURL) = 0;
		//-----------------------------------------------
		/// Dismiss
		/// 
		/// Dismiss the web view interface
		//-----------------------------------------------
		virtual void Dismiss() = 0;
		//-----------------------------------------------
		/// Get Dismissed Event 
		///
		/// @return Event triggered when activity is
		/// dismissed
		//-----------------------------------------------
		IEvent<ActivityDismissedEvent>& GetDismissedEvent();
		//-----------------------------------------------
		/// Set Size
		///
		/// Set the size of the webview on the screen
		///
		/// @param Unified Vector of size
		//-----------------------------------------------
		virtual void SetSize(const UnifiedVector2 & invSize) = 0;
		//-----------------------------------------------
		/// Get Size
		///
		/// Get the size of the webview on the screen
		///
		/// return Unified Vector of size
		//-----------------------------------------------
		virtual UnifiedVector2 GetSize() const = 0;
	protected:
		
		CEvent1<ActivityDismissedEvent> mOnDismissedEvent;
	};
}

#endif