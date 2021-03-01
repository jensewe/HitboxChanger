#include <model_types.h>
#include <studio.h>
#include <ivmodelinfo.h>
#include <datacache/imdlcache.h>

#define MAX_OSPATH 260

typedef void *FileNameHandle_t;

#if SOURCE_ENGINE == SE_LEFT4DEAD2 || SOURCE_ENGINE == SE_CSGO
struct model_t
{
	FileNameHandle_t		fnHandle;
	char				szPathName[MAX_OSPATH];

	int				nLoadFlags;		// mark loaded/not loaded
	int				nServerCount;	// marked at load

	modtype_t			type;
	int				flags;			// MODELFLAG_???

	// volume occupied by the model graphics	
	Vector				mins, maxs;
#if SOURCE_ENGINE == SE_CSGO
        float                           radius;
#endif
	KeyValues			*m_pKeyValues;
	union
	{
		//brushdata_t		brush;
		MDLHandle_t		studio;
		//spritedata_t	        sprite;
	};

};
#endif

struct proxy_mstudiobbox_t
{
	DECLARE_BYTESWAP_DATADESC();
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box, or the ends of the capsule if flCapsuleRadius > 0 
	Vector				bbmax;	
	int					szhitboxnameindex;	// offset to the name of the hitbox.
#if SOURCE_ENGINE == SE_CSGO
	QAngle				angOffsetOrientation;
	float				flCapsuleRadius;
	int32				unused[4];
#else
	int32				unused[8];
#endif

	const char* pszHitboxName() const
	{
		if( szhitboxnameindex == 0 )
			return "";

		return ((const char*)this) + szhitboxnameindex;
	}

	proxy_mstudiobbox_t() {}

private:
	// No copy constructors allowed
	proxy_mstudiobbox_t(const proxy_mstudiobbox_t& vOther);
};
