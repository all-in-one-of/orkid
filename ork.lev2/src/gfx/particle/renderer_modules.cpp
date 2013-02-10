////////////////////////////////////////////////////////////////
// Orkid Media Engine
// Copyright 1996-2012, Michael T. Mayers.
// Distributed under the Boost Software License - Version 1.0 - August 17, 2003
// see http://www.boost.org/LICENSE_1_0.txt
////////////////////////////////////////////////////////////////

#include <ork/pch.h>
#include <ork/reflect/RegisterProperty.h>
#include <ork/lev2/gfx/gfxmodel.h>
#include <ork/lev2/gfx/texman.h>
#include <ork/lev2/gfx/renderer.h>
#include <ork/gfx/camera.h>
#include <ork/lev2/gfx/gfxprimitives.h>
#include <ork/lev2/gfx/gfxmaterial_test.h>
#include <ork/reflect/enum_serializer.h>

#include <ork/reflect/DirectObjectPropertyType.hpp>
#include <ork/reflect/DirectObjectMapPropertyType.hpp>
#include <ork/kernel/orklut.hpp>
#include <ork/lev2/gfx/particle/modular_particles.h>
#include <ork/lev2/lev2_asset.h>
#include <ork/kernel/fixedlut.hpp>
#include <ork/kernel/string/string.h>
//#include <pkg/ent/PerfController.h>

INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::RendererModule, "psys::RendererModule");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::SpriteRenderer, "psys::SpriteRenderer");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::StreakRenderer, "psys::StreakRenderer");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::ModelRenderer, "psys::ModelRenderer");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::MaterialBase, "psys::MaterialBase");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::TextureMaterial, "psys::TextureMaterial");
INSTANTIATE_TRANSPARENT_RTTI(ork::lev2::particle::VolTexMaterial, "psys::VolTexMaterial");

///////////////////////////////////////////////////////////////////////////////
namespace ork { namespace lev2 { namespace particle {
///////////////////////////////////////////////////////////////////////////////

void RendererModule::Describe()
{	RegisterObjInpPlug ( RendererModule, Input );
	//static const char* EdGrpStr =
	//	        "grp://BasicRenderer Input Gradient";
	//reflect::AnnotateClassForEditor<BasicRendererModule>( "editor.prop.groups", EdGrpStr );
}
RendererModule::RendererModule()
	: ConstructInpPlug( Input, dataflow::EPR_UNIFORM, gNoCon )
{	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SpriteRenderer::Describe()
{	RegisterFloatXfPlug( SpriteRenderer, Size, -100.0f, 100.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, Rot, -180.0f, 180.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, GradientIntensity, 0.0f, 10.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, AnimFrame, 0.0f, 1.0f, ged::OutPlugChoiceDelegate );

	RegisterFloatXfPlug( SpriteRenderer, NoiseAmp0, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseAmp1, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseAmp2, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );

	RegisterFloatXfPlug( SpriteRenderer, NoiseFreq0, -2.0f, 2.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseFreq1, -2.0f, 2.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseFreq2, -2.0f, 2.0f, ged::OutPlugChoiceDelegate );

	RegisterFloatXfPlug( SpriteRenderer, NoiseShift0, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseShift1, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( SpriteRenderer, NoiseShift2, -1.0f, 1.0f, ged::OutPlugChoiceDelegate );

	/////////////////
	ork::reflect::RegisterProperty("ActiveMaterial", &SpriteRenderer::mActiveMaterial);
	ork::reflect::RegisterMapProperty( "Materials", & SpriteRenderer::mMaterials );
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >("Materials", "editor.factorylistbase", "psys::MaterialBase" );
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >("Materials", "editor.map.policy.impexp", "true" );
	/////////////////

	/////////////////
	ork::reflect::RegisterProperty("ActiveGradient", &SpriteRenderer::mActiveGradient);
	ork::reflect::RegisterMapProperty( "Gradients", & SpriteRenderer::mGradients );
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >("Gradients", "editor.factorylistbase", "GradientV4" );
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >("Gradients", "editor.map.policy.impexp", "true" );
	/////////////////

	ork::reflect::RegisterProperty("Alignment", &SpriteRenderer::meAlignment);
	ork::reflect::RegisterProperty("BlendMode", &SpriteRenderer::meBlendMode);

	ork::reflect::RegisterProperty("AnimTexDim", &SpriteRenderer::miAnimTexDim);
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "AnimTexDim", "editor.range.min", "1" );
	ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "AnimTexDim", "editor.range.max", "8" );
	//ork::reflect::RegisterProperty( "VolumeTexture", & SpriteRenderer::GetVolumeTextureAccessor, & SpriteRenderer::SetVolumeTextureAccessor );

	//ork::reflect::RegisterProperty( "Material", & SpriteRenderer::GetMaterial );

	ork::reflect::RegisterProperty("DepthSort", & SpriteRenderer::mbSort );
	//ork::reflect::RegisterProperty("ImgSeqBegin", & SpriteRenderer::miImgSequenceBegin );
	//ork::reflect::RegisterProperty("ImgSeqEnd", & SpriteRenderer::miImgSequenceEnd );
	//ork::reflect::RegisterProperty("ImgSeq", & SpriteRenderer::mbImageSequence );

	/////////////////
	//ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("Gradient", "editor.class", "ged.factory.gradient" );
	/////////////////
	ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("Alignment", "editor.class", "ged.factory.enum" );
	ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("BlendMode", "editor.class", "ged.factory.enum" );
	/////////////////
	//ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("VolumeTexture", "editor.class", "ged.factory.assetlist" );
	//ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("VolumeTexture", "editor.assettype", "lev2tex" );
	//ork::reflect::AnnotatePropertyForEditor<SpriteRenderer>("VolumeTexture", "editor.assetclass", "lev2tex");
	/////////////////
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "ImgSeqBegin", "editor.range.min", "0" );
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "ImgSeqBegin", "editor.range.max", "999" );
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "ImgSeqEnd", "editor.range.min", "0" );
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "ImgSeqEnd", "editor.range.max", "999" );
	/////////////////
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "AnimTexDim", "editor.range.min", "1" );
	//ork::reflect::AnnotatePropertyForEditor< SpriteRenderer >( "AnimTexDim", "editor.range.max", "16" );
	static const char* EdGrpStr =
		        "grp://Base DepthSort Alignment Rot Size BlendMode "
				"grp://Gradient ActiveGradient GradientIntensity Gradients "
				"grp://Material ActiveMaterial AnimFrame Materials AnimTexDim ";
				//"grp://Image VolumeTexture Texture ImgSeq ImgSeqBegin ImgSeqEnd ImgAnimTexDim AnimTexFrame "
				//"grp://Noise NoiseAmp0 NoiseAmp1 NoiseAmp2 NoiseFreq0 NoiseFreq1 NoiseFreq2 NoiseShift0 NoiseShift1 NoiseShift2  ";
	reflect::AnnotateClassForEditor<SpriteRenderer>( "editor.prop.groups", EdGrpStr );
}
///////////////////////////////////////////////////////////////////////////////

