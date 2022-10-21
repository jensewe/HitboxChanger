#include "extension.h"
#include "modeltypes.h"

HitboxChanger g_HitboxChanger;
IVModelInfo   *g_ModelInfo = NULL;
IMDLCache     *g_ModelCache = NULL;

SMEXT_LINK(&g_HitboxChanger);

enum StudioHdrReturn 
{
    StudioHdrRet_Successful,
    StudioHdrRet_BadModel,
    StudioHdrRet_BadModelType,
    StudioHdrRet_BadStudioHdr,
};

StudioHdrReturn GetStudioHdr(studiohdr_t *&pStudioHdr, int modelIndex)
{
	const model_t* pModel = g_ModelInfo->GetModel(modelIndex);

	if (!pModel)
	{
            return StudioHdrRet_BadModel;
	}

        // This will be a hack until IVModelInfo is updated in their respective SDK
#if SOURCE_ENGINE == SE_LEFT4DEAD2 || SOURCE_ENGINE == SE_CSGO
	if (pModel->type != mod_studio)
	{
            return StudioHdrRet_BadModelType;
	}

	pStudioHdr = g_ModelCache->GetStudioHdr(pModel->studio);
#else

#if DEBUG
        // Pointer to easily check the vtable offset with an debugger
        studiohdr_t *(IVModelInfo::*fp)(const model_t *) = (studiohdr_t *(IVModelInfo::*)(const model_t *))&IVModelInfo::GetStudiomodel;
        pStudioHdr = (g_ModelInfo->*fp)(pModel);
#else
        pStudioHdr = g_ModelInfo->GetStudiomodel(pModel);
#endif
#endif

	if (!pStudioHdr)
	{
            return StudioHdrRet_BadStudioHdr;
	}

        return StudioHdrRet_Successful;
}

cell_t HitboxInfo(IPluginContext *pContext, const cell_t *params)
{
        cell_t modelIndex = params[1];

        studiohdr_t *pStudioHdr = nullptr;

        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                return pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! IVModelInfo::GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                return pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                return pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}
	
        for (auto i = 0; i < hitboxset->numhitboxes; i++)
        {
                proxy_mstudiobbox_t *hitbox = reinterpret_cast<proxy_mstudiobbox_t *>(hitboxset->pHitbox(i));
                
                if (!hitbox)
                        continue;

#if SOURCE_ENGINE == SE_CSGO
                rootconsole->ConsolePrint("HB: %d, Bone: %d, Bone Name: %s, BoneFlags: %d, Group: %d, Min: [ %.2f, %.2f, %.2f ], Max: [ %.2f, %.2f, %.2f ], Angle: [ %.2f, %.2f, %.2f ], Radius: %.2f",
                                                                        i, hitbox->bone, pStudioHdr->pBone(hitbox->bone)->pszName(), pStudioHdr->pBone(hitbox->bone)->flags, hitbox->group,
                                                                        hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z,
                                                                        hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z,
                                                                        hitbox->angOffsetOrientation.x, hitbox->angOffsetOrientation.y, hitbox->angOffsetOrientation.z,
                                                                        hitbox->flCapsuleRadius);
#else
                rootconsole->ConsolePrint("HB: %d, Bone: %d, Bone Name: %s, BoneFlags: %d, Group: %d, Min: [ %.2f, %.2f, %.2f ], Max: [ %.2f, %.2f, %.2f ]",
                                                                        i, hitbox->bone, pStudioHdr->pBone(hitbox->bone)->pszName(), pStudioHdr->pBone(hitbox->bone)->flags, hitbox->group,
                                                                        hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z,
                                                                        hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z);
#endif
        }
	return 1;
}

