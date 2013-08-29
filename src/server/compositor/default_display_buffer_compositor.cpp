/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "default_display_buffer_compositor.h"

#include "mir/compositor/rendering_operator.h"
#include "mir/compositor/overlay_renderer.h"
#include "mir/compositor/scene.h"
#include "mir/compositor/compositing_criteria.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/buffer.h"
#include "mir/surfaces/buffer_stream.h"
#include "bypass.h"
#include <mutex>
#include <cstdlib>

namespace mc = mir::compositor;
namespace mg = mir::graphics;

namespace
{

struct FilterForVisibleSceneInRegion : public mc::FilterForScene
{
    FilterForVisibleSceneInRegion(mir::geometry::Rectangle const& enclosing_region)
        : enclosing_region(enclosing_region)
    {
    }
    bool operator()(mc::CompositingCriteria const& info)
    {
        return info.should_be_rendered_in(enclosing_region);
    }

    mir::geometry::Rectangle const& enclosing_region;
};

std::mutex global_frame_count_lock;
unsigned long global_frame_count = 0;

bool wrapped_greater_or_equal(unsigned long a, unsigned long b)
{
    return (a - b) < (~0UL / 2UL);
}

}

mc::DefaultDisplayBufferCompositor::DefaultDisplayBufferCompositor(
    mg::DisplayBuffer& display_buffer,
    std::shared_ptr<mc::Scene> const& scene,
    std::shared_ptr<mc::Renderer> const& renderer,
    std::shared_ptr<mc::OverlayRenderer> const& overlay_renderer)
    : mc::BasicDisplayBufferCompositor{display_buffer},
      scene{scene},
      renderer{renderer},
      overlay_renderer{overlay_renderer},
      local_frame_count{global_frame_count}
{
}


void mc::DefaultDisplayBufferCompositor::composite()
{
    static bool got_bypass_env = false;
    static bool bypass_env = true;
    bool bypassed = false;

    /*
     * Increment frame counts for each tick of the fastest instance of
     * DefaultDisplayBufferCompositor. This means for the fastest refresh
     * rate of all attached outputs.
     */
    local_frame_count++;
    {
        std::lock_guard<std::mutex> lock(global_frame_count_lock);
        if (wrapped_greater_or_equal(local_frame_count, global_frame_count))
            global_frame_count = local_frame_count;
        else
            local_frame_count = global_frame_count;
    }

    if (!got_bypass_env)
    {
        const char *env = getenv("MIR_BYPASS");
        if (env != NULL)
            bypass_env = env[0] != '0';

        got_bypass_env = true;
    }

    if (bypass_env && display_buffer.can_bypass())
    {
        std::unique_lock<Scene> lock(*scene);

        mc::BypassFilter filter(display_buffer);
        mc::BypassMatch match;

        // It would be *really* nice if Scene had an iterator to simplify this
        scene->for_each_if(filter, match);

        if (filter.fullscreen_on_top())
        {
            auto bypass_buf =
                match.topmost_fullscreen()->lock_compositor_buffer(
                    local_frame_count);

            if (bypass_buf->can_bypass())
            {
                lock.unlock();
                display_buffer.post_update(bypass_buf);
                bypassed = true;
            }
        }
    }

    if (!bypassed)
        mc::BasicDisplayBufferCompositor::composite();
}

void mc::DefaultDisplayBufferCompositor::compose(
    mir::geometry::Rectangle const& view_area,
    std::function<void(std::shared_ptr<void> const&)> save_resource)
{
    renderer->clear(local_frame_count);

    mc::RenderingOperator applicator(*renderer, save_resource);
    FilterForVisibleSceneInRegion selector(view_area);
    scene->for_each_if(selector, applicator);

    overlay_renderer->render(view_area, save_resource);
}
