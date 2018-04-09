/*
 * Copyright © 2015-2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "wl_subcompositor.h"

#include "wl_surface.h"

namespace mf = mir::frontend;
namespace geom = mir::geometry;

mf::WlSubcompositor::WlSubcompositor(struct wl_display* display)
    : wayland::Subcompositor(display, 1)
{}

void mf::WlSubcompositor::destroy(struct wl_client* /*client*/, struct wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void mf::WlSubcompositor::get_subsurface(struct wl_client* client, struct wl_resource* resource, uint32_t id,
                                         struct wl_resource* surface, struct wl_resource* parent)
{
    new WlSubsurface(client, resource, id, WlSurface::from(surface), WlSurface::from(parent));
}

mf::WlSubsurface::WlSubsurface(struct wl_client* client, struct wl_resource* object_parent, uint32_t id,
                               WlSurface* surface, WlSurface* parent_surface)
    : wayland::Subsurface(client, object_parent, id),
      surface{surface},
      parent{parent_surface->add_child(this)},
      synchronized_{true}
{
    surface->set_role(this);
}

mf::WlSubsurface::~WlSubsurface()
{
    // unique pointer automatically removes `this` from parent child list

    invalidate_buffer_list();
    surface->clear_role();
}

void mf::WlSubsurface::populate_buffer_list(std::vector<shell::StreamSpecification>& buffers,
                                            geometry::Displacement const& parent_offset) const
{
    surface->populate_buffer_list(buffers, parent_offset);
}

bool mf::WlSubsurface::synchronized() const
{
    return synchronized_ || parent->synchronized();
}

void mf::WlSubsurface::parent_has_committed()
{
    if (cached_state && synchronized())
    {
        surface->commit(std::move(cached_state.value()));
        cached_state = std::experimental::nullopt;
    }
}

std::pair<geom::Point, wl_resource*> mf::WlSubsurface::transform_point(geom::Point point) const
{
    return surface->transform_point(point);
}

void mf::WlSubsurface::set_position(int32_t x, int32_t y)
{
    surface->set_buffer_offset(geom::Displacement{x, y});
}

void mf::WlSubsurface::place_above(struct wl_resource* sibling)
{
    (void)sibling;
    // TODO
}

void mf::WlSubsurface::place_below(struct wl_resource* sibling)
{
    (void)sibling;
    // TODO
}

void mf::WlSubsurface::set_sync()
{
    synchronized_ = true;
}

void mf::WlSubsurface::set_desync()
{
    synchronized_ = false;
}

void mf::WlSubsurface::destroy()
{
    wl_resource_destroy(resource);
}

void mf::WlSubsurface::invalidate_buffer_list()
{
    parent->invalidate_buffer_list();
}

void mf::WlSubsurface::commit(WlSurfaceState const& state)
{
    if (synchronized())
    {
        if (!cached_state)
            cached_state = WlSurfaceState();

        cached_state.value().update_from(state);
    }
    else
    {
        if (cached_state) // unusual
        {
            cached_state.value().update_from(state);
            surface->commit(std::move(cached_state.value()));
        }
        else
        {
            surface->commit(state);
        }
    }
}

void mf::WlSubsurface::visiblity(bool visible)
{
    (void)visible;
    // TODO
}
