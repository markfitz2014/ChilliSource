//
//  VideoPlayer.cpp
//  Chilli Source
//  Created by S Downie on 12/05/2011.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2011 Tag Games Limited
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

#include <ChilliSource/Backend/Platform/iOS/Video/Base/VideoPlayer.h>

#include <ChilliSource/Backend/Platform/iOS/Core/Base/EAGLView.h>
#include <ChilliSource/Backend/Platform/iOS/Core/File/FileSystem.h>
#include <ChilliSource/Backend/Platform/iOS/Core/Notification/NSNotificationAdapter.h>
#include <ChilliSource/Backend/Platform/iOS/Core/String/NSStringUtils.h>
#include <ChilliSource/Backend/Platform/iOS/Video/Base/SubtitlesRenderer.h>
#include <ChilliSource/Backend/Platform/iOS/Video/Base/VideoPlayerTapListener.h>
#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/MakeDelegate.h>
#include <ChilliSource/Core/Base/Screen.h>
#include <ChilliSource/Core/String/StringUtils.h>
#include <ChilliSource/Core/Math/MathUtils.h>
#include <ChilliSource/Core/Base/ApplicationEvents.h>

#include <MediaPlayer/MediaPlayer.h>
#include <AudioToolbox/AudioSession.h>

namespace ChilliSource
{
    namespace iOS
    {
        namespace
        {
            //--------------------------------------------------------------
            //--------------------------------------------------------------
            void AudioRouteCallback(void *inpUserData, AudioSessionPropertyID inudwPropertyID, UInt32 inudwPropertyValueSize, const void *inpPropertyValue)
            {
                // Only interested in audio route changes.
                if(inudwPropertyID != kAudioSessionProperty_AudioRouteChange)
                {
                    return;
                }
                
                const CFDictionaryRef kRouteChangeDictionary = (CFDictionaryRef)inpPropertyValue;
                const CFNumberRef     kRouteChangeReasonRef  = (CFNumberRef)CFDictionaryGetValue(kRouteChangeDictionary, CFSTR (kAudioSession_AudioRouteChangeKey_Reason));
                
                SInt32 dwRouteChangeReason(0);
                CFNumberGetValue (kRouteChangeReasonRef, kCFNumberSInt32Type, &dwRouteChangeReason);
                
                // If headphones pulled out or connection to speakers lost.
                if(dwRouteChangeReason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
                {
                    // Get the movie controller.
                    MPMoviePlayerController *pMovieController = static_cast<MPMoviePlayerController*>(inpUserData);
                    
                    // Restart the movie.
                    if(pMovieController)
                    {
                        // The sleep is required, as sometimes it seems the callback is applied before the video has been paused.
                        usleep(3000);
                        [pMovieController play];
                    }
                }
            }
        }
        
