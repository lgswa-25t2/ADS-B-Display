//---------------------------------------------------------------------------

#ifndef TileManagerH
#define TileManagerH
//---------------------------------------------------------------------------
#include <map>

#include "global.h"
#include "TextureTile.h"
#include "TileStorage.h"

#define DEFAULT_MAX_TEXTURES 500

/**
 * Tile manager.
 *
 * Handles requests for tiles, starts retrival of needed tiles from
 * storages, removes unneeded tiles from memory.
 *
 * @deprecated should be restructured and merged to GoogleLayer
 */
class TileManager {
public:
	/**
	 * Constructor.
	 *
	 * @param ts tile storage to pass tile to for loading
	 */
	TileManager(TileStorage *ts);

	/**
	 * Destructor.
	 */
	virtual ~TileManager();

	/**
	 * Gets texture tile with given coords.
	 */
	TextureTilePtr GetTexture(int x, int y, int level);

	/**
	 * Removes unneeded objects from memory.
	 *
	 * Currently removes one texture each call, until number
	 * of textures >= DEFAULT_MAX_TEXTURES
	 *
	 * @todo implement tunable limits
	 */
	int Cleanup();

	/**
	 * Set new tile storage for loading tiles.
	 *
	 * @param ts new tile storage to use
	 */
	void SetTileStorage(TileStorage *ts);

	/**
	 * Set maximum number of textures to keep in memory.
	 *
	 * @param maxTextures maximum number of textures
	 */
	void SetMaxTextures(int maxTextures);

	/**
	 * Get current number of textures in memory.
	 */
	int GetTextureCount() const;

	/**
	 * Get maximum number of textures allowed.
	 */
	int GetMaxTextures() const;

private:
	/**
	 * Finds best texture tile to remove from memory.
	 */
	TextureTilePtr FindTextureToDrop(TextureTilePtr cur, TextureTilePtr best);

private:
	TileStorage	*m_FirstTileStorage;	///< Tile storage to pass tile to for loading

	int		m_nTextureTiles;	///< Root element of TextureTile quadtree
	TextureTilePtr	m_TextureRoot;		///< Number of texture tiles in a tree
	int		m_MaxTextures;		///< Maximum number of textures to keep in memory
};


#endif
