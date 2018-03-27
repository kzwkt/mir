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

#include <memory>

namespace mir
{
class Executor;
namespace frontend
{
class WindowRole;

class WindowEventSink : public EventSink
{
public:
    WindowEventSink(WindowRole* target, std::shared_ptr<mir::Executor> executor, std::shared_ptr<bool> destroyed);

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

private:


    std::shared_ptr<mir::Executor> const executor;
    WindowRole* const target;
    std::shared_ptr<bool> const destroyed;
};


/*
class WlSeat;

class BasicSurfaceEventSink : public NullEventSink
{
public:
    BasicSurfaceEventSink(WlSeat* seat, wl_client* client, wl_resource* target, wl_resource* event_sink)
        : seat{seat},
        client{client},
        target{target},
        event_sink{event_sink},
        window_size{geometry::Size{0,0}}
    {
    }

    void handle_event(MirEvent const& e) override;

    void latest_client_size(geometry::Size window_size)
    {
        this->window_size = window_size;
    }

    auto latest_timestamp() const -> uint64_t
    {
        return timestamp;
    }

    auto is_active() const -> bool
    {
        return has_focus;
    }

    auto state() const -> MirWindowState
    {
        return current_state;
    }

    virtual void send_resize(geometry::Size const& new_size) const = 0;

protected:
    WlSeat* const seat;
    wl_client* const client;
    wl_resource* const target;
    wl_resource* const event_sink;
    std::atomic<geometry::Size> window_size;
    std::atomic<int64_t> timestamp{0};
    std::atomic<geometry::Size> requested_size;
    std::atomic<bool> has_focus{false};
    std::atomic<MirWindowState> current_state{mir_window_state_unknown};
};
*/
}
}

#endif //MIR_FRONTEND_BASIC_EVENT_SINK_H_
