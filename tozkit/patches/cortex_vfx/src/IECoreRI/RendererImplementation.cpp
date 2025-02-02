//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECoreRI/private/RendererImplementation.h"
#include "IECoreRI/PrimitiveVariableList.h"
#include "IECoreRI/ParameterList.h"
#include "IECoreRI/Convert.h"
#include "IECoreRI/ScopedContext.h"

#include "IECore/MessageHandler.h"
#include "IECore/Shader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/Transform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Group.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"

#include <iostream>

#include "ri.h"
#include "rx.h"

using namespace IECore;
using namespace IECoreRI;
using namespace Imath;
using namespace std;
using namespace boost;

////////////////////////////////////////////////////////////////////////
// AttributeState implementation
////////////////////////////////////////////////////////////////////////

IECoreRI::RendererImplementation::AttributeState::AttributeState()
{
}

IECoreRI::RendererImplementation::AttributeState::AttributeState( const AttributeState &other )
{
	primVarTypeHints = other.primVarTypeHints;
}

////////////////////////////////////////////////////////////////////////
// IECoreRI::RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

const unsigned int IECoreRI::RendererImplementation::g_shaderCacheSize = 10 * 1024 * 1024;
tbb::queuing_rw_mutex IECoreRI::RendererImplementation::g_nLoopsMutex;
std::vector<int> IECoreRI::RendererImplementation::g_nLoops;

IECoreRI::RendererImplementation::RendererImplementation( RendererImplementationPtr parent )
	:	m_context( 0 ), m_sharedData( parent ? parent->m_sharedData : SharedData::Ptr( new SharedData ) )
{
	constructCommon();
}

IECoreRI::RendererImplementation::RendererImplementation( const std::string &name )
	:	m_sharedData( new SharedData )
{
	constructCommon();
	if( name!="" )
	{
		RiBegin( (char *)name.c_str() );
	}
	else
	{
#ifdef PRMANEXPORT	
		RiBegin( "launch:prman? -t" );
#else
		RiBegin( 0 );
#endif		
	}
	m_context = RiGetContext();
}

void IECoreRI::RendererImplementation::constructCommon()
{
	m_camera = new Camera;
	m_camera->addStandardParameters();
	m_camera->setTransform( new MatrixTransform() );

	m_attributeStack.push( AttributeState() );

	const char *fontPath = getenv( "IECORE_FONT_PATHS" );
	if( fontPath )
	{
		m_fontSearchPath.setPaths( fontPath, ":" );
	}

	m_shaderCache = defaultShaderCache();

	m_setAttributeHandlers["toz:dof"] = &IECoreRI::RendererImplementation::tozOptSetDof;



	m_setOptionHandlers["ri:searchpath:shader"] = &IECoreRI::RendererImplementation::setShaderSearchPathOption;
	m_setOptionHandlers["ri:pixelsamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;
	m_setOptionHandlers["ri:pixelSamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;
	m_setOptionHandlers["searchPath:font"] = &IECoreRI::RendererImplementation::setFontSearchPathOption;

	m_getOptionHandlers["shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:resolution"] = &IECoreRI::RendererImplementation::getResolutionOption;
	m_getOptionHandlers["searchPath:font"] = &IECoreRI::RendererImplementation::getFontSearchPathOption;

	m_setAttributeHandlers["ri:shadingRate"] = &IECoreRI::RendererImplementation::setShadingRateAttribute;
	m_setAttributeHandlers["ri:matte"] = &IECoreRI::RendererImplementation::setMatteAttribute;
	m_setAttributeHandlers["ri:color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["ri:opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["ri:sides"] = &IECoreRI::RendererImplementation::setSidesAttribute;
	m_setAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::setDoubleSidedAttribute;
	m_setAttributeHandlers["rightHandedOrientation"] = &IECoreRI::RendererImplementation::setRightHandedOrientationAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:motionFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:focusFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["name"] = &IECoreRI::RendererImplementation::setNameAttribute;
	m_setAttributeHandlers["ri:subsurface"] = &IECoreRI::RendererImplementation::setSubsurfaceAttribute;
	m_setAttributeHandlers["ri:detail"] = &IECoreRI::RendererImplementation::setDetailAttribute;
	m_setAttributeHandlers["ri:detailRange"] = &IECoreRI::RendererImplementation::setDetailRangeAttribute;

	m_getAttributeHandlers["ri:shadingRate"] = &IECoreRI::RendererImplementation::getShadingRateAttribute;
	m_getAttributeHandlers["ri:matte"] = &IECoreRI::RendererImplementation::getMatteAttribute;
	m_getAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::getDoubleSidedAttribute;
	m_getAttributeHandlers["rightHandedOrientation"] = &IECoreRI::RendererImplementation::getRightHandedOrientationAttribute;
	m_getAttributeHandlers["name"] = &IECoreRI::RendererImplementation::getNameAttribute;

	m_commandHandlers["ri:readArchive"] = &IECoreRI::RendererImplementation::readArchiveCommand;
	m_commandHandlers["ri:archiveRecord"] = &IECoreRI::RendererImplementation::archiveRecordCommand;
	m_commandHandlers["ri:illuminate"] = &IECoreRI::RendererImplementation::illuminateCommand;

	m_inMotion = false;
}

IECoreRI::RendererImplementation::~RendererImplementation()
{
	if( m_context )
	{
		RtContextHandle c = RiGetContext();
		RiContext( m_context );
		RiEnd();
		if( c!=m_context )
		{
			RiContext( c );
		}
	}
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	ScopedContext scopedContext( m_context );

	SetOptionHandlerMap::iterator it = m_setOptionHandlers.find( name );
	if( it!=m_setOptionHandlers.end() )
	{
		(this->*(it->second))( name, value );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", format( "Expected option name matching \"ri:*:*\" but got \"%s\"." ) % name );
		}
		else
		{
			string s1( name, 3, i-3 );
			string s2( name, i+1 );
			ParameterList pl( s2, value );
			RiOptionV( (char *)s1.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		ParameterList pl( s, value );
		RiOptionV( "user", pl.n(), pl.tokens(), pl.values() );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore options prefixed for some other RendererImplementation
		return;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
	}
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getOption( const std::string &name ) const
{
	ScopedContext scopedContext( m_context );
	GetOptionHandlerMap::const_iterator it = m_getOptionHandlers.find( name );
	if( it!=m_getOptionHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
#ifdef PRMANEXPORT	
		return getRxOption( name.c_str() + 5 );
#else		
		return getRxOption( name.c_str() );		
#endif		
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		return getRxOption( name.c_str() + 3 );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// silently ignore options prefixed for some other RendererImplementation
		return 0;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::getOption", format( "Unknown option \"%s\"." ) % name );
	}
	return 0;
}

void IECoreRI::RendererImplementation::tozOptSetDof( const std::string &name, IECore::ConstDataPtr d )
{
	printf( "tozOptSetDof\n");

	if( ConstCompoundDataPtr s = runTimeCast<const CompoundData>( d ) )
	{
		ConstFloatDataPtr pfstop = runTimeCast<FloatData>((s->readable().find("fstop"))->second);
		ConstFloatDataPtr pflen = runTimeCast<FloatData>((s->readable().find("focallength"))->second);
		ConstFloatDataPtr pfdist = runTimeCast<FloatData>((s->readable().find("focaldistance"))->second);

		float fstop = pfstop->readable();
		float flen = pflen->readable();
		float fdist = pfdist->readable();

		RiDepthOfField (RtFloat(fstop), RtFloat(flen), RtFloat(fdist));
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected CompoundData for \"toz:dof\"." );
	}
}