SpriteRenderer::SpriteRenderer()
	: meBlendMode(ork::lev2::EBLENDING_ALPHA)
	, ConstructOutPlug(UnitAge, dataflow::EPR_VARYING1)
	, ConstructOutPlug(PtcRandom, dataflow::EPR_VARYING1)
	, ConstructInpPlug( GradientIntensity, dataflow::EPR_UNIFORM, mfGradientIntensity )
	, ConstructInpPlug( Rot, dataflow::EPR_VARYING1, mfRot )
	, ConstructInpPlug( Size, dataflow::EPR_VARYING1, mfSize )
	, ConstructInpPlug( AnimFrame, dataflow::EPR_VARYING1, mfAnimFrame )

	, ConstructInpPlug( NoiseAmp0, dataflow::EPR_VARYING1, mfNoiseAmp0 )
	, ConstructInpPlug( NoiseAmp1, dataflow::EPR_VARYING1, mfNoiseAmp1 )
	, ConstructInpPlug( NoiseAmp2, dataflow::EPR_VARYING1, mfNoiseAmp2 )

	, ConstructInpPlug( NoiseFreq0, dataflow::EPR_VARYING1, mfNoiseFreq0 )
	, ConstructInpPlug( NoiseFreq1, dataflow::EPR_VARYING1, mfNoiseFreq1 )
	, ConstructInpPlug( NoiseFreq2, dataflow::EPR_VARYING1, mfNoiseFreq2 )

	, ConstructInpPlug( NoiseShift0, dataflow::EPR_VARYING1, mfNoiseShift0 )
	, ConstructInpPlug( NoiseShift1, dataflow::EPR_VARYING1, mfNoiseShift1 )
	, ConstructInpPlug( NoiseShift2, dataflow::EPR_VARYING1, mfNoiseShift2 )

	, mOutDataUnitAge(0.0f)
	, meAlignment( EPIA_BILLBOARD )
	//, mVolumeTexture( 0 )
	, mfSize(1.0f)
	, mfRot(0.0f)
	, mfGradientIntensity(1.0f)
	, mfAnimFrame(0.0f)
	, miAnimTexDim(1)
	//, miImgSequenceBegin(0)
	//, miImgSequenceEnd(0)
	, mbSort(false)
	//, mbImageSequence(false)
	//, mbImageSequenceOK(false)
	//, miImageFrame(0)
{	
}

