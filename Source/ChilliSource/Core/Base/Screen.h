//
//  Screen.h
//  Chilli Source
//  Created by Scott Downie on 12/10/2010.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2010 Tag Games Limited
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

#ifndef _CHILLISOURCE_CORE_BASE_SCREEN_H_
#define _CHILLISOURCE_CORE_BASE_SCREEN_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/Math/Vector2.h>
#include <ChilliSource/Core/System/AppSystem.h>

#include <functional>

namespace ChilliSource
{
	namespace Core
	{
        //----------------------------------------------------------------
        /// An application system for retreiving information on the
        /// screen such as the screen dimensions or the pixel density.
        ///
        /// @author S Downie
        //----------------------------------------------------------------
		class Screen : public AppSystem
		{
		public:
            CS_DECLARE_NAMEDTYPE(Screen);
            //-----------------------------------------------------------
            /// A delegate called when the application screen resolution
            /// changes. This can happen when the window is resized in
            /// a desktop app or when the orientation changes in a mobile
            /// app.
            ///
			/// @author Ian Copland
			///
			/// @param The new screen resolution.
			//-----------------------------------------------------------
            using ResolutionChangedDelegate = std::function<void(const Vector2&)>;
			//-----------------------------------------------------------
			/// @author S Downie
			///
			/// @return Vector containing the width and height of screen
            /// space available to the application. For a desktop app
            /// this will be the current size of the window. For a mobile
            /// application this will be full size of the screen.
			//-----------------------------------------------------------
			virtual const Vector2& GetResolution() const = 0;
            //-----------------------------------------------------------
            /// The density scale factor as reported by the device. What
            /// this factor relates to is platform dependant. On iOS it
            /// is relative to a non-retina screen resolution. On Android
            /// it is a factor that changes depending on whether the
            /// screen is considered, low, medium, high or extra high
            /// density.
            ///
            /// @author S Downie
            ///
            /// @return The density scale factor of the screen
            //-----------------------------------------------------------
            virtual f32 GetDensityScale() const = 0;
            //-----------------------------------------------------------
            /// @author S Downie
            ///
            /// @return The inverse of density scale factor of the screen
            //-----------------------------------------------------------
            virtual f32 GetInverseDensityScale() const = 0;
            //-----------------------------------------------------------
            /// @author Ian Copland
			///
			/// @return An event that is called when the screen resolution
            /// changes.
			//-----------------------------------------------------------
			virtual IConnectableEvent<ResolutionChangedDelegate>& GetResolutionChangedEvent() = 0;
            //-----------------------------------------------------------
            /// Destructor
            ///
			/// @author Ian Copland
			//-----------------------------------------------------------
			virtual ~Screen() {};
		protected:
            friend class Application;
            //-----------------------------------------------------------
            /// Factory creation method called by Application's Create
            /// System method.
            ///
            /// @author Ian Copland
            ///
            /// @param The new instance of the system.
            //-----------------------------------------------------------
            static ScreenUPtr Create();
		};
	}
}

#endif