void IECoreRI::RendererImplementation::setShaderSearchPathOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstStringDataPtr s = runTimeCast<const StringData>( d ) )
	{
		m_shaderCache = new CachedReader( SearchPath( s->readable(), ":" ), g_shaderCacheSize );
		ParameterList p( "shader", d );
		RiOptionV( "searchpath", p.n(), p.tokens(), p.values() );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected StringData for \"ri:searchpath:shader\"." );
	}
}

void IECoreRI::RendererImplementation::setPixelSamplesOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstV2iDataPtr s = runTimeCast<const V2iData>( d ) )
	{
		RiPixelSamples( s->readable().x, s->readable().y );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected V2iData for \"ri:pixelSamples\"." );
	}
}

void IECoreRI::RendererImplementation::setFontSearchPathOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstStringDataPtr s = runTimeCast<const StringData>( d ) )
	{
		m_fontSearchPath.setPaths( s->readable(), ":" );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected StringData for \"searchPath:font\"." );
	}
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getFontSearchPathOption( const std::string &name ) const
{
	return new StringData( m_fontSearchPath.getPaths( ":" ) );
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getShutterOption( const std::string &name ) const
{
	float shutter[2];
	RxInfoType_t resultType;
	int resultCount;
	int s = RxOption( "Shutter", shutter, 2 * sizeof( float ), &resultType, &resultCount );
	if( s==0 )
	{
		if( resultType==RxInfoFloat && resultCount==2 )
		{
			return new V2fData( V2f( shutter[0], shutter[1] ) );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getResolutionOption( const std::string &name ) const
{
	float format[3];
	RxInfoType_t resultType;
	int resultCount;
	int s = RxOption( "Format", format, 3 * sizeof( float ), &resultType, &resultCount );
	if( s==0 )
	{
		if( resultType==RxInfoFloat && resultCount==3 )
		{
			return new V2iData( V2i( (int)format[0], (int)format[1] ) );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getRxOption( const char *name ) const
{
	char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
	memset( result, 0, 16 * sizeof( RtFloat ) ); // 3delight has a bug where it'll try to free some random part of memory if this is not null (v 7.0.54)
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxOption( (char *)name, result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
	{
		return convert( result, resultType, resultCount );
	}
	return 0;
}

void IECoreRI::RendererImplementation::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	CompoundDataPtr parameterData = (new CompoundData( parameters ))->copy();

	CameraPtr camera = new Camera( name, 0, parameterData );
	camera->addStandardParameters(); // it simplifies outputCamera() to know that the camera is complete

	bool outputNow = false;
	CompoundDataMap::const_iterator outputNowIt=parameters.find( "ri:outputNow" );
	if( outputNowIt!=parameters.end() )
	{
		if( ConstBoolDataPtr b = runTimeCast<BoolData>( outputNowIt->second ) )
		{
			outputNow = b->readable();
		}
		else
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::camera", "\"ri:outputNow\" parameter should be of type BoolData." );
		}
	}

	if( outputNow )
	{
		outputCamera( camera );
		m_camera = 0;
	}
	else
	{
		// add transform and store for output at worldBegin().
		CompoundDataMap::const_iterator transformIt=parameters.find( "transform" );
		if( transformIt!=parameters.end() )
		{
			if( M44fDataPtr m = runTimeCast<M44fData>( transformIt->second ) )
			{
				camera->setTransform( new MatrixTransform( m->readable() ) );
			}
			else
			{
				msg( Msg::Error, "IECoreRI::RendererImplementation::camera", "\"transform\" parameter should be of type M44fData." );
			}
		}
		else
		{
			camera->setTransform( new MatrixTransform( getTransform() ) );
		}
		m_camera = camera;
	}
}

void IECoreRI::RendererImplementation::outputCamera( IECore::CameraPtr camera )
{
	// then shutter
	CompoundDataMap::const_iterator it = camera->parameters().find( "shutter" );
	ConstV2fDataPtr shutterD = runTimeCast<const V2fData>( it->second );
	RiShutter( shutterD->readable()[0], shutterD->readable()[1] );

	// then hider
	it = camera->parameters().find( "ri:hider" );
	if( it!=camera->parameters().end() )
	{
		ConstStringDataPtr d = runTimeCast<const StringData>( it->second );
		if( d )
		{
			ParameterList p( camera->parameters(), "ri:hider:" );
			RiHiderV( (char *)d->readable().c_str(), p.n(), p.tokens(), p.values() );
		}
		else
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::worldBegin", "Camera \"ri:hider\" parameter should be of type StringData." );
		}
	}

	// then resolution
	it = camera->parameters().find( "resolution" );
	ConstV2iDataPtr d = runTimeCast<const V2iData>( it->second );
	RiFormat( d->readable().x, d->readable().y, 1 );

	// then screen window
	it = camera->parameters().find( "screenWindow" );
	ConstBox2fDataPtr screenWindowD = runTimeCast<const Box2fData>( it->second );
	RiScreenWindow( screenWindowD->readable().min.x, screenWindowD->readable().max.x, screenWindowD->readable().min.y, screenWindowD->readable().max.y );

	// then crop window
	it = camera->parameters().find( "cropWindow" );
	ConstBox2fDataPtr cropWindowD = runTimeCast<const Box2fData>( it->second );
	RiCropWindow( cropWindowD->readable().min.x, cropWindowD->readable().max.x, cropWindowD->readable().min.y, cropWindowD->readable().max.y );

	// then clipping
	it = camera->parameters().find( "clippingPlanes" );
	ConstV2fDataPtr clippingD = runTimeCast<const V2fData>( it->second );
	RiClipping( clippingD->readable()[0], clippingD->readable()[1] );

	// then projection
	it = camera->parameters().find( "projection" );
	ConstStringDataPtr projectionD = runTimeCast<const StringData>( it->second );
	ParameterList p( camera->parameters(), "projection:" );
	RiProjectionV( (char *)projectionD->readable().c_str(), p.n(), p.tokens(), p.values() );

	// transform last
	if( camera->getTransform() )
	{
		M44f m = camera->getTransform()->transform();
		m.scale( V3f( 1.0f, 1.0f, -1.0f ) );
		m.invert();
		setTransform( m );
	}
}

/// \todo This should be outputting several calls to display as a series of secondary displays, and also trying to find the best display
/// to be used as the primary display.
void IECoreRI::RendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	ParameterList pl( parameters );
	RiDisplayV( (char *)name.c_str(), (char *)type.c_str(), (char *)data.c_str(), pl.n(), pl.tokens(), pl.values() );
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::worldBegin()
{
	ScopedContext scopedContext( m_context );
	if( m_camera )
	{
		outputCamera( m_camera );
	}
	RiWorldBegin();
}

void IECoreRI::RendererImplementation::worldEnd()
{
	ScopedContext scopedContext( m_context );
	RiWorldEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::transformBegin()
{
	ScopedContext scopedContext( m_context );
	RiTransformBegin();
}

void IECoreRI::RendererImplementation::transformEnd()
{
	ScopedContext scopedContext( m_context );
	RiTransformEnd();
}

void IECoreRI::RendererImplementation::setTransform( const Imath::M44f &m )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	RtMatrix mm;
	convert( m, mm );
	RiTransform( mm );
}

void IECoreRI::RendererImplementation::setTransform( const std::string &coordinateSystem )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	RiCoordSysTransform( (char *)coordinateSystem.c_str() );
}

Imath::M44f IECoreRI::RendererImplementation::getTransform() const
{
	return getTransform( "object" );
}

Imath::M44f IECoreRI::RendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	ScopedContext scopedContext( m_context );

	M44f result;
	RtMatrix matrix;
	if( RxTransform( (char *)coordinateSystem.c_str(), "world", 0.0f, matrix ) == 0 )
	{
		result = M44f( matrix );
	}
	else
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::getTransform", boost::format( "Unable to transform to coordinate system \"%s\"." ) % coordinateSystem );
	}

	return result;
}

