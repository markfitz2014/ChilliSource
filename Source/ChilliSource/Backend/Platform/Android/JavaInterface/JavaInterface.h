/*
 *  JavaInterface.h
 *  moFlow
 *
 *  Created by Ian Copland on 09/08/2012.
 *  Copyright 2012 Tag Games. All rights reserved.
 *
 */

#ifndef _MOFLOW_PLATFORM_ANDROID_JAVAINTERFACE_JAVAINTERFACE_H_
#define _MOFLOW_PLATFORM_ANDROID_JAVAINTERFACE_JAVAINTERFACE_H_

#include <ChilliSource/Backend/Platform/Android/ForwardDeclarations.h>
#include <ChilliSource/Core/Main/QueryableInterface.h>
#include <jni.h>

namespace moFlo
{
	namespace AndroidPlatform
	{
		//========================================================
		/// Java Interface Manager
		///
		/// Handles all of the java interfaces and provides an
		/// interface to access them.
		//========================================================
		class IJavaInterface : public Core::IQueryableInterface
		{
		public:
			//--------------------------------------------------------
			/// Constructor
			//--------------------------------------------------------
			IJavaInterface();
			//--------------------------------------------------------
			/// Destructor
			//--------------------------------------------------------
			virtual ~IJavaInterface();
		protected:
			//--------------------------------------------------------
			/// Create Native Interface
			///
			/// Creates the Native Interface on the Java side.
			///
			/// @param the name of the native interface.
			//--------------------------------------------------------
			void CreateNativeInterface(const std::string& instrInterfaceName);
			//--------------------------------------------------------
			/// Create Method Reference
			///
			/// Creates a reference to a method in the created native
			/// interface.
			///
			/// @param the name of the native interface.
			//--------------------------------------------------------
			void CreateMethodReference(const std::string& instrMethodName, const std::string& instrMethodSignature);
			//--------------------------------------------------------
			/// Get Java Object
			///
			/// @return the Java Object
			//--------------------------------------------------------
			jobject GetJavaObject() const;
			//--------------------------------------------------------
			/// Get Method ID
			///
			/// @param the method name
			/// @return the method id.
			//--------------------------------------------------------
			jmethodID GetMethodID(const std::string& instrMethodName) const;
		private:

			jobject mpJavaObject;
			std::map<std::string, jmethodID> mMethodReferenceMap;
		};
	}
}

#endif