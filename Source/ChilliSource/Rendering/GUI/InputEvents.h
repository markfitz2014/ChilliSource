//
//  InputEvents.h
//  moFloTest
//
//  Created by Scott Downie on 16/06/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#ifndef _MO_FLO_GUI_INPUT_EVENTS_H_
#define _MO_FLO_GUI_INPUT_EVENTS_H_

#include <ChilliSource/Rendering/GUI/ForwardDeclarations.h>

#include <ChilliSource/Input/Pointer/TouchScreen.h>

#include <ChilliSource/Core/Main/GenericEvent.h>

namespace moFlo
{
    namespace GUI
    {
        typedef fastdelegate::FastDelegate2<CGUIView*, const Input::TouchInfo&> GUIEventDelegate;
        
        class CInputEvents
        {
        public:
            //---User action delegates
            //-----------------------------------------------------------
            /// Get Pressed Inside Event
            ///
            /// A event that is triggered when input is started within
            /// the bounds of the view
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetPressedInsideEvent();
            //-----------------------------------------------------------
            /// Get Released Inside Event
            ///
            /// A event that is triggered when input is ended within
            /// the bounds of the view
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetReleasedInsideEvent();
            //-----------------------------------------------------------
            /// Get Released Outside Event
            ///
            /// A event that is triggered when input is ended outwith
            /// the bounds of the view having started within it
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetReleasedOutsideEvent();
            //-----------------------------------------------------------
            /// Get Moved Outside Event
            ///
            /// A event that is triggered when input is registered outwith
            /// the bounds of the view having started within it
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetMovedOutsideEvent();
            //-----------------------------------------------------------
            /// Get Moved Inside Event
            ///
            /// A event that is triggered when input is detected within
            /// the bounds of the view having started outwith it
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetMovedInsideEvent();
            //-----------------------------------------------------------
            /// Get Moved Within Event
            ///
            /// A event that is triggered when input is detected within
            /// the bounds of the view having started within it
            ///
            /// @return Event
            //-----------------------------------------------------------
            IEvent<GUIEventDelegate>& GetMovedWithinEvent();
            
            //---Touch Delegates
            //-----------------------------------------------------------
            /// On Touch Began
            ///
            /// Called when the window receives cursor/touch input
            ///
            /// @param Touch data
            /// @return Whether the view contains the touch
            //-----------------------------------------------------------
            bool OnTouchBegan(CGUIView* inpView, const Input::TouchInfo & insTouchInfo);
            //-----------------------------------------------------------
            /// On Touch Moved
            ///
            /// Called when the window receives cursor/touch input
            ///
            /// @param Touch data
            /// @return Whether the view contains the touch
            //-----------------------------------------------------------
            bool OnTouchMoved(CGUIView* inpView, const Input::TouchInfo & insTouchInfo);
            //-----------------------------------------------------------
            /// On Touch Ended
            ///
            /// Called when the window stops receiving cursor/touch input
            ///
            /// @param Touch data
            //-----------------------------------------------------------
            void OnTouchEnded(CGUIView* inpView, const Input::TouchInfo & insTouchInfo);
            
        protected:
            
            //---View events
            CEvent2<GUIEventDelegate> mTouchPressedInside;
            CEvent2<GUIEventDelegate> mTouchReleasedInside;
            CEvent2<GUIEventDelegate> mTouchReleasedOutside;
            CEvent2<GUIEventDelegate> mTouchMoveEnter;
            CEvent2<GUIEventDelegate> mTouchMoveExit;
            CEvent2<GUIEventDelegate> mTouchMovedWithin;
            
        private: 
            
            DYNAMIC_ARRAY<u32> mOpenTouches;  
        };
    }
}

#endif