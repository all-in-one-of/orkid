////////////////////////////////////////////////////////////////
// Orkid Media Engine
// Copyright 1996-2012, Michael T. Mayers.
// Distributed under the Boost Software License - Version 1.0 - August 17, 2003
// see http://www.boost.org/LICENSE_1_0.txt
//////////////////////////////////////////////////////////////// 

#pragma once

///////////////////////////////////////////////////////////////////////////////
#include <utility>
//////////////////////////////////////////////////////////////////////////////
namespace ork{
///////////////////////////////////////////////////////////////////////////////

// TODO: escape double-quotes in all strings

template<typename T> void CPropType<T>::GetValueSet( const std::string * & ValueStrings, int & NumStrings )
{	
	NumStrings = 0;
	ValueStrings = 0;
}

///////////////////////////////////////////////////////////////////////////////

template <typename U>
static U ConvInt2U( int iv )
{
	U rval = static_cast<U>(iv);
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
template <typename U>
U CPropType<T>::FindValFromStrings( const std::string& String, const std::string Strings[], U defaultval )
{	U rval = defaultval;
	int idx = 0;
	while( idx>=0 )
	{	const std::string & ptest = Strings[idx];
		if( ptest.length() )
		{	if( ptest == String )
			{	rval = ConvInt2U<U>(idx);
			}
			idx++;
		}
		else
			idx = -1;
	}
	return rval;
}

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////