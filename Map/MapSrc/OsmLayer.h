//---------------------------------------------------------------------------Add commentMore actions

#ifndef OsmLayerH
#define OsmLayerH

#include "MasterLayer.h"
#include "TileManager.h"

#define MIN_TEXTURE_DISTANCE 192.0

class OsmLayer: public MasterLayer {
public:
	OsmLayer(TileManager *tm);
	virtual ~OsmLayer();

	void RenderRegion(Region *rgn);

	int GetSplitLevel(double wlen, double plen);

protected:
	TileManager	*m_TileManager;
};
//---------------------------------------------------------------------------
#endif