void IECoreRI::RendererImplementation::concatTransform( const Imath::M44f &m )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	RtMatrix mm;
	convert( m, mm );
	RiConcatTransform( mm );
}

void IECoreRI::RendererImplementation::coordinateSystem( const std::string &name )
{
	ScopedContext scopedContext( m_context );
	RiScopedCoordinateSystem( (char *)name.c_str() );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::attributeBegin()
{
	ScopedContext scopedContext( m_context );
	m_attributeStack.push( m_attributeStack.top() );
	RiAttributeBegin();
}

void IECoreRI::RendererImplementation::attributeEnd()
{
	ScopedContext scopedContext( m_context );

	if( m_attributeStack.size() )
	{
		m_attributeStack.pop();
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::attributeEnd", "No matching attributeBegin call." );
	}
	RiAttributeEnd();
}

void IECoreRI::RendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	ScopedContext scopedContext( m_context );
	SetAttributeHandlerMap::iterator it = m_setAttributeHandlers.find( name );
	if( it!=m_setAttributeHandlers.end() )
	{
		(this->*(it->second))( name, value );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			const CompoundData *compoundValue = runTimeCast<const CompoundData>( value );
			if( !compoundValue )
			{
				msg( Msg::Warning, "IECoreRI::RendererImplementation::setAttribute", format( "Expected CompoundData for name matching \"ri:*\" but got \"%s\"." ) % value->typeName() );
			}
			else
			{
				ParameterList pl( compoundValue->readable() );
				RiAttributeV( (char *)name.c_str() + 3, pl.n(), pl.tokens(), pl.values() );
			}
		}
		else
		{
			string s1( name, 3, i-3 );
			string s2( name, i+1 );
			ParameterList pl( s2, value );
			RiAttributeV( (char *)s1.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		ParameterList pl( s, value );
		RiAttributeV( "user", pl.n(), pl.tokens(), pl.values() );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore attributes prefixed for some other RendererImplementation
		return;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setAttribute", format( "Unknown attribute \"%s\"." ) % name );
	}
}