cell_t BoneInfo(IPluginContext *pContext, const cell_t *params)
{
        cell_t modelIndex = params[1];
        
        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}
        
        for (auto i = 0; i < pStudioHdr->numbones; i++)
        {
                const mstudiobone_t *bone = pStudioHdr->pBone(i);
                if (!bone)
                        continue;
                rootconsole->ConsolePrint("Bone: %d, Name: %s, Flags: %d",
					  i,
					  bone->pszName(),
					  bone->flags);
		const QAngle& ang = bone->rot.ToQAngle();
		rootconsole->ConsolePrint("pos: [ %.2f, %.2f, %.2f ], quat: [ %.2f, %.2f, %.2f, %.2f ], rot: [ %.2f, %.2f, %.2f ], ang: [ %.2f, %.2f, %.2f ]",
					  bone->pos.x, bone->pos.y, bone->pos.z,
					  bone->quat.x, bone->quat.y, bone->quat.z, bone->quat.w,
					  bone->rot.x, bone->rot.y, bone->rot.z,
					  ang.x, ang.y, ang.z);
        }
	return 1;
}

cell_t SetHitbox(IPluginContext *pContext, const cell_t *params)
{	
        cell_t modelIndex = params[1];
	cell_t hitboxIndex = params[2];

        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}
	
        if(hitboxIndex < 0 || hitboxset->numhitboxes <= hitboxIndex)
        {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Set Hitbox Failure!!! Hitbox Index [%d] is invalid!", hitboxIndex);
                return 0;
        }
        if(params[3] < 0 || pStudioHdr->numbones <= params[3])
        {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Set Hitbox Failure!!! Invalid Bone Index");
                return 0;
        }

        proxy_mstudiobbox_t *hitbox = reinterpret_cast<proxy_mstudiobbox_t *>(hitboxset->pHitbox(hitboxIndex));

        if(!hitbox)
        {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Set Hitbox Failure!!! Bad HitBox [%d]", hitboxIndex);
                return 0;
        }
        if((pStudioHdr->pBone(params[3])->flags & BONE_USED_BY_HITBOX) == 0)
        {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Set Hitbox Failure!!! Bone does not have hitbox flag set");
                return 0;
        }

        hitbox->bone = params[3];			
        hitbox->group = params[4];
        
        cell_t *bbmin, *bbmax;
        pContext->LocalToPhysAddr(params[5], &bbmin);
        pContext->LocalToPhysAddr(params[6], &bbmax);

        //Set Min Point
        hitbox->bbmin.x = sp_ctof(bbmin[0]);
        hitbox->bbmin.y = sp_ctof(bbmin[1]);
        hitbox->bbmin.z = sp_ctof(bbmin[2]);
        
        //Set Max Point
        hitbox->bbmax.x = sp_ctof(bbmax[0]);
        hitbox->bbmax.y = sp_ctof(bbmax[1]);
        hitbox->bbmax.z = sp_ctof(bbmax[2]);
        
#if SOURCE_ENGINE == SE_CSGO
        cell_t *angle;
        pContext->LocalToPhysAddr(params[7], &angle);

        //Set Angle
        hitbox->angOffsetOrientation.x = sp_ctof(angle[0]);
        hitbox->angOffsetOrientation.y = sp_ctof(angle[1]);
        hitbox->angOffsetOrientation.z = sp_ctof(angle[2]);
        
        //Set Radius (0 for box)
        hitbox->flCapsuleRadius = sp_ctof(params[8]);
#endif

	return 1;
}

cell_t GetHitbox(IPluginContext *pContext, const cell_t *params)
{	
        cell_t modelIndex = params[1];
	cell_t hitboxIndex = params[2];

        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}
	
        if(hitboxIndex < 0 || hitboxset->numhitboxes <= hitboxIndex)
        {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Get Hitbox Failure!!! Hitbox Index [%d] is invalid!", hitboxIndex);
                return 0;
        }

        proxy_mstudiobbox_t *hitbox = reinterpret_cast<proxy_mstudiobbox_t *>(hitboxset->pHitbox(hitboxIndex));
        
        cell_t *bone;
        pContext->LocalToPhysAddr(params[3], &bone);
        *bone = hitbox->bone;

        cell_t *group;
        pContext->LocalToPhysAddr(params[4], &group);
        *group = hitbox->group;
        
        cell_t *bbmin, *bbmax;
        pContext->LocalToPhysAddr(params[5], &bbmin);
        pContext->LocalToPhysAddr(params[6], &bbmax);
        //Get Min Point
        bbmin[0] = sp_ftoc(hitbox->bbmin.x);
        bbmin[1] = sp_ftoc(hitbox->bbmin.y);
        bbmin[2] = sp_ftoc(hitbox->bbmin.z);
        
        //Get Max Point
        
        bbmax[0] = sp_ftoc(hitbox->bbmax.x);
        bbmax[1] = sp_ftoc(hitbox->bbmax.y);
        bbmax[2] = sp_ftoc(hitbox->bbmax.z);
        
