/*
 * File: FMODSoundEffect.cpp
 * Date: 16/11/2010 2010 
 * Description: 
 */

/*
 * Author: Scott Downie
 * Version: v 1.0
 * Copyright ©2010 Tag Games Limited - All rights reserved 
 */

#include <ChilliSource/Platform/Windows/FMOD/FMODAudioResourceWindows.h>
#include <ChilliSource/Platform/Windows/FMOD/FMODSystemWindows.h>

namespace moFlo
{
	namespace WindowsPlatform
	{
		//----------------------------------------------------
		/// Constructor
		//----------------------------------------------------
		CFMODAudioResource::CFMODAudioResource() : mpFMODSound(NULL)
		{
		}
        //----------------------------------------------------------
        /// Is A
        //----------------------------------------------------------
        bool CFMODAudioResource::IsA(moFlo::Core::InterfaceIDType inInterfaceID) const
        {
            return (inInterfaceID == Audio::IAudioResource::InterfaceID);
        }
		//----------------------------------------------------------
		/// Get Length
		//----------------------------------------------------------
		f32 CFMODAudioResource::GetLength()
		{
            u32 udwLengthInMs = 0;

            if (mpFMODSound != NULL)
            {
                mpFMODSound->getLength(&udwLengthInMs, FMOD_TIMEUNIT_MS);
            }

			f32 fLengthInSecs = udwLengthInMs * 0.001f;
			return fLengthInSecs;
		}
        //----------------------------------------------------
        /// Set Looping
        //----------------------------------------------------
        void CFMODAudioResource::SetLooping(bool inbShouldLoop)
        {
            if (mpFMODSound != NULL)
            {
                inbShouldLoop ? mpFMODSound->setMode(FMOD_LOOP_NORMAL) : mpFMODSound->setMode(FMOD_LOOP_OFF);
            }
        }
		//----------------------------------------------------
		/// Destructor
		//----------------------------------------------------
		CFMODAudioResource::~CFMODAudioResource()
		{
            if (mpFMODSound != NULL)
            {
                mpFMODSound->release();
            }
		}
	}
}