void IECoreRI::RendererImplementation::setShadingRateAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatDataPtr f = runTimeCast<const FloatData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:shadingRate attribute expects a FloatData value." );
		return;
	}

	RiShadingRate( f->readable() );
}

void IECoreRI::RendererImplementation::setMatteAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:matte attribute expects a BoolData value." );
		return;
	}

	RiMatte( f->readable() ? 1 : 0 );
}

void IECoreRI::RendererImplementation::setColorAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstColor3fDataPtr f = runTimeCast<const Color3fData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:color attribute expects a Color3fData value." );
		return;
	}

	RiColor( (RtFloat *)&(f->readable().x) );
}

void IECoreRI::RendererImplementation::setOpacityAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstColor3fDataPtr f = runTimeCast<const Color3fData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:opacity attribute expects a Color3fData value." );
		return;
	}

	RiOpacity( (RtFloat *)&(f->readable().x) );
}

void IECoreRI::RendererImplementation::setSidesAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstIntDataPtr f = runTimeCast<const IntData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:sides attribute expects an IntData value." );
		return;
	}

	RiSides( f->readable() );
}

void IECoreRI::RendererImplementation::setDoubleSidedAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "doubleSided attribute expects a BoolData value." );
		return;
	}

	RiSides( f->readable() ? 2 : 1 );
}

void IECoreRI::RendererImplementation::setRightHandedOrientationAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "rightHandedOrientation attribute expects a BoolData value." );
		return;
	}
	if( f->readable() )
	{
		RiOrientation( "rh" );
	}
	else
	{
		RiOrientation( "lh" );
	}
}

void IECoreRI::RendererImplementation::setGeometricApproximationAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatDataPtr f = runTimeCast<const FloatData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects an IntData value." ) % name );
		return;
	}
	string s = string( name, name.find_last_of( ":" ) + 1 );
	to_lower( s );
	RiGeometricApproximation( (char *)s.c_str(), f->readable() );
}

void IECoreRI::RendererImplementation::setNameAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstStringDataPtr f = runTimeCast<const StringData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects a StringData value." ) % name );
		return;
	}
	ParameterList pl( "name", f );
	RiAttributeV( "identifier", pl.n(), pl.tokens(), pl.values() );
}

void IECoreRI::RendererImplementation::setSubsurfaceAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstCompoundDataPtr c = runTimeCast<const CompoundData>( d );
	if( !c )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects a CompoundData value." ) % name );
		return;
	}

	CompoundDataMap::const_iterator it = c->readable().find( "visibility" );
	if( it==c->readable().end() )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expected to contain a \"visibility\" value." ) % name );
		return;
	}

	ConstStringDataPtr v = runTimeCast<const StringData>( it->second );
	if( !v )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute \"visibility\" value must be of type StringData." ) % name );
		return;
	}

	const char *ssv = v->readable().c_str();
	RiAttribute( "visibility", "string subsurface", &ssv, 0 );

	CompoundDataPtr ssParms = c->copy();
	ssParms->writable().erase( "visibility" );
	ParameterList pl( ssParms->readable() );
	RiAttributeV( "subsurface", pl.n(), pl.tokens(), pl.values() );
}