#if SOURCE_ENGINE == SE_CSGO
        cell_t *angle;
        pContext->LocalToPhysAddr(params[7], &angle);

        //Get Angle
        angle[0] = sp_ftoc(hitbox->angOffsetOrientation.x);
        angle[1] = sp_ftoc(hitbox->angOffsetOrientation.y);
        angle[2] = sp_ftoc(hitbox->angOffsetOrientation.z);
        
        //Set Radius (0 for box)
        cell_t *radius;
        pContext->LocalToPhysAddr(params[8], &radius);
        *radius = sp_ftoc(hitbox->flCapsuleRadius);
#endif

	return 1;
}


cell_t SetNumHitboxes(IPluginContext *pContext, const cell_t *params)
{	
        cell_t modelIndex = params[1];

        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}
	
        hitboxset->numhitboxes = params[2];

	return 1;
}

cell_t GetNumHitboxes(IPluginContext *pContext, const cell_t *params)
{	
        cell_t modelIndex = params[1];

        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	mstudiohitboxset_t *hitboxset = pStudioHdr->pHitboxSet(0);

        if (!hitboxset)
	{
		pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return 0;
	}

        return hitboxset->numhitboxes;
}

cell_t FindBone(IPluginContext *pContext, const cell_t *params)
{
	char *boneName;
	pContext->LocalToString(params[1], &boneName);

	cell_t modelIndex = params[2];
	
        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

	//bones are a bitch im stealing the code instead of using the function
	// binary search for the bone matching boneName
	int start = 0, end = pStudioHdr->numbones-1;
	const byte *pBoneTable = pStudioHdr->GetBoneTableSortedByName();
	const mstudiobone_t *pbones = pStudioHdr->pBone( 0 );
	while (start <= end)
	{
		int mid = (start + end) >> 1;
		int cmp = Q_stricmp( pbones[pBoneTable[mid]].pszName(), boneName );
		
		if ( cmp < 0 )
		{
			start = mid + 1;
		}
		else if ( cmp > 0 )
		{
			end = mid - 1;
		}
		else
		{
			return pBoneTable[mid];
		}
	}

        return 0;
}

cell_t FindValidBones(IPluginContext *pContext, const cell_t *params)
{
        cell_t modelIndex = params[1];

        studiohdr_t *pStudioHdr = nullptr;
        StudioHdrReturn ret = GetStudioHdr(pStudioHdr, modelIndex);

        switch (ret)
        {
            case StudioHdrRet_BadModel:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! g_ModelInfo->GetModel() returned null");
                break;
            }
            case StudioHdrRet_BadModelType:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Hitbox Info Failure!!! model is not of type studiomodel");
                break;
            }
            case StudioHdrRet_BadStudioHdr:
            {
                pContext->ThrowNativeError("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
                break;
            }
        }

        for (auto i = 0; i < pStudioHdr->numbones; i++)
        {
                const mstudiobone_t *bone = pStudioHdr->pBone(i);
                if (!(bone->flags & BONE_USED_BY_HITBOX))
                        continue;
                rootconsole->ConsolePrint("ValidHitboxBone: %d, Name: %s, Flags: %d", i, bone->pszName(),bone->flags);
        }

	return 1;
}

const sp_nativeinfo_t MyNatives[] = 
{
	{"HitboxInfo",		HitboxInfo},
	{"BoneInfo",		BoneInfo},
	{"SetHitbox",		SetHitbox},
	{"GetHitbox",		GetHitbox},
	{"SetNumHitboxes",	SetNumHitboxes},
	{"GetNumHitboxes",	GetNumHitboxes},
	{"FindBone",		FindBone},
	{"FindValidBones",	FindValidBones},
	{NULL,			NULL},
};

bool HitboxChanger::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, MyNatives);

	return true;
}

bool HitboxChanger::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, g_ModelInfo,    IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_ModelCache,   IMDLCache,   MDLCACHE_INTERFACE_VERSION);

	return true;
}
