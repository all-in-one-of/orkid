
////////////////////////////////////////////////////////////////
// Orkid Media Engine
// Copyright 1996-2012, Michael T. Mayers.
// Distributed under the Boost Software License - Version 1.0 - August 17, 2003
// see http://www.boost.org/LICENSE_1_0.txt
//////////////////////////////////////////////////////////////// 


#include <ork/pch.h>

#include <ork/asset/Asset.h>
#include <ork/asset/AssetSetEntry.h>

///////////////////////////////////////////////////////////////////////////////

INSTANTIATE_TRANSPARENT_RTTI(ork::asset::Asset, "Asset2");


//template ork::rtti::RTTI<ork::asset::Asset, ork::Object, ork::rtti::AbstractPolicy, ork::asset::AssetClass>;

///////////////////////////////////////////////////////////////////////////////
namespace ork { namespace asset {
///////////////////////////////////////////////////////////////////////////////


void Asset::Describe(){} 

///////////////////////////////////////////////////////////////////////////////

void Asset::SetName(PoolString name)
{
	mName = name;
}

///////////////////////////////////////////////////////////////////////////////

PoolString Asset::GetName() const
{
	return mName;
}

///////////////////////////////////////////////////////////////////////////////

PoolString Asset::GetType() const
{
	return rtti::safe_downcast<AssetClass *>(GetClass())->Name();
}

///////////////////////////////////////////////////////////////////////////////

bool Asset::Load() const
{
	AssetSetEntry *entry = GetAssetSetEntry(this);

	return entry->Load(GetClass()->GetAssetSet().GetTopLevel());
}

bool Asset::LoadUnManaged() const
{
	AssetSetEntry *entry = GetAssetSetEntry(this);

	return entry->Load(GetClass()->GetAssetSet().GetTopLevel());
}

///////////////////////////////////////////////////////////////////////////////

bool Asset::IsLoaded() const
{
	AssetSetEntry *entry = GetAssetSetEntry(this);

	return entry && entry->IsLoaded();
}

///////////////////////////////////////////////////////////////////////////////
} }
///////////////////////////////////////////////////////////////////////////////