void IECoreRI::RendererImplementation::setDetailAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBox3fDataPtr b = runTimeCast<const Box3fData>( d );
	if( !b )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailAttribute", format( "%s attribute expects a Box3fData value." ) % name );
		return;
	}
	
	RtBound bound;
	convert( b->readable(), bound );
	RiDetail( bound );
}

void IECoreRI::RendererImplementation::setDetailRangeAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatVectorDataPtr f = runTimeCast<const FloatVectorData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailRangeAttribute", format( "%s attribute expects a FloatVectorData value." ) % name );
		return;
	}
	
	const vector<float> &values = f->readable();
	if( values.size()!=4 )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailRangeAttribute", format( "Value must contain 4 elements (found %d)." ) %  values.size() );
		return;
	}
	
	RiDetailRange( values[0], values[1], values[2], values[3] );
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getAttribute( const std::string &name ) const
{
	ScopedContext scopedContext( m_context );
	GetAttributeHandlerMap::const_iterator it = m_getAttributeHandlers.find( name );
	if( it!=m_getAttributeHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::getAttribute", format( "Expected attribute name matching \"ri:*:*\" but got \"%s\"." ) % name );
			return 0;
		}
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		memset( result, 0, 16 * sizeof( RtFloat ) ); // 3delight has a bug where it'll try to free some random part of memory if this is not null (v 7.0.54)
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxAttribute( (char *)name.c_str()+3, result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxAttribute( (char *)name.c_str(), result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// silently ignore attributes prefixed for some other RendererImplementation
		return 0;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::getAttribute", format( "Unknown attribute \"%s\"." ) % name );
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getShadingRateAttribute( const std::string &name ) const
{
	float result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "ShadingRate", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			return new FloatData( result );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getMatteAttribute( const std::string &name ) const
{
	float result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Matte", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			return new BoolData( result > 0.0f );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getDoubleSidedAttribute( const std::string &name ) const
{
	float result = 2;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Sides", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			if( result==1 )
			{
				return new BoolData( false );
			}
			else
			{
				return new BoolData( true );
			}
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getRightHandedOrientationAttribute( const std::string &name ) const
{
	char *result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Ri:Orientation", &result, sizeof( char * ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoStringV && resultCount==1 )
		{
			// it'd be nice if we were just told "rh" or "lh"
			if( 0==strcmp( result, "rh" ) )
			{
				return new BoolData( true );
			}
			else if( 0==strcmp( result, "lh" ) )
			{
				return new BoolData( false );
			}
			// but in practice we seem to be told "outside" or "inside"
			else
			{
				bool determinantNegative = determinant( getTransform() ) < 0.0f;
				if( 0==strcmp( result, "outside" ) )
				{
					return new BoolData( !determinantNegative );
				}
				else if( 0==strcmp( result, "inside" ) )
				{
					return new BoolData( determinantNegative );
				}
			}
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getNameAttribute( const std::string &name ) const
{
	char *result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "identifier:name", (char *)&result, sizeof( char * ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoStringV && resultCount==1 )
		{
			return new StringData( result );
		}
	}
	return 0;
}

IECore::CachedReaderPtr IECoreRI::RendererImplementation::defaultShaderCache()
{
	static IECore::CachedReaderPtr g_defaultShaderCache = new CachedReader(
		SearchPath( getenv( "DL_SHADERS_PATH" ) ? getenv( "DL_SHADERS_PATH" ) : "", ":" ),
		g_shaderCacheSize
	);
	return g_defaultShaderCache;
}

void IECoreRI::RendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstShaderPtr s = 0;
	try
	{
		s = runTimeCast<const Shader>( m_shaderCache->read( name + ".sdl" ) );
		if( !s )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\"." ) % name );
			return;
		}
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\" - %s" ) % name % e.what() );
		return;
		// we don't want exceptions to halt rendering - we'd rather just report the error below
	}
	catch( ... )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\" - an unknown exception was thrown" ) % name );
		return;
	}

	// if we are here then we loaded a shader ok. now process the parameters for it and make the ri call.

	AttributeState &state = m_attributeStack.top();
	state.primVarTypeHints.clear();
	CompoundDataMap::const_iterator it = s->blindData()->readable().find( "ri:parameterTypeHints" );
	if( it!=s->blindData()->readable().end() )
	{
		if( ConstCompoundDataPtr h = runTimeCast<const CompoundData>( it->second ) )
		{
			for( it=h->readable().begin(); it!=h->readable().end(); it++ )
			{
				if( it->second->typeId()==StringData::staticTypeId() )
				{
					state.primVarTypeHints.insert( std::pair<string, string>( it->first, staticPointerCast<const StringData>( it->second )->readable() ) );
				}
			}
		}
	}
	if( type=="surface" || type=="ri:surface" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiSurfaceV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="displacement" || type=="ri:displacement" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiDisplacementV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="atmosphere" || type=="ri:atmosphere" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiAtmosphereV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="interior" || type=="ri:interior" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiInteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="exterior" || type=="ri:exterior" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiExteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="deformation" || type=="ri:deformation" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiDeformationV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="shader" || type=="ri:shader" )
	{
		const StringData *handleData = 0;
		CompoundDataMap::const_iterator it = parameters.find( "__handle" );
		if( it!=parameters.end() )
		{
			handleData = runTimeCast<const StringData>( it->second );
		}
		if( !handleData )
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::shader", "Must specify StringData \"__handle\" parameter for coshaders." );
			return;
		}
		CompoundDataMap parametersCopy = parameters;
		parametersCopy.erase( "__handle" );
		ParameterList pl( parametersCopy, &state.primVarTypeHints );
		RiShaderV( (char *)name.c_str(), (char *)handleData->readable().c_str(), pl.n(), pl.tokens(), pl.values() );
	}
}

