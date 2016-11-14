/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2014 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "TerrainRenderData"
#include "TileNode"
#include "SurfaceNode"

using namespace osgEarth::Drivers::RexTerrainEngine;

#undef  LC
#define LC "[TerrainRenderData] "


void
TerrainRenderData::sortDrawCommands()
{
    for (LayerDrawableList::iterator i = _layerList.begin(); i != _layerList.end(); ++i)
    {
        i->get()->_tiles.sort();
    }
}

void
TerrainRenderData::setup(const MapFrame& frame, const RenderBindings& bindings, osg::StateSet* defaultStateSet, unsigned frameNum)
{
    _bindings = &bindings;

    // Create a new State object to track sampler and uniform settings
    _drawState = new DrawState();
    _drawState->_frame = frameNum;
    _drawState->_bindings = &bindings;

    // Make a drawable for each rendering pass (i.e. each render-able map layer).
    for(LayerVector::const_iterator i = frame.layers().begin();
        i != frame.layers().end();
        ++i)
    {
        Layer* layer = i->get();
        if (layer->getRenderType() != Layer::RENDERTYPE_NONE && layer->getEnabled())
        {
            bool render = true;

            // If this is an image layer, check the enabled/visible states.
            ImageLayer* imageLayer = dynamic_cast<ImageLayer*>(layer);
            if (imageLayer)
            {
                render = imageLayer->getVisible();
            }

            if (render)
            {
                addLayerDrawable(layer);

                // Make a list of "global" layers. There are layers whose data is not
                // represented in the TerrainTileModel, like a splatting layer or a patch
                // layer. The data for these is dynamic and not based on data fetched.
                if (imageLayer == 0 && layer->getRenderType() == Layer::RENDERTYPE_TILE)
                {
                    tileLayers().push_back(layer);
                }
                else if (layer->getRenderType() == Layer::RENDERTYPE_PATCH)
                {
                    patchLayers().push_back(dynamic_cast<PatchLayer*>(layer));
                }
            }
        }
    }
}

LayerDrawable*
TerrainRenderData::addLayerDrawable(const Layer* layer)
{
    UID uid = layer ? layer->getUID() : -1;
    LayerDrawable* ld = new LayerDrawable();
    _layerList.push_back(ld);
    _layerMap[uid] = ld;
    ld->_layer = layer;
    ld->_imageLayer = dynamic_cast<const ImageLayer*>(layer);
    ld->_order = _layerList.size() - 1;
    ld->_drawState = _drawState.get();
    if (layer)
        ld->setStateSet(layer->getStateSet());

    return ld;
}