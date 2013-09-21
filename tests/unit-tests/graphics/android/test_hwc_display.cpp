/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/display_buffer.h"
#include "src/server/graphics/android/android_display.h"
#include "src/server/graphics/android/hwc_android_display_buffer_factory.h"

#include "mir_test_doubles/mock_hwc_interface.h"
#include "mir_test_doubles/mock_android_framebuffer_window.h"
#include "mir_test_doubles/mock_display_report.h"
#include "mir_test_doubles/mock_egl.h"

#include <memory>

namespace geom=mir::geometry;
namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace mtd=mir::test::doubles;

class AndroidTestHWCFramebuffer : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        native_win = std::make_shared<testing::NiceMock<mtd::MockAndroidFramebufferWindow>>();
        mock_hwc_device = std::make_shared<mtd::MockHWCInterface>();

        /* silence uninteresting warning messages */
        mock_egl.silence_uninteresting();
        mock_display_report = std::make_shared<mtd::MockDisplayReport>();
    }

    std::shared_ptr<mga::AndroidDisplay> create_display()
    {
        auto db_factory = std::make_shared<mga::HWCAndroidDisplayBufferFactory>(mock_hwc_device);
        return std::make_shared<mga::AndroidDisplay>(native_win, db_factory, mock_hwc_device, mock_display_report);
    }

    std::shared_ptr<mtd::MockDisplayReport> mock_display_report;
    std::shared_ptr<mtd::MockAndroidFramebufferWindow> native_win;
    std::shared_ptr<mtd::MockHWCInterface> mock_hwc_device;

    mtd::MockEGL mock_egl;
};

TEST_F(AndroidTestHWCFramebuffer, test_post_submits_right_egl_parameters)
{
    using namespace testing;

    geom::Size fake_display_size{223, 332};
    EXPECT_CALL(*mock_hwc_device, display_size())
        .Times(AnyNumber())
        .WillRepeatedly(Return(fake_display_size)); 

    auto display = create_display();

    testing::InSequence sequence_enforcer;
    EXPECT_CALL(*mock_hwc_device, commit_frame(mock_egl.fake_egl_display, mock_egl.fake_egl_surface))
        .Times(1);

    display->for_each_display_buffer([](mg::DisplayBuffer& buffer)
    {
        buffer.post_update();
    });
}

TEST_F(AndroidTestHWCFramebuffer, test_hwc_reports_size_correctly)
{
    using namespace testing;

    geom::Size fake_display_size{223, 332};
    EXPECT_CALL(*mock_hwc_device, display_size())
        .Times(AnyNumber())
        .WillRepeatedly(Return(fake_display_size)); 
    auto display = create_display();
    
    std::vector<geom::Rectangle> areas;

    display->for_each_display_buffer([&areas](mg::DisplayBuffer& buffer)
    {
        areas.push_back(buffer.view_area());
    });

    ASSERT_EQ(1u, areas.size());

    auto view_area = areas[0];

    geom::Point origin_pt{geom::X{0}, geom::Y{0}};
    EXPECT_EQ(view_area.size, fake_display_size);
    EXPECT_EQ(view_area.top_left, origin_pt);
}

TEST_F(AndroidTestHWCFramebuffer, test_dpms_configuration_changes_reach_device)
{
    using namespace testing;

    geom::Size fake_display_size{223, 332};
    EXPECT_CALL(*mock_hwc_device, display_size())
        .Times(1)
        .WillOnce(Return(fake_display_size)); 
    auto display = create_display();
    
    auto on_configuration = display->configuration();
    on_configuration->for_each_output([&](mg::DisplayConfigurationOutput const& output) -> void
    {
        on_configuration->configure_output(output.id, output.used, output.top_left, output.current_mode_index,
                                           mir_power_mode_on);
    });
    auto off_configuration = display->configuration();
    off_configuration->for_each_output([&](mg::DisplayConfigurationOutput const& output) -> void
    {
        off_configuration->configure_output(output.id, output.used, output.top_left, output.current_mode_index,
                                           mir_power_mode_off);
    });
    auto standby_configuration = display->configuration();
    standby_configuration->for_each_output([&](mg::DisplayConfigurationOutput const& output) -> void
    {
        standby_configuration->configure_output(output.id, output.used, output.top_left, output.current_mode_index,
                                           mir_power_mode_standby);
    });
    auto suspend_configuration = display->configuration();
    suspend_configuration->for_each_output([&](mg::DisplayConfigurationOutput const& output) -> void
    {
        suspend_configuration->configure_output(output.id, output.used, output.top_left, output.current_mode_index,
                                           mir_power_mode_suspend);
    });

    {
        InSequence seq;
        EXPECT_CALL(*mock_hwc_device, blank_or_unblank_screen(false));
        EXPECT_CALL(*mock_hwc_device, blank_or_unblank_screen(true));
        EXPECT_CALL(*mock_hwc_device, blank_or_unblank_screen(true));
        EXPECT_CALL(*mock_hwc_device, blank_or_unblank_screen(true));
    }
    display->configure(*on_configuration.get());
    display->configure(*off_configuration.get());
    display->configure(*suspend_configuration.get());
    display->configure(*standby_configuration.get());
}
