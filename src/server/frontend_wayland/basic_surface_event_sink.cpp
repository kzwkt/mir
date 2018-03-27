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

#include "basic_surface_event_sink.h"

#include "window_role.h"
#include "wl_seat.h"
#include "wayland_utils.h"
#include "wayland_executor.h"

#include <mir/events/event.h>
#include <mir/geometry/size.h>

namespace mf = mir::frontend;

mf::WindowEventSink::WindowEventSink(WindowRole* target, std::shared_ptr<mir::Executor> executor, std::shared_ptr<bool> destroyed)
    : executor{executor},
      target{target},
      destroyed{destroyed}
{}

void mf::WindowEventSink::handle_event(EventUPtr&& event)
{
    executor->spawn(run_unless(
        destroyed,
        [event = event, target = target]()
        {
            switch (mir_event_get_type(event.get()))
            {
            case mir_event_type_resize:
            {
                //auto* const resize_event = mir_event_get_resize_event(event.get());
                //geometry::Size requested_size = {mir_resize_event_get_width(resize_event),
                //                                 mir_resize_event_get_height(resize_event)};
                //target->set_size(requested_size);
                break;
            }
            case mir_event_type_input:
            {
                auto input_event = mir_event_get_input_event(event.get());

                // Remember the timestamp of any events "signed" with a cookie
                //if (mir_input_event_has_cookie(input_event))
                //    timestamp = mir_input_event_get_event_time(input_event);

                switch (mir_input_event_get_type(input_event))
                {
                case mir_input_event_type_key:
                    //target->seat()->handle_keyboard_event(client, input_event, target);
                    break;
                case mir_input_event_type_pointer:
                    //seat->handle_pointer_event(client, input_event, target);
                    break;
                case mir_input_event_type_touch:
                    //seat->handle_touch_event(client, input_event, target);
                    break;
                default:
                    break;
                }
                break;
            }
            case mir_event_type_keymap:
            {
                //auto const map_ev = mir_event_get_keymap_event(event.get());

                //seat->handle_event(client, map_ev, target);
                break;
            }
            case mir_event_type_window:
            {
                auto const wev = mir_event_get_window_event(event.get());

                switch (mir_window_event_get_attribute(wev))
                {
                case mir_window_attrib_focus:
                    has_focus = mir_window_event_get_attribute_value(wev);
                    send_resize(requested_size);
                    break;

                case mir_window_attrib_state:
                    current_state = MirWindowState(mir_window_event_get_attribute_value(wev));
                    send_resize(requested_size);
                    break;

                default:;
                }

                seat->handle_event(client, wev, target);
            }
            default:
                break;
            }
        }
    ))
    /*
    switch (mir_event_get_type(&event))
    {
    case mir_event_type_resize:
    {
        auto* const resize_event = mir_event_get_resize_event(&event);
        requested_size = {mir_resize_event_get_width(resize_event), mir_resize_event_get_height(resize_event)};
        if (requested_size != window_size)
            send_resize(requested_size);
        break;
    }
    case mir_event_type_input:
    {
        auto input_event = mir_event_get_input_event(&event);

        // Remember the timestamp of any events "signed" with a cookie
        if (mir_input_event_has_cookie(input_event))
            timestamp = mir_input_event_get_event_time(input_event);

        switch (mir_input_event_get_type(input_event))
        {
        case mir_input_event_type_key:
            seat->handle_keyboard_event(client, input_event, target);
            break;
        case mir_input_event_type_pointer:
            seat->handle_pointer_event(client, input_event, target);
            break;
        case mir_input_event_type_touch:
            seat->handle_touch_event(client, input_event, target);
            break;
        default:
            break;
        }
        break;
    }
    case mir_event_type_keymap:
    {
        auto const map_ev = mir_event_get_keymap_event(&event);

        seat->handle_event(client, map_ev, target);
        break;
    }
    case mir_event_type_window:
    {
        auto const wev = mir_event_get_window_event(&event);

        switch (mir_window_event_get_attribute(wev))
        {
        case mir_window_attrib_focus:
            has_focus = mir_window_event_get_attribute_value(wev);
            send_resize(requested_size);
            break;

        case mir_window_attrib_state:
            current_state = MirWindowState(mir_window_event_get_attribute_value(wev));
            send_resize(requested_size);
            break;

        default:;
        }

        seat->handle_event(client, wev, target);
    }
    default:
        break;
    }
    */
}
