/*
 *  InformationProvider.h
 *  moFlow
 *
 *  Created by Stuart McGaw on 10/06/2011.
 *  Copyright 2011 Tag Games. All rights reserved.
 *
 */

#ifndef _MOFLO_CORE_INFORMATIONPROVIDER_H_
#define _MOFLO_CORE_INFORMATIONPROVIDER_H_

#include <ChilliSource/Core/Main/QueryableInterface.h>

namespace moFlo{
	
	/*
	 IInformationProvider is the base class for objects representing interfaces into a data store of some kind
	 */
	class IInformationProvider : public Core::IQueryableInterface{
	public:
		virtual ~IInformationProvider(){}
		
	};
	
}

#endif