        CS_DEFINE_NAMEDTYPE(VideoPlayer);
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        VideoPlayer::VideoPlayer()
            : m_moviePlayerController(nil), m_tapListener(nil), m_playing(false), m_dismissWithTap(false), m_videoOverlayView(nil), m_subtitlesRenderer(nil)
        {
        }
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool VideoPlayer::IsA(Core::InterfaceIDType in_interfaceId) const
		{
			return (in_interfaceId == Video::VideoPlayer::InterfaceID || in_interfaceId == VideoPlayer::InterfaceID);
		}
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        void VideoPlayer::Present(Core::StorageLocation in_storageLocation, const std::string& in_fileName, const VideoDelegate& in_delegate, bool in_dismissWithTap, const Core::Colour& in_backgroundColour)
        {
            @autoreleasepool
            {
                CS_ASSERT(m_playing == false, "Cannot present video while a video is already playing.");
                
                m_playing = true;
                
                m_completionDelegate = in_delegate;
                m_backgroundColour = in_backgroundColour;
                
                std::string filePath = Core::Application::Get()->GetFileSystem()->GetAbsolutePathToFile(in_storageLocation, in_fileName);
                
                NSString* urlString = [NSStringUtils newNSStringWithString:filePath];
                NSURL* pMovieURL = [NSURL fileURLWithPath:urlString];
                [urlString release];
                
                m_moviePlayerController = [[MPMoviePlayerController alloc] initWithContentURL:pMovieURL];
                
                AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, AudioRouteCallback, m_moviePlayerController);
                
                Prepare();
                
                m_dismissWithTap = in_dismissWithTap;
                
                ListenForMoviePlayerNotifications();
            }
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        void VideoPlayer::PresentWithSubtitles(Core::StorageLocation in_storageLocation, const std::string& in_fileName, const Video::SubtitlesCSPtr& in_subtitles, const VideoDelegate& in_delegate,
                                                     bool in_dismissWithTap, const Core::Colour& in_backgroundColour)
        {
            m_subtitles = in_subtitles;
            Present(in_storageLocation, in_fileName, in_delegate, in_dismissWithTap);
        }
        //-------------------------------------------------------------
        //-------------------------------------------------------------
        f32 VideoPlayer::GetCurrentTime() const
        {
            f32 time = 0.0f;
            
            if (m_moviePlayerController != nil)
            {
                time = (f32)[m_moviePlayerController currentPlaybackTime];
            }
            
            return time;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        Core::Vector2 VideoPlayer::GetVideoDimensions() const
        {
            return Core::Vector2(m_moviePlayerController.naturalSize.width, m_moviePlayerController.naturalSize.height);
        }
        //--------------------------------------------------------
        //--------------------------------------------------------
        void VideoPlayer::OnInit()
        {
            m_tapListener = [[CVideoPlayerTapListener alloc] init];
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::Prepare()
        {
            [m_moviePlayerController setControlStyle:MPMovieControlStyleNone];
            [m_moviePlayerController setFullscreen:YES];
            [m_moviePlayerController setRepeatMode:MPMovieRepeatModeNone];
            [m_moviePlayerController prepareToPlay];
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::Play()
        {
            CreateVideoOverlay();
            Core::Application::Get()->Suspend();
            [m_moviePlayerController play];
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::ListenForMoviePlayerNotifications()
        {
            [[NSNotificationAdapter sharedInstance] BeginListeningForMPLoadStateChanged];
            m_moviePlayerLoadStateChangedConnection = [[NSNotificationAdapter sharedInstance] GetMPLoadStateChangeEvent].OpenConnection(Core::MakeDelegate(this, &VideoPlayer::OnLoadStateChanged));
            [[NSNotificationAdapter sharedInstance] BeginListeningForMPPlaybackDidFinish];
            m_moviePlayerPlaybackFinishedConnection = [[NSNotificationAdapter sharedInstance] GetMPPlaybackDidFinishEvent].OpenConnection(Core::MakeDelegate(this, &VideoPlayer::OnPlaybackFinished));
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::StopListeningForMoviePlayerNotifications()
        {
            m_moviePlayerLoadStateChangedConnection = nullptr;
            m_moviePlayerPlaybackFinishedConnection = nullptr;
            
            [[NSNotificationAdapter sharedInstance] StopListeningForMPLoadStateChanged];
            [[NSNotificationAdapter sharedInstance] StopListeningForMPPlaybackDidFinish];
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::OnTapped()
        {
            if(m_moviePlayerController)
            {
                [m_moviePlayerController stop];
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::OnLoadStateChanged()
        {
            //Unless state is unknown, start playback
            if([m_moviePlayerController loadState] != MPMovieLoadStateUnknown)
            {
                [[NSNotificationAdapter sharedInstance] StopListeningForMPLoadStateChanged];
                m_moviePlayerLoadStateChangedConnection = nullptr;
                
                SetupMovieView();
                AttachMovieViewToWindow();
                Play();
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::SetupMovieView()
        {
            @autoreleasepool
            {
                f32 orientedWidthDensityCorrected = Core::Screen::GetOrientedWidth() * Core::Screen::GetInverseDensity();
                f32 orientedHeightDensityCorrected = Core::Screen::GetOrientedHeight() * Core::Screen::GetInverseDensity();
            
                m_moviePlayerController.backgroundView.backgroundColor = [UIColor colorWithRed:m_backgroundColour.r green:m_backgroundColour.g blue:m_backgroundColour.b alpha:m_backgroundColour.a];
                [[m_moviePlayerController view] setFrame:CGRectMake(0, 0, orientedWidthDensityCorrected, orientedHeightDensityCorrected)];
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::AttachMovieViewToWindow()
        {
            [[[[[UIApplication sharedApplication] keyWindow] rootViewController] view] addSubview:m_moviePlayerController.view];
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::OnPlaybackFinished()
        {
            m_playing = false;
            
            DeleteVideoOverlay();
            
            Core::Application::Get()->Resume();
            
            if([m_moviePlayerController respondsToSelector:@selector(setFullscreen:animated:)])
            {
                [m_moviePlayerController.view removeFromSuperview];
            }
            
            AudioSessionRemovePropertyListenerWithUserData(kAudioSessionProperty_AudioRouteChange, AudioRouteCallback, m_moviePlayerController);
            
            StopListeningForMoviePlayerNotifications();
            
            [m_moviePlayerController release];
            m_moviePlayerController = nil;
            
            [[NSNotificationAdapter sharedInstance] StopListeningForMPPlaybackDidFinish];
            m_moviePlayerPlaybackFinishedConnection = nullptr;
            
            if (m_completionDelegate != nullptr)
            {
                VideoDelegate delegate = m_completionDelegate;
                m_completionDelegate = nullptr;
                delegate();
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::OnResume()
        {
            if (m_moviePlayerController != nil)
            {
                if (m_playing == true)
                {
                    [m_moviePlayerController play];
                }
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::CreateVideoOverlay()
        {
            if (m_videoOverlayView == nil)
            {
                //create the overlay
                CGRect rect = CGRectMake(0, 0, Core::Screen::GetOrientedWidth() * Core::Screen::GetInverseDensity(), Core::Screen::GetOrientedHeight() * Core::Screen::GetInverseDensity());
                m_videoOverlayView = [[UIView alloc] initWithFrame: rect];
                UIView* rootView = [[[[UIApplication sharedApplication] keyWindow] rootViewController] view];
                [rootView addSubview:m_videoOverlayView];
                [rootView bringSubviewToFront:m_videoOverlayView];
                
                //setup the tap gesture if we can dismiss with tap
                if (m_dismissWithTap && m_tapListener != nil)
                {
                    [m_tapListener SetupWithView: m_videoOverlayView AndDelegate:Core::MakeDelegate(this, &VideoPlayer::OnTapped)];
                }
                
                //create the subtitles renderer
                if (m_subtitles != nullptr && m_subtitlesRenderer == nil)
                {
                    m_subtitlesRenderer = [[CSubtitlesRenderer alloc] initWithVideoPlayer:this view:m_videoOverlayView andSubtitles:m_subtitles];
                }
            }
        }
        //---------------------------------------------------------------
        //---------------------------------------------------------------
        void VideoPlayer::DeleteVideoOverlay()
        {
            if (m_videoOverlayView != nil)
            {
                //cleanup the subtitles renderer
                if (m_subtitlesRenderer != nil)
                {
                    [m_subtitlesRenderer CleanUp];
                    [m_subtitlesRenderer release];
                    m_subtitlesRenderer = nil;
                }
                
                //cleanup the tap listener
                if (m_tapListener != nil)
                {
                    [m_tapListener Reset];
                }
                
                //delete the overlay
                [m_videoOverlayView removeFromSuperview];
                [m_videoOverlayView release];
                m_videoOverlayView = nil;
                
                //reset the overlay options
                m_dismissWithTap = false;
            }
        }
        //--------------------------------------------------------
        //--------------------------------------------------------
        void VideoPlayer::OnDestroy()
        {
            [m_tapListener release];
        }
    }
}
