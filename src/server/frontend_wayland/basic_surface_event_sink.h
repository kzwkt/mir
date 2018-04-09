/*
 * Copyright © 2018 Canonical Ltd.
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

#ifndef MIR_FRONTEND_BASIC_EVENT_SINK_H_
#define MIR_FRONTEND_BASIC_EVENT_SINK_H_

#include "mir/frontend/event_sink.h"

struct wl_client;

namespace mir
{
namespace frontend
{
class WlSurface;
class WlSeat;
class WlAbstractMirWindow;

class BasicSurfaceEventSink : public EventSink
{
public:
    BasicSurfaceEventSink(WlSeat* seat, wl_client* client, WlSurface* surface, WlAbstractMirWindow* window);
    ~BasicSurfaceEventSink();

    void handle_event(EventUPtr&& event) override;

    void handle_lifecycle_event(MirLifecycleState) override {}
    void handle_display_config_change(graphics::DisplayConfiguration const&) override {}
    void send_ping(int32_t) override {}
    void send_buffer(BufferStreamId, graphics::Buffer&, graphics::BufferIpcMsgType) override {}
    void handle_input_config_change(MirInputConfig const&) override {}
    void handle_error(ClientVisibleError const&) override {}
    void add_buffer(graphics::Buffer&) override {}
    void error_buffer(geometry::Size, MirPixelFormat, std::string const&) override {}
    void update_buffer(graphics::Buffer&) override {}

    void latest_client_size(geometry::Size window_size)
    {
        this->window_size = window_size;
    }

    auto latest_timestamp_ns() const -> uint64_t
    {
        return timestamp_ns;
    }

    auto is_active() const -> bool
    {
        return has_focus;
    }

    auto state() const -> MirWindowState
    {
        return current_state;
    }

protected:
    WlSeat* const seat;
    wl_client* const client;
    WlSurface* const surface;
    WlAbstractMirWindow* window;
    geometry::Size window_size;
    int64_t timestamp_ns{0};
    geometry::Size requested_size;
    bool has_focus{false};
    MirWindowState current_state{mir_window_state_unknown};
    std::shared_ptr<bool> const destroyed;

private:
    void handle_resize_event(MirResizeEvent const* event);
    void handle_input_event(MirInputEvent const* event);
    void handle_keymap_event(MirKeymapEvent const* event);
    void handle_window_event(MirWindowEvent const* event);
};
}
}

#endif //MIR_FRONTEND_BASIC_EVENT_SINK_H_