void IECoreRI::RendererImplementation::clear_typehint_map()
{
	AttributeState &state = m_attributeStack.top();
	state.primVarTypeHints.clear();
}
void IECoreRI::RendererImplementation::set_typehint( const std::string& k, const std::string& v )
{
	AttributeState &state = m_attributeStack.top();
	state.primVarTypeHints[k] = v;
}

void IECoreRI::RendererImplementation::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	IECore::CompoundDataMap parametersCopy = parameters;
	parametersCopy["__handleid"] = new StringData( handle );

	AttributeState &state = m_attributeStack.top();

	if( state.primVarTypeHints.size() )
	{
		ParameterList pl( parametersCopy, &state.primVarTypeHints );
		RiLightSourceV( const_cast<char *>(name.c_str()), pl.n(), pl.tokens(), pl.values() );
	}
	else
	{
		ParameterList pl( parametersCopy );
		RiLightSourceV( const_cast<char *>(name.c_str()), pl.n(), pl.tokens(), pl.values() );
	}

}

void IECoreRI::RendererImplementation::illuminate( const std::string &lightHandle, bool on )
{
	ScopedContext scopedContext( m_context );
	RiIlluminate( (RtLightHandle)lightHandle.c_str(), on );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::motionBegin( const std::set<float> &times )
{
	m_delayedMotionTimes.resize( times.size() );
	unsigned int i = 0;
	for( set<float>::const_iterator it = times.begin(); it!=times.end(); it++ )
	{
		m_delayedMotionTimes[i++] = *it;
	}
}

void IECoreRI::RendererImplementation::delayedMotionBegin()
{
	if( m_delayedMotionTimes.size() )
	{
		RiMotionBeginV( m_delayedMotionTimes.size(), &*(m_delayedMotionTimes.begin() ) );
		m_delayedMotionTimes.clear();
		m_inMotion = true;
	}
}

void IECoreRI::RendererImplementation::motionEnd()
{
	ScopedContext scopedContext( m_context );
	RiMotionEnd();
	m_inMotion = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiPointsV( numPoints, pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiDiskV( z, radius, thetaMax, pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );

	// emit basis if we're not in a motion block right now
	if( !m_inMotion )
	{
		RtMatrix b;
		convert( basis.matrix, b );
		RiBasis( b, basis.step, b, basis.step );
	}

	// then emit any overdue motionbegin calls.
	delayedMotionBegin();

	// finally emit the curves

	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	vector<int> &numVerticesV = const_cast<vector<int> &>( numVertices->readable() );

	RiCurvesV(	(char *)( basis==CubicBasisf::linear() ? "linear" : "cubic" ),
				numVerticesV.size(), &*( numVerticesV.begin() ),
				(char *)( periodic ? "periodic" : "nonperiodic" ),
				pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();

#ifdef IECORE_WITH_FREETYPE
	IECore::FontPtr f = 0;
	FontMap::const_iterator it = m_fonts.find( font );
	if( it!=m_fonts.end() )
	{
		f = it->second;
	}
	else
	{
		string file = m_fontSearchPath.find( font ).string();
		if( file!="" )
		{
			try
			{
				f = new IECore::Font( file );
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Warning, "Renderer::text", e.what() );
			}
		}
		m_fonts[font] = f;
	}

	if( !f )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::text", boost::format( "Font \"%s\" not found." ) % font );
		return;
	}

	f->setKerning( kerning );
	f->meshGroup( text )->render( this );
#else
	IECore::msg( IECore::Msg::Warning, "Renderer::text", "IECore was not built with FreeType support." );
#endif // IECORE_WITH_FREETYPE
}

void IECoreRI::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiSphereV( radius, zMin * radius, zMax * radius, thetaMax, pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();

	msg( Msg::Warning, "IECoreRI::RendererImplementation::image", "Not implemented" );
}