void SpriteRenderer::DoLink()
{
	//printf( "Checking for IMGSEQUENCE mbImageSequence<%d> Texture<%p>\n", mbImageSequence, mTexture );
	/*if( mbImageSequence && mTexture && mTexture->IsLoaded() )
	{
		const file::Path& BaseAsset = mTexture->GetName().c_str();
		printf( "BaseAssetPath<%s>\n", BaseAsset.c_str() );
		const char* srch = strstr( BaseAsset.c_str(), "frame0000" );
		if( srch )
		{
			printf( "IMGSEQUENCE DETECTED\n" );
			for( int i=miImgSequenceBegin; i<=miImgSequenceEnd; i++ )
			{
				std::string assetname = BaseAsset.c_str();
				std::string seqidx = CreateFormattedString("%04d",i);
				OrkSTXFindAndReplace<std::string>( assetname, "0000", seqidx );
				
				ork::lev2::TextureAsset* passet = ork::asset::AssetManager<ork::lev2::TextureAsset>::Load( assetname.c_str() );
			
				if( passet )
				{
					ork::lev2::Texture* ptex = passet->GetTexture();
					if( ptex )
					{
						printf( "AddedAsset<%p>\n", ptex );
						mSequenceTextures.push_back(passet);
					}
				}
			}
			mbImageSequenceOK = true;
		}
		
	}*/
}
///////////////////////////////////////////////////////////////////////////////
//void SpriteRenderer::SetVolumeTextureAccessor( ork::rtti::ICastable* const & tex)
//{	mVolumeTexture = tex ? ork::rtti::autocast( tex ) : 0;
//}
///////////////////////////////////////////////////////////////////////////////
//void SpriteRenderer::GetVolumeTextureAccessor( ork::rtti::ICastable* & tex) const
//{	tex = mVolumeTexture;
//}
///////////////////////////////////////////////////////////////////////////////
dataflow::inplugbase* SpriteRenderer::GetInput(int idx)
{	dataflow::inplugbase* rval = 0;
	switch(idx)
	{	case 0:	rval = & mPlugInpInput;	break;
		case 1:	rval = & mPlugInpSize;	break;
		case 2:	rval = & mPlugInpRot;	break;
		case 3: rval = & mPlugInpGradientIntensity; break;
		case 4: rval = & mPlugInpAnimFrame; break;

		case 5: rval = & mPlugInpNoiseAmp0; break;
		case 6: rval = & mPlugInpNoiseAmp1; break;
		case 7: rval = & mPlugInpNoiseAmp2; break;

		case 8: rval = & mPlugInpNoiseFreq0; break;
		case 9: rval = & mPlugInpNoiseFreq1; break;
		case 10: rval = & mPlugInpNoiseFreq2; break;

		case 11: rval = & mPlugInpNoiseShift0; break;
		case 12: rval = & mPlugInpNoiseShift1; break;
		case 13: rval = & mPlugInpNoiseShift2; break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////
dataflow::outplugbase* SpriteRenderer::GetOutput(int idx)
{	dataflow::outplugbase* rval = 0;
	switch(idx)
	{	
		case 0:	rval = & OutPlugName(UnitAge);		break;
		case 1:	rval = & OutPlugName(PtcRandom);	break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////

ParticlePoolRenderBuffer::ParticlePoolRenderBuffer()
	: miMaxParticles(0)
	, miNumParticles(0)
	, mpParticles(0)
{
}
ParticlePoolRenderBuffer::~ParticlePoolRenderBuffer()
{
	if( mpParticles )
	{
		delete[] mpParticles;
	}
}

int NextHighestPowerOfTwo( int inp )
{
	int outp = inp;
	outp--;
	outp |= outp >> 1;
	outp |= outp >> 2;
	outp |= outp >> 4;
	outp |= outp >> 8;
	outp |= outp >> 16;
	outp++;
	return outp;
}

void ParticlePoolRenderBuffer::SetCapacity( int icap )
{
	if( icap > miMaxParticles )
	{
		if( mpParticles ) delete[] mpParticles;
		miMaxParticles = NextHighestPowerOfTwo( icap );
		mpParticles = new BasicParticle[ miMaxParticles ];
	}
}
void ParticlePoolRenderBuffer::Update( const Pool<BasicParticle>& the_pool )
{
	int icnt = the_pool.GetNumAlive();
	SetCapacity( icnt );
	miNumParticles = icnt;
	
	for( int i=0; i<icnt; i++ )
	{
		const ork::lev2::particle::BasicParticle* ptcl = the_pool.GetActiveParticle(i);
		mpParticles[i] = *ptcl;
	}
}

///////////////////////////////////////////////////////////////////////////////

ork::lev2::GfxTarget* gtarg = 0;
 
void SpriteRenderer::Compute( float dt )
{
	//miImageFrame++;
	
	MaterialBase* pMTLBASE = 0;
	if( mpTemplateModule )
	{
		SpriteRenderer* ptemplate_module = 0;
		ptemplate_module = rtti::autocast(mpTemplateModule);
		const PoolString& active_m = ptemplate_module->mActiveMaterial;
		orklut<PoolString,ork::Object*>::const_iterator itM = mMaterials.find(active_m);
		if( itM != mMaterials.end() )
		{
			MaterialBase* pMTLBASE = rtti::autocast( itM->second );
			float ftexframe = mPlugInpAnimFrame.GetValue();
			if( pMTLBASE )
			{
				pMTLBASE->Update(ftexframe);
			}
		}
	}
} 

///////////////////////////////////////////////////////////////////////////////

const Pool<BasicParticle>* RendererModule::GetPool()
{
	return mPlugInpInput.GetValue().mPool;
}

///////////////////////////////////////////////////////////////////////////////

bool SpriteRenderer::DoNotify(const ork::event::Event *event)
{
/*	if( const ork::ent::PerfControlEvent* pce = rtti::autocast(event) )
	{	const char* keyname = pce->mTarget.c_str();
		////////////////////
		if( 0 == strcmp(keyname,"ActiveGradient") )
			mActiveGradient = pce->ValueAsPoolString();
		////////////////////
		if( 0 == strcmp(keyname,"ActiveMaterial") )
			mActiveMaterial = pce->ValueAsPoolString();
		////////////////////
		//if( 0 == strcmp(keyname,"GradientIntensity") )
		//	mActiveMaterial = pce->ValueAsPoolString();
		////////////////////
		return true;
	}
	else if( const ork::ent::PerfSnapShotEvent* psse = rtti::autocast(event) )
	{
		////////////////////
		psse->PushNode( "ActiveGradient" );
			psse->AddTarget( mActiveGradient.c_str() );
		psse->PopNode();
		////////////////////
		psse->PushNode( "ActiveMaterial" );
			psse->AddTarget( mActiveMaterial.c_str() );
		psse->PopNode();
		////////////////////
		//psse->PushNode( "GradientIntensity" );
		//	AddTarget( mActiveMaterial.c_str() );
		//psse->PopNode();
		////////////////////
		return true;
	}*/
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void SpriteRenderer::Render(const CMatrix4& mtx, ork::lev2::RenderContextInstData& rcid, const ParticlePoolRenderBuffer& buffer, ork::lev2::GfxTarget* targ)
{	
	gtarg = targ;
	
	const ork::lev2::RenderContextFrameData* __restrict framedata = targ->GetRenderContextFrameData();
	const ork::CCameraData* __restrict cdata = framedata->GetCameraData();
	MaterialBase* pMTLBASE = 0;
	//////////////////////////////////////////
	mpVB = & GfxEnv::GetSharedDynamicVB();
	float Scale = 1.0f;
	ork::CMatrix4 MatScale;
	MatScale.SetScale( Scale,Scale,Scale );
	const CMatrix4& MVP = targ->MTXI()->RefMVPMatrix();
	///////////////////////////////////////////////////////////////
	mCurFGI = mPlugInpGradientIntensity.GetValue();
	///////////////////////////////////////////////////////////////
	// compute particle dynamic vertex buffer
	//////////////////////////////////////////
	int icnt = buffer.miNumParticles;
	int ivertexlockcount = icnt*6;
	//int ivertexlockbase = mpVB->GetNumVertices();
	if( icnt )
	{	lev2::VtxWriter<SVtxV12C4T16> vw;
		vw.Lock( targ, mpVB, ivertexlockcount );
		{	//ork::CColor4 CL;
			switch( meAlignment )
			{	case EPIA_BILLBOARD:
				{	ork::CMatrix4 matrs( mtx );
					matrs.SetTranslation( 0.0f, 0.0f, 0.0f );
					matrs.Transpose();
					ork::CVector3 nx = cdata->GetXNormal().Transform( matrs );
					ork::CVector3 ny = cdata->GetYNormal().Transform( matrs );
					ork::CVector3 NX = nx*-1.0f;
					ork::CVector3 PX = nx;
					ork::CVector3 NY = ny*-1.0f;
					ork::CVector3 PY = ny;
					NX_NY = (NX+NY);
					PX_NY = (PX+NY);
					PX_PY = (PX+PY);
					NX_PY = (NX+PY);
					break;
				}
				case EPIA_XZ:
				{	NX_NY = ork::CVector3(-1.0f,0.0f,-1.0f);
					PX_NY = ork::CVector3(+1.0f,0.0f,-1.0f);
					PX_PY = ork::CVector3(+1.0f,0.0f,+1.0f);
					NX_PY = ork::CVector3(-1.0f,0.0f,+1.0f);
					break;
				}
				case EPIA_XY:
				{	NX_NY = ork::CVector3(-1.0f,-1.0f,0.0f);
					PX_NY = ork::CVector3(+1.0f,-1.0f,0.0f);
					PX_PY = ork::CVector3(+1.0f,+1.0f,0.0f);
					NX_PY = ork::CVector3(-1.0f,+1.0f,0.0f);
					break;
				}
				case EPIA_YZ:
				{	NX_NY = ork::CVector3(0.0f,-1.0f,-1.0f);
					PX_NY = ork::CVector3(0.0f,+1.0f,-1.0f);
					PX_PY = ork::CVector3(0.0f,+1.0f,+1.0f);
					NX_PY = ork::CVector3(0.0f,-1.0f,+1.0f);
					break;
				}
			}

			miTexCnt = 1; //(miAnimTexDim*miAnimTexDim);
			mfTexs = 1.0f/1.0f; //float(miAnimTexDim);
			miTexCnt = (miAnimTexDim*miAnimTexDim);
			mfTexs = 1.0f/float(miAnimTexDim);
			uvr0 = ork::CVector2( 0.0f, 0.0f );
			uvr1 = ork::CVector2( mfTexs, 0.0f );
			uvr2 = ork::CVector2( mfTexs, mfTexs );
			uvr3 = ork::CVector2( 0.0f, mfTexs );

			////////////////////////////////////////////////////////////////////

			ork::Gradient<ork::CVector4>* pGRAD = 0;
			if( mpTemplateModule )
			{
				SpriteRenderer* ptemplate_module = 0;
				ptemplate_module = rtti::autocast(mpTemplateModule);
				const PoolString& active_g = ptemplate_module->mActiveGradient;
				const PoolString& active_m = ptemplate_module->mActiveMaterial;

				orklut<PoolString,ork::Object*>::const_iterator itG = mGradients.find(active_g);
				if( itG != mGradients.end() )
				{
					pGRAD = rtti::autocast( itG->second );
				}
				orklut<PoolString,ork::Object*>::const_iterator itM = mMaterials.find(active_m);
				if( itM != mMaterials.end() )
				{
					pMTLBASE = rtti::autocast( itM->second );
				}
			}
			////////////////////////////////////////////////////////////////////

			bool bsort = mbSort;

			if( meBlendMode>=ork::lev2::EBLENDING_ADDITIVE && meBlendMode<=EBLENDING_ALPHA_SUBTRACTIVE ) bsort=false;

			if( bsort )
			{	static ork::fixedlut<float,const ork::lev2::particle::BasicParticle*,20000> SortedParticles(EKEYPOLICY_MULTILUT);
				SortedParticles.clear();
				for( int i=0; i<icnt; i++ )
				{	const ork::lev2::particle::BasicParticle* ptcl = buffer.mpParticles+i;
					{	CVector4 proj = ptcl->mPosition.Transform(MVP);
						proj.PerspectiveDivide();
						float fv = proj.GetZ();
						SortedParticles.AddSorted( fv, ptcl );
					}
				}
				for( int i=(icnt-1); i>=0; i-- )
				{	const ork::lev2::particle::BasicParticle* __restrict ptcl = SortedParticles.GetItemAtIndex(i).second;
					//////////////////////////////////////////////////////
					float fage = ptcl->mfAge;
					float funitage = (fage/ptcl->mfLifeSpan);
					mOutDataUnitAge = funitage;
					mOutDataUnitAge = (mOutDataUnitAge<0.0f) ? 0.0f : mOutDataUnitAge;
					mOutDataUnitAge = (mOutDataUnitAge>1.0f) ? 1.0f : mOutDataUnitAge;
					float fiunitage = (1.0f-mOutDataUnitAge);
					float fsize = mPlugInpSize.GetValue();
					//////////////////////////////////////////////////////
					CVector4 color;
					 if( pGRAD )
						color = (pGRAD->Sample(mOutDataUnitAge)*mCurFGI).Saturate();
					U32 ucolor = color.GetVtxColorAsU32();
					//////////////////////////////////////////////////////
					float fang = mPlugInpRot.GetValue()*DTOR;
					float sinfr = ork::CFloat::Sin(fang)*fsize;
					float cosfr = ork::CFloat::Cos(fang)*fsize;
					CVector3 rota = (NX_NY*cosfr)+(NX_PY*sinfr);
					CVector3 rotb = (NX_PY*cosfr)-(NX_NY*sinfr);
					CVector3 p0 = ptcl->mPosition+rota;
					CVector3 p1 = ptcl->mPosition+rotb;
					CVector3 p2 = ptcl->mPosition-rota;
					CVector3 p3 = ptcl->mPosition-rotb;
					//////////////////////////////////////////////////////
					float flastframe = float(miTexCnt-1);
					float ftexframe = mPlugInpAnimFrame.GetValue()*flastframe;
					ftexframe = ( ftexframe<0.0f ) ? 0.0f : (ftexframe>=flastframe) ? flastframe : ftexframe;
					ork::CVector2 uvA( funitage, 1.0f ); //float(miAnimTexDim) );
					//////////////////////////////////////////////////////
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p0,uvr0,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p1,uvr1,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p2,uvr2,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p0,uvr0,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p2,uvr2,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p3,uvr3,uvA, ucolor) );	
				}
			}
			////////////////////////////////////////////////////////////////////
			else
			////////////////////////////////////////////////////////////////////
			{	
				const ork::lev2::particle::BasicParticle* __restrict pbase = buffer.mpParticles;

				for( int i=0; i<icnt; i++ )
				{	
					const ork::lev2::particle::BasicParticle* __restrict ptcl = pbase + i;
					//DrawParticle( buffer.mpParticles + i );
					float fage = ptcl->mfAge;
					float flspan = (ptcl->mfLifeSpan!=0.0f)?ptcl->mfLifeSpan:0.01f;
					mOutDataUnitAge = (fage/flspan);
					mOutDataUnitAge = (mOutDataUnitAge<0.0f) ? 0.0f : mOutDataUnitAge;
					mOutDataUnitAge = (mOutDataUnitAge>1.0f) ? 1.0f : mOutDataUnitAge;
					mOutDataPtcRandom = ptcl->mfRandom;
					float fiunitage = (1.0f-mOutDataUnitAge);
					float fsize = mPlugInpSize.GetValue();
					CVector4 color;
					 if( pGRAD )
						color = (pGRAD->Sample(mOutDataUnitAge)*mCurFGI).Saturate();
					U32 ucolor = color.GetVtxColorAsU32();
					float fang = mPlugInpRot.GetValue()*DTOR;
					//////////////////////////////////////////////////////
					float sinfr = ork::CFloat::Sin(fang)*fsize;
					float cosfr = ork::CFloat::Cos(fang)*fsize;
					CVector3 rota = (NX_NY*cosfr)+(NX_PY*sinfr);
					CVector3 rotb = (NX_PY*cosfr)-(NX_NY*sinfr);
					CVector3 p0 = ptcl->mPosition+rota;
					CVector3 p1 = ptcl->mPosition+rotb;
					CVector3 p2 = ptcl->mPosition-rota;
					CVector3 p3 = ptcl->mPosition-rotb;
					//////////////////////////////////////////////////////
					float flastframe = float(miTexCnt-1);
					float ftexframe = mPlugInpAnimFrame.GetValue()*flastframe;
					ftexframe = ( ftexframe<0.0f ) ? 0.0f : (ftexframe>=flastframe) ? flastframe : ftexframe;
					ork::CVector2 uvA( ftexframe, 1.0f ); //float(miAnimTexDim) );
					if( miAnimTexDim>1 )
					{
						int iframe = int(ftexframe)%miTexCnt;
						int ifrX = iframe%miAnimTexDim;
						int ifrY = iframe/miAnimTexDim;
						//miTexCnt = (miAnimTexDim*miAnimTexDim);
						//mfTexs = 1.0f/float(miAnimTexDim);
						float fu0 = float(ifrX)*mfTexs;
						float fu1 = float(ifrX+1)*mfTexs;
						float fv0 = float(ifrY)*mfTexs;
						float fv1 = float(ifrY+1)*mfTexs;
						uvr0 = ork::CVector2( fu0, fv0 );
						uvr1 = ork::CVector2( fu1, fv0 );
						uvr2 = ork::CVector2( fu1, fv1 );
						uvr3 = ork::CVector2( fu0, fv1 );						
						uvA = ork::CVector2( 1.0f, 0.0f ); //mOutDataUnitAge,mOutDataPtcRandom );
					}
					else
					{
						uvA = ork::CVector2( mOutDataUnitAge,mOutDataPtcRandom );
					}
					//////////////////////////////////////////////////////
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p0,uvr0,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p1,uvr1,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p2,uvr2,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p0,uvr0,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p2,uvr2,uvA, ucolor) );
					vw.AddVertex( ork::lev2::SVtxV12C4T16(p3,uvr3,uvA, ucolor) );
				}
			}
			////////////////////////////////////////////////////////////////////
		}
		vw.UnLock(targ);		

		//////////////////////////////////////////
		// setup particle material
		//////////////////////////////////////////
						
		if( pMTLBASE )
		{	
			lev2::GfxMaterial* bound_mtl = pMTLBASE->Bind( targ );
		
			if( bound_mtl )
			{
				bound_mtl->mRasterState.SetBlending( meBlendMode );
				targ->MTXI()->PushMMatrix(MatScale*mtx);
				targ->GBI()->DrawPrimitive( vw, ork::lev2::EPRIM_TRIANGLES, ivertexlockcount ); 
				mpVB = 0;
				targ->MTXI()->PopMMatrix();
				targ->BindMaterial( 0 );
			
			}
		}		
		
	} // if( icnt )
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void StreakRenderer::Describe()
{	RegisterFloatXfPlug( StreakRenderer, Length, -10.0f, 10.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( StreakRenderer, Width, -10.0f, 10.0f, ged::OutPlugChoiceDelegate );
	RegisterFloatXfPlug( StreakRenderer, GradientIntensity, 0.0f, 10.0f, ged::OutPlugChoiceDelegate );
	ork::reflect::RegisterProperty( "Gradient", & StreakRenderer::GradientAccessor );
	ork::reflect::RegisterProperty("BlendMode", &StreakRenderer::meBlendMode);
	ork::reflect::RegisterProperty( "Texture", & StreakRenderer::GetTextureAccessor, & StreakRenderer::SetTextureAccessor );
	ork::reflect::AnnotatePropertyForEditor<StreakRenderer>("Gradient", "editor.class", "ged.factory.gradient" );
	ork::reflect::AnnotatePropertyForEditor<StreakRenderer>("BlendMode", "editor.class", "ged.factory.enum" );
	ork::reflect::AnnotatePropertyForEditor<StreakRenderer>("Texture", "editor.class", "ged.factory.assetlist" );
	ork::reflect::AnnotatePropertyForEditor<StreakRenderer>("Texture", "editor.assettype", "lev2tex" );
	ork::reflect::AnnotatePropertyForEditor<StreakRenderer>("Texture", "editor.assetclass", "lev2tex");
	static const char* EdGrpStr =
		        "grp://StreakRenderer Input Length Width BlendMode Gradient GradientIntensity Texture ";
	reflect::AnnotateClassForEditor<StreakRenderer>( "editor.prop.groups", EdGrpStr );
}
///////////////////////////////////////////////////////////////////////////////
StreakRenderer::StreakRenderer()
	: meBlendMode(ork::lev2::EBLENDING_ALPHA)
	, ConstructOutPlug(UnitAge, dataflow::EPR_UNIFORM)
	, ConstructInpPlug( GradientIntensity, dataflow::EPR_UNIFORM, mfGradientIntensity )
	, ConstructInpPlug( Length, dataflow::EPR_UNIFORM, mfLength )
	, ConstructInpPlug( Width, dataflow::EPR_UNIFORM, mfWidth )
	, mOutDataUnitAge(0.0f)
	, mTexture( 0 )
	, mfLength(1.0f)
	, mfWidth(1.0f)
	, mfGradientIntensity(1.0f)
{
	ork::lev2::GfxTarget* targ = ork::lev2::GfxEnv::GetRef().GetLoaderTarget();
	mpMaterial = new GfxMaterial3DSolid(targ, "orkshader://particle", "tbasicparticle");
}
///////////////////////////////////////////////////////////////////////////////
void StreakRenderer::SetTextureAccessor( ork::rtti::ICastable* const & tex)
{	mTexture = tex ? ork::rtti::autocast( tex ) : 0;
}
///////////////////////////////////////////////////////////////////////////////
void StreakRenderer::GetTextureAccessor( ork::rtti::ICastable* & tex) const
{	tex = mTexture;
}
///////////////////////////////////////////////////////////////////////////////
ork::lev2::Texture*	StreakRenderer::GetTexture() const
{	return (mTexture==0) ? 0 : mTexture->GetTexture();
}
///////////////////////////////////////////////////////////////////////////////
dataflow::inplugbase* StreakRenderer::GetInput(int idx)
{	dataflow::inplugbase* rval = 0;
	switch(idx)
	{	case 0:	rval = & mPlugInpInput;		break;
		case 1:	rval = & mPlugInpLength;	break;
		case 2:	rval = & mPlugInpWidth;		break;
		case 3: rval = & mPlugInpGradientIntensity; break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////
dataflow::outplugbase* StreakRenderer::GetOutput(int idx)
{	dataflow::outplugbase* rval = 0;
	switch(idx)
	{	case 0:	rval = & OutPlugName(UnitAge);		break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////
void StreakRenderer::Render(const CMatrix4& mtx, ork::lev2::RenderContextInstData& rcid, const ParticlePoolRenderBuffer& buffer, ork::lev2::GfxTarget* targ)
{	const ork::lev2::RenderContextFrameData* framedata = targ->GetRenderContextFrameData();
	const ork::CCameraData* cdata = framedata->GetCameraData();
	//////////////////////////////////////////
	ork::lev2::CVtxBuffer<ork::lev2::SVtxV12C4T16>& vtxbuf = lev2::GfxEnv::GetSharedDynamicVB(); 
	float Scale = 1.0f;
	ork::CMatrix4 MatScale;
	MatScale.SetScale( Scale,Scale,Scale );
	///////////////////////////////////////////////////////////////
	float fgi = mPlugInpGradientIntensity.GetValue();
	///////////////////////////////////////////////////////////////
	// compute particle dynamic vertex buffer
	//////////////////////////////////////////
	int icnt = buffer.miNumParticles;
	int ivertexlockcount = icnt*6;
	//int ivertexlockbase = vtxbuf.GetNumVertices();
	if( icnt )
	{	////////////////////////////////////////////////////////////////////////////
		ork::CMatrix4 mtx_iw = mtx;
		mtx_iw.Inverse();
		CVector3 obj_nrmz = CVector4(cdata->GetZNormal(),0.0f).Transform(mtx_iw).Normal();
		////////////////////////////////////////////////////////////////////////////
		lev2::VtxWriter<SVtxV12C4T16> vw;
		vw.Lock( targ, &vtxbuf, ivertexlockcount );
		{	////////////////////////////////////////////////
			// uniform properties
			////////////////////////////////////////////////	
			ork::CColor4 CL;
			ork::CVector2 uvr0( 0.0f, 0.0f );
			ork::CVector2 uvr1( 1.0f, 0.0f );
			ork::CVector2 uvr2( 1.0f, 1.0f );
			ork::CVector2 uvr3( 0.0f, 1.0f );
			const ork::lev2::particle::BasicParticle* __restrict ptclbase = buffer.mpParticles;
			for( int i=0; i<icnt; i++ )
			{	const ork::lev2::particle::BasicParticle* __restrict ptcl = ptclbase+i;
				////////////////////////////////////////////////
				// varying properties
				////////////////////////////////////////////////
				float fage = ptcl->mfAge;
				mOutDataUnitAge = (fage/ptcl->mfLifeSpan);
				mOutDataUnitAge = (mOutDataUnitAge<0.0f) ? 0.0f : mOutDataUnitAge;
				mOutDataUnitAge = (mOutDataUnitAge>1.0f) ? 1.0f : mOutDataUnitAge;
				float fiunitage = (1.0f-mOutDataUnitAge);
				float fwidth = mPlugInpWidth.GetValue();
				float flength = mPlugInpLength.GetValue();
				////////////////////////////////////////////////
				// compute streak positions
				////////////////////////////////////////////////
				CVector3 lpos = ptcl->mPosition - ptcl->mVelocity*flength;
				CVector3 pvel = ptcl->mVelocity.Cross(obj_nrmz).Normal()*fwidth;
				ork::CVector3 p0 = ptcl->mPosition + pvel;
				ork::CVector3 p1 = ptcl->mPosition - pvel;
				ork::CVector3 p2 = lpos - pvel;
				ork::CVector3 p3 = lpos + pvel;
				////////////////////////////////////////////////
				CVector4 color = mGradient.Sample(mOutDataUnitAge)*fgi;
				U32 ucolor = color.GetVtxColorAsU32();

				vw.AddVertex( ork::lev2::SVtxV12C4T16( p0, uvr0, ucolor ) );
				vw.AddVertex( ork::lev2::SVtxV12C4T16( p1, uvr1, ucolor ) );
				vw.AddVertex( ork::lev2::SVtxV12C4T16( p2, uvr2, ucolor ) );
				vw.AddVertex( ork::lev2::SVtxV12C4T16( p0, uvr0, ucolor ) );
				vw.AddVertex( ork::lev2::SVtxV12C4T16( p2, uvr2, ucolor ) );
				vw.AddVertex( ork::lev2::SVtxV12C4T16( p3, uvr3, ucolor ) );
				////////////////////////////////////////////////
			}
		}	
		vw.UnLock(targ);			
		////////////////////////////////////////////////////////////////////////////
		// setup particle material
		//////////////////////////////////////////
		targ->BindMaterial( mpMaterial );
		mpMaterial->SetColorMode( ork::lev2::GfxMaterial3DSolid::EMODE_USER );
		mpMaterial->mRasterState.SetAlphaTest( ork::lev2::EALPHATEST_GREATER, 0.0f );
		mpMaterial->mRasterState.SetDepthTest( ork::lev2::EDEPTHTEST_LEQUALS );
		mpMaterial->mRasterState.SetBlending( meBlendMode );
		mpMaterial->mRasterState.SetZWriteMask( false );
		mpMaterial->mRasterState.SetCullTest( ork::lev2::ECULLTEST_OFF );
		mpMaterial->mRasterState.SetPointSize( 32.0f );
		mpMaterial->SetTexture( GetTexture() );
		//////////////////////////////////////////
		// Draw Particles 
		//////////////////////////////////////////
		targ->MTXI()->PushMMatrix(MatScale*mtx);
		targ->GBI()->DrawPrimitive( vw, ork::lev2::EPRIM_TRIANGLES, ivertexlockcount ); 
		targ->MTXI()->PopMMatrix();
		//////////////////////////////////////////
		targ->BindMaterial( 0 );
	} // if( icnt )
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ModelRenderer::Describe()
{	RegisterFloatXfPlug( ModelRenderer, Scale, -100.0f, 100.0f, ged::OutPlugChoiceDelegate );
	ork::reflect::RegisterProperty( "Model", & ModelRenderer::GetModelAccessor, & ModelRenderer::SetModelAccessor );
	ork::reflect::AnnotatePropertyForEditor<ModelRenderer>("Model", "editor.class", "ged.factory.assetlist" );
	ork::reflect::AnnotatePropertyForEditor<ModelRenderer>("Model", "editor.assettype", "xgmodel" );
	ork::reflect::AnnotatePropertyForEditor<ModelRenderer>("Model", "editor.assetclass", "xgmodel");
	static const char* EdGrpStr =
		        "sort://Input Model Scale";
	reflect::AnnotateClassForEditor<ModelRenderer>( "editor.prop.groups", EdGrpStr );
}
///////////////////////////////////////////////////////////////////////////////
ModelRenderer::ModelRenderer()
	: ConstructOutPlug( UnitAge, dataflow::EPR_UNIFORM)
	, ConstructInpPlug( Scale, dataflow::EPR_UNIFORM, mfScale )
	, mOutDataUnitAge(0.0f)
	, mModel( 0 )
	, mfScale(1.0f)
{
}
///////////////////////////////////////////////////////////////////////////////
void ModelRenderer::SetModelAccessor( ork::rtti::ICastable* const & tex)
{	mModel = tex ? ork::rtti::autocast( tex ) : 0;
}
///////////////////////////////////////////////////////////////////////////////
void ModelRenderer::GetModelAccessor( ork::rtti::ICastable* & mdl) const
{	mdl = mModel;
}
///////////////////////////////////////////////////////////////////////////////
ork::lev2::XgmModel* ModelRenderer::GetModel() const
{	return (mModel==0) ? 0 : mModel->GetModel();
}
///////////////////////////////////////////////////////////////////////////////
dataflow::inplugbase* ModelRenderer::GetInput(int idx)
{	dataflow::inplugbase* rval = 0;
	switch(idx)
	{	case 0:	rval = & mPlugInpInput;		break;
		case 1:	rval = & mPlugInpScale;		break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////
dataflow::outplugbase* ModelRenderer::GetOutput(int idx)
{	dataflow::outplugbase* rval = 0;
	switch(idx)
	{	case 0:	rval = & OutPlugName(UnitAge);		break;
	}
	return rval;
}
///////////////////////////////////////////////////////////////////////////////
void ModelRenderer::Render(const CMatrix4& mtx, ork::lev2::RenderContextInstData& rcid, const ParticlePoolRenderBuffer& buffer, ork::lev2::GfxTarget* targ)
{	if( 0 == GetModel() ) return;
	const ork::lev2::RenderContextFrameData* framedata = targ->GetRenderContextFrameData();
	const ork::CCameraData* cdata = framedata->GetCameraData();
	int icnt = buffer.miNumParticles;
	static const int kmaxinstances = 1024;
	static CMatrix4 gmatrixblock[ kmaxinstances ]; 
	OrkAssert( icnt<kmaxinstances );
	if( icnt>=kmaxinstances ) icnt=kmaxinstances-1;
	if( icnt )
	{	////////////////////////////////////////////////
		// uniform properties
		////////////////////////////////////////////////	
		//printf( "psys::ModelRenderer::Render() icnt<%d>\n", icnt );
		CMatrix4 nmtx;
		for( int i=0; i<icnt; i++ )
		{	const ork::lev2::particle::BasicParticle* ptcl = buffer.mpParticles+i;
			////////////////////////////////////////////////
			// varying properties
			////////////////////////////////////////////////
			float fage = ptcl->mfAge;
			mOutDataUnitAge = (fage/ptcl->mfLifeSpan);
			mOutDataUnitAge = (mOutDataUnitAge<0.0f) ? 0.0f : mOutDataUnitAge;
			mOutDataUnitAge = (mOutDataUnitAge>1.0f) ? 1.0f : mOutDataUnitAge;
			float fscale = mPlugInpScale.GetValue();
			nmtx.SetScale( fscale, fscale, fscale );
			nmtx.SetTranslation( ptcl->mPosition );
			gmatrixblock[i] = (nmtx*mtx);
		}
		ork::lev2::XgmModelInst minst( GetModel() );
		ork::lev2::RenderContextInstData MatCtx;
		ork::lev2::RenderContextInstModelData MdlCtx;
		ork::lev2::XgmMaterialStateInst MatInst(minst);
		MatCtx.SetMaterialInst(&MatInst);

		MdlCtx.SetSkinned(false);

		///////////////////////////////////////////////////////////
		// setup headlight (default lighting)
		///////////////////////////////////////////////////////////
		ork::CMatrix4				HeadLightMatrix;
		ork::lev2::LightingGroup	HeadLightGroup;
		ork::lev2::AmbientLightData	HeadLightData;
		ork::lev2::AmbientLight		HeadLight(HeadLightMatrix,&HeadLightData);
		ork::lev2::LightManagerData	HeadLightManagerData;
		ork::lev2::LightManager HeadLightManager(HeadLightManagerData);
		HeadLightData.SetColor(ork::CVector3(1.3f, 1.3f, 1.5f));
		HeadLightData.SetAmbientShade( 0.75f );
		HeadLight.miInFrustumID = 1;
		HeadLightGroup.mLightMask.AddLight( & HeadLight );
		HeadLightGroup.mLightManager = & HeadLightManager;
		const RenderContextFrameData& FrameData = *targ->GetRenderContextFrameData();
		HeadLightMatrix = FrameData.GetCameraData()->GetIVMatrix();
		HeadLightManager.mGlobalMovingLights.AddLight( & HeadLight );
		HeadLightManager.mLightsInFrustum.push_back(& HeadLight);
		MatCtx.SetLightingGroup( & HeadLightGroup );
		//FrameData.SetLightManager( & HeadLightManager );
		//targ->SetRenderContextInstData( & rcid );
		///////////////////////////////////////////////////////////
		// setup headlight (default lighting)
		///////////////////////////////////////////////////////////
		int inummeshes = GetModel()->GetNumMeshes();
		for( int imesh=0; imesh<inummeshes; imesh++ )
		{
			const lev2::XgmMesh& mesh = * GetModel()->GetMesh(imesh);

			int inumclusset = mesh.GetNumSubMeshes();

			for( int ics=0; ics<inumclusset; ics++ )
			{
				const lev2::XgmSubMesh& submesh = * mesh.GetSubMesh(ics);
				const lev2::GfxMaterial* material = submesh.mpMaterial;

				int inumclus = submesh.miNumClusters;

				MatCtx.SetMaterialIndex(ics);

				for( int ic=0; ic<inumclus; ic++ )
				{

					MdlCtx.mMesh = & mesh;
					MdlCtx.mSubMesh = & submesh;
					MdlCtx.mCluster = & submesh.RefCluster(ic);

					GetModel()->RenderMultipleRigid(	ork::CColor4::White(), 
														gmatrixblock, icnt,
														targ,
														MatCtx,
														MdlCtx );
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MaterialBase::Describe()
{
}
///////////////////////////////////////////////////////////////////////////////
void TextureMaterial::Describe()
{
	ork::reflect::RegisterProperty( "Texture", & TextureMaterial::GetTextureAccessor, & TextureMaterial::SetTextureAccessor );
	ork::reflect::AnnotatePropertyForEditor<TextureMaterial>("Texture", "editor.class", "ged.factory.assetlist" );
	ork::reflect::AnnotatePropertyForEditor<TextureMaterial>("Texture", "editor.assettype", "lev2tex" );
	ork::reflect::AnnotatePropertyForEditor<TextureMaterial>("Texture", "editor.assetclass", "lev2tex");
}
///////////////////////////////////////////////////////////////////////////////
TextureMaterial::TextureMaterial()
	: mTexture(0)
{
	ork::lev2::GfxTarget* targ = ork::lev2::GfxEnv::GetRef().GetLoaderTarget();
	mpMaterial = new GfxMaterial3DSolid(targ, "orkshader://particle", "tbasicparticle");
	mpMaterial->SetColorMode( GfxMaterial3DSolid::EMODE_USER );
	//mpMaterialVOLUME = new GfxMaterial3DSolid(targ, "orkshader://particle", "tvolumeparticle");
	//mpMaterialVOLUME->SetColorMode( GfxMaterial3DSolid::EMODE_USER );
}
///////////////////////////////////////////////////////////////////////////////
void TextureMaterial::SetTextureAccessor( ork::rtti::ICastable* const & tex)
{	mTexture = tex ? ork::rtti::autocast( tex ) : 0;
}
///////////////////////////////////////////////////////////////////////////////
void TextureMaterial::GetTextureAccessor( ork::rtti::ICastable* & tex) const
{	tex = mTexture;
}
///////////////////////////////////////////////////////////////////////////////
ork::lev2::Texture*	TextureMaterial::GetTexture() const
{	return (mTexture==0) ? 0 : mTexture->GetTexture();
}
void TextureMaterial::Update( float ftexframe )
{
	if( gtarg && GetTexture() )
	{
		lev2::TextureAnimationBase* texanim = GetTexture()->GetTexAnim();
		
		if( texanim )
		{
			TextureAnimationInst tai( texanim );
			tai.SetCurrentTime( ftexframe );
			gtarg->TXI()->UpdateAnimatedTexture( GetTexture(), &tai );
		}
	}
}
lev2::GfxMaterial* TextureMaterial::Bind( lev2::GfxTarget* pT )
{

	mpMaterial->SetTexture( GetTexture() );
	mpMaterial->SetColorMode( ork::lev2::GfxMaterial3DSolid::EMODE_USER );
	mpMaterial->mRasterState.SetAlphaTest( ork::lev2::EALPHATEST_GREATER, 0.0f );
	mpMaterial->mRasterState.SetDepthTest( ork::lev2::EDEPTHTEST_LEQUALS );
	mpMaterial->mRasterState.SetZWriteMask( false );
	mpMaterial->mRasterState.SetCullTest( ork::lev2::ECULLTEST_OFF );
	mpMaterial->mRasterState.SetPointSize( 32.0f );
	pT->BindMaterial(mpMaterial);

	return mpMaterial;
}

///////////////////////////////////////////////////////////////////////////////
void VolTexMaterial::Describe()
{
	ork::reflect::RegisterProperty( "Texture", & VolTexMaterial::GetTextureAccessor, & VolTexMaterial::SetTextureAccessor );
	ork::reflect::AnnotatePropertyForEditor<VolTexMaterial>("Texture", "editor.class", "ged.factory.assetlist" );
	ork::reflect::AnnotatePropertyForEditor<VolTexMaterial>("Texture", "editor.assettype", "lev2tex" );
	ork::reflect::AnnotatePropertyForEditor<VolTexMaterial>("Texture", "editor.assetclass", "lev2tex");
}

///////////////////////////////////////////////////////////////////////////////
VolTexMaterial::VolTexMaterial()
	: mTexture(0)
{
	ork::lev2::GfxTarget* targ = ork::lev2::GfxEnv::GetRef().GetLoaderTarget();
	mpMaterial = new GfxMaterial3DSolid(targ, "orkshader://particle", "tvolumeparticle");
	mpMaterial->SetColorMode( GfxMaterial3DSolid::EMODE_USER );
}
///////////////////////////////////////////////////////////////////////////////
void VolTexMaterial::SetTextureAccessor( ork::rtti::ICastable* const & tex)
{	mTexture = tex ? ork::rtti::autocast( tex ) : 0;
}
///////////////////////////////////////////////////////////////////////////////
void VolTexMaterial::GetTextureAccessor( ork::rtti::ICastable* & tex) const
{	tex = mTexture;
}
///////////////////////////////////////////////////////////////////////////////
ork::lev2::Texture*	VolTexMaterial::GetTexture() const
{	return (mTexture==0) ? 0 : mTexture->GetTexture();
}
void VolTexMaterial::Update( float ftexframe )
{
	if( gtarg && GetTexture() )
	{
		lev2::TextureAnimationBase* texanim = GetTexture()->GetTexAnim();
		
		if( texanim )
		{
			TextureAnimationInst tai( texanim );
			tai.SetCurrentTime( ftexframe );
			gtarg->TXI()->UpdateAnimatedTexture( GetTexture(), &tai );
		}
	}
}
lev2::GfxMaterial* VolTexMaterial::Bind( lev2::GfxTarget* pT )
{

	mpMaterial->SetVolumeTexture( GetTexture() );
	mpMaterial->SetColorMode( ork::lev2::GfxMaterial3DSolid::EMODE_USER );
	mpMaterial->mRasterState.SetAlphaTest( ork::lev2::EALPHATEST_GREATER, 0.0f );
	mpMaterial->mRasterState.SetDepthTest( ork::lev2::EDEPTHTEST_LEQUALS );
	mpMaterial->mRasterState.SetZWriteMask( false );
	mpMaterial->mRasterState.SetCullTest( ork::lev2::ECULLTEST_OFF );
	mpMaterial->mRasterState.SetPointSize( 32.0f );
	pT->BindMaterial(mpMaterial);

	return mpMaterial;
}


	/*
		if( mVolumeTexture && mVolumeTexture->GetTexture() && mVolumeTexture->GetTexture()->IsVolumeTexture() )
		{	//printf( "VOLTEK\n" );
			pmat = mpMaterialVOLUME;
			pmat->SetVolumeTexture( mVolumeTexture->GetTexture() );
		}
		//////////////////////////////////////////		
		//////////////////////////////////////////
		CVector3 NoiseAmp, NoiseFreq, NoiseShift;

		NoiseAmp.SetX( mPlugInpNoiseAmp0.GetValue() );
		NoiseAmp.SetY( mPlugInpNoiseAmp1.GetValue() );
		NoiseAmp.SetZ( mPlugInpNoiseAmp2.GetValue() );

		NoiseFreq.SetX( mPlugInpNoiseFreq0.GetValue() );
		NoiseFreq.SetY( mPlugInpNoiseFreq1.GetValue() );
		NoiseFreq.SetZ( mPlugInpNoiseFreq2.GetValue() );

		NoiseShift.SetX( mPlugInpNoiseShift0.GetValue() );
		NoiseShift.SetY( mPlugInpNoiseShift1.GetValue() );
		NoiseShift.SetZ( mPlugInpNoiseShift2.GetValue() );
		pmat->SetNoiseAmp( NoiseAmp );
		pmat->SetNoiseFreq( NoiseFreq );
		pmat->SetNoiseShift( NoiseShift );
		//////////////////////////////////////////


*/
		/*
		if( mbImageSequenceOK )
		{
			printf( "mbImageSequenceOK\n" );
			
			int maxframes = mSequenceTextures.size();
			if( maxframes < 1 )
			{
				pmat->SetTexture( 0 );
			}
			else
			{
				int iframe = miImageFrame%maxframes;
				printf( "SetTex frame<%d>\n", iframe );
				lev2::TextureAsset* passet = mSequenceTextures[iframe];
				lev2::Texture* ptex = (passet==0) ? 0 : passet->GetTexture();
				pmat->SetTexture( ptex );
				
			}
			
		}
		else
		{
			pmat->SetTexture( GetTexture() );
			
		}
		targ->BindMaterial( pmat );*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
}}}