void IECoreRI::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();

	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );

	if( interpolation=="catmullClark" )
	{
		int numNames = 1;
		const char *names[] = { "interpolateboundary", 0, 0, 0, 0, 0 };
		int defaultNArgs[] = { 0, 0 };
		const int *nArgs = defaultNArgs;
		const float *floats = 0;
		const int *integers = 0;
		
		IECore::PrimitiveVariableMap::const_iterator tagIt = primVars.find( "tags" );
		if( tagIt != primVars.end() )
		{
			const CompoundData *tagsData = runTimeCast<const CompoundData>( tagIt->second.data.get() );
			if( tagsData )
			{
				const StringVectorData *namesData = tagsData->member<const StringVectorData>( "names" );
				const IntVectorData *nArgsData = tagsData->member<const IntVectorData>( "nArgs" );
				const FloatVectorData *floatsData = tagsData->member<const FloatVectorData>( "floats" );
				const IntVectorData *integersData = tagsData->member<const IntVectorData>( "integers" );
				if( namesData && nArgsData && floatsData && integersData )
				{
					// we only have room for 6 names in the array above as we're trying to avoid having to dynamically allocate
					// any memory here - should be ok as there are only 5 tag types currently specified by 3delight.
					numNames = std::min( (int)namesData->readable().size(), 6 );
					for( int i=0; i<numNames; i++ )
					{
						names[i] = namesData->readable()[i].c_str();
					}
					nArgs = &(nArgsData->readable()[0]);
					floats = &(floatsData->readable()[0]);
					integers = &(integersData->readable()[0]);
				}
				else
				{
					msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Primitive variable \"tags\" does not contain the required members - ignoring." );
				}
			}
			else
			{
				msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Primitive variable \"tags\" is not of type CompoundData - ignoring." );
			}
		}

		RiSubdivisionMeshV( "catmull-clark", vertsPerFace->readable().size(), (int *)&vertsPerFace->readable()[0], (int *)&vertIds->readable()[0],
			numNames, (char **)names, (int *)nArgs, (int *)integers, (float *)floats,
			pv.n(), pv.tokens(), pv.values() );

		return;
	}

	if( interpolation!="linear" )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", boost::format( "Unsupported interpolation type \"%s\" - rendering as polygons." ) % interpolation );
	}

	tbb::queuing_rw_mutex::scoped_lock lock( g_nLoopsMutex, false /* read only */ );
	if( g_nLoops.size()<vertsPerFace->readable().size() )
	{
		lock.upgrade_to_writer();
			if( g_nLoops.size()<vertsPerFace->readable().size() ) // checking again as i think g_nLoops may have been resized by another thread getting the write lock first
			{
				g_nLoops.resize( vertsPerFace->readable().size(), 1 );
			}
		lock.downgrade_to_reader();
	}

	vector<int> &vertsPerFaceV = const_cast<vector<int> &>( vertsPerFace->readable() );
	vector<int> &vertIdsV = const_cast<vector<int> &>( vertIds->readable() );

	RiPointsGeneralPolygonsV( vertsPerFaceV.size(), &*(g_nLoops.begin()), &*(vertsPerFaceV.begin()), &*(vertIdsV.begin()),
		pv.n(), pv.tokens(), pv.values() );

}

void IECoreRI::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin();

	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiNuPatchV(
		uKnot->readable().size() - uOrder, // nu
		uOrder,
		const_cast<float *>( &(uKnot->readable()[0]) ),
		uMin,
		uMax,
		vKnot->readable().size() - vOrder, // nv
		vOrder,
		const_cast<float *>( &(vKnot->readable()[0]) ),
		vMin,
		vMax,
		pv.n(), pv.tokens(), pv.values()
	);
}

void IECoreRI::RendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );

	bool uLinear = uBasis == CubicBasisf::linear();
	bool vLinear = vBasis == CubicBasisf::linear();

	if ( uLinear != vLinear )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Mismatched u/v basis.");
		return;
	}

	if( !m_inMotion )
	{
		RtMatrix ub, vb;
		convert( uBasis.matrix, ub );
		convert( vBasis.matrix, vb );
		RiBasis( ub, uBasis.step, vb, vBasis.step );
	}

	delayedMotionBegin();

	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	const char *type = uLinear ? "bilinear" : "bicubic";
	RiPatchMeshV(
		const_cast< char * >( type ),
		nu,
		(char *)( uPeriodic ? "periodic" : "nonperiodic" ),
		nv,
		(char *)( vPeriodic ? "periodic" : "nonperiodic" ),
		pv.n(), pv.tokens(), pv.values()
	);
}

void IECoreRI::RendererImplementation::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );

	if( type=="teapot" || type=="ri:teapot" )
	{
		delayedMotionBegin();
		RiGeometry( "teapot", 0 );
	}
	else
	{
		delayedMotionBegin();
		msg( Msg::Warning, "IECoreRI::RendererImplementation::geometry", format( "Unsupported geometry type \"%s\"." ) % type );
	}
}

void IECoreRI::RendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	ScopedContext scopedContext( m_context );

	Imath::Box3f bound = proc->bound();
	if( bound.isEmpty() )
	{
		return;
	}

	RtBound riBound;
	riBound[0] = bound.min.x;
	riBound[1] = bound.max.x;
	riBound[2] = bound.min.y;
	riBound[3] = bound.max.y;
	riBound[4] = bound.min.z;
	riBound[5] = bound.max.z;

	ProceduralData *data = new ProceduralData;
	data->procedural = proc;
	data->parentRenderer = this;
	
	RiProcedural( data, riBound, procSubdivide, procFree );
}

void IECoreRI::RendererImplementation::procSubdivide( void *data, float detail )
{
	ProceduralData *proceduralData = reinterpret_cast<ProceduralData *>( data );
	// we used to try to use the same IECoreRI::Renderer that had the original procedural() call issued to it.
	// this turns out to be incorrect as each procedural subdivide invocation is in a new RtContextHandle
	// and the original renderer would be trying to switch to its original context. so we just create a temporary
	// renderer which doesn't own a context and therefore use the context 3delight has arranged to call subdivide with.
	// we do however share SharedData with the parent renderer, so that we can share instances among procedurals.
	IECoreRI::RendererPtr renderer = new IECoreRI::Renderer( new RendererImplementation( proceduralData->parentRenderer ) );
	proceduralData->procedural->render( renderer );
}

void IECoreRI::RendererImplementation::procFree( void *data )
{
	ProceduralData *proceduralData = reinterpret_cast<ProceduralData *>( data );
	delete proceduralData;
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
#ifdef IECORERI_WITH_OBJECTBEGINV
	// we get to choose the name for the object
	const char *tokens[] = { "__handleid", "scope" };
	const char *namePtr = name.c_str();
	const char *scope = "world";
	const void *values[] = { &namePtr, &scope };
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
	m_sharedData->objectHandles[name] = RiObjectBeginV( 2, (char **)&tokens, (void **)&values );
#else
	// we have to put up with a rubbish name
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
	m_sharedData->objectHandles[name] = RiObjectBegin();
#endif
}

void IECoreRI::RendererImplementation::instanceEnd()
{
	ScopedContext scopedContext( m_context );
	RiObjectEnd();
}

void IECoreRI::RendererImplementation::instance( const std::string &name )
{
	ScopedContext scopedContext( m_context );
	
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex, false /* read only lock */ );
	SharedData::ObjectHandleMap::const_iterator hIt = m_sharedData->objectHandles.find( name );
	if( hIt==m_sharedData->objectHandles.end() )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::instance", boost::format( "No object named \"%s\" available for instancing." ) % name );
		return;
	}
	RiObjectInstance( const_cast<void *>( hIt->second ) );
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreRI::RendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
   ScopedContext scopedContext( m_context );

	CommandHandlerMap::iterator it = m_commandHandlers.find( name );
	if( it==m_commandHandlers.end() )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::command", boost::format( "Unknown command \"%s\"" ) % name );
		return 0;
	}
	return (this->*(it->second))( name, parameters );
}

IECore::DataPtr IECoreRI::RendererImplementation::readArchiveCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr nameData;
	CompoundDataMap::const_iterator it = parameters.find( "name" );
	if( it!=parameters.end() )
	{
		nameData = runTimeCast<StringData>( it->second );
	}
	if( !nameData )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:readArchive command expects a StringData value called \"name\"." );
		return 0;
	}
	RiReadArchiveV( (char *)nameData->readable().c_str(), 0, 0, 0, 0 );
	return 0;
}

IECore::DataPtr IECoreRI::RendererImplementation::archiveRecordCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr typeData;
	ConstStringDataPtr recordData;
	CompoundDataMap::const_iterator typeIt = parameters.find( "type" );
	CompoundDataMap::const_iterator recordIt = parameters.find( "record" );
	if( typeIt!=parameters.end() )
	{
		typeData = runTimeCast<StringData>( typeIt->second );
	}
	if( recordIt!=parameters.end() )
	{
		recordData = runTimeCast<StringData>( recordIt->second );
	}

	if( !(typeData && recordData) )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:archiveRecord command expects StringData values called \"type\" and \"record\"." );
		return 0;
	}

	// if there are printf style format specifiers in the record then we're in trouble - we're about to pass them through a c interface which
	// isn't type safe, and not provide any additional arguments for them. try to avoid that by using boost format to catch the problem.
	try
	{
		string tested = boost::format( recordData->readable() ).str();
		RiArchiveRecord( const_cast<char *>( typeData->readable().c_str() ), const_cast<char *>( tested.c_str() ) );
	}
	catch( ... )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:archiveRecord \"record\" parameter appears to contain printf format specifiers." );
	}
	return 0;
}

IECore::DataPtr IECoreRI::RendererImplementation::illuminateCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr handleData = 0;
	ConstBoolDataPtr stateData = 0;
	CompoundDataMap::const_iterator handleIt = parameters.find( "handle" );
	CompoundDataMap::const_iterator stateIt = parameters.find( "state" );
	if( handleIt!=parameters.end() )
	{
		handleData = runTimeCast<StringData>( handleIt->second );
	}
	if( stateIt!=parameters.end() )
	{
		stateData = runTimeCast<BoolData>( stateIt->second );
	}

	if( !(handleData && stateData) )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:illuminate command expects a StringData value called \"handle\" and a BoolData value called \"state\"." );
		return 0;
	}

	RiIlluminate( (void *)handleData->readable().c_str(), stateData->readable() );
	return 0;
}
