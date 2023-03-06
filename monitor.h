#pragma once
#include <string>
#include <dpp/dpp.h>

namespace status {
    class monitor {
    public:
        std::string message;
        std::string state;
        std::string prev_state;
        std::string name;
        std::string id;
        std::string url;
        monitor() : message(":x: offline"), state("down"), prev_state("up"), name("name"), id("id") {}
        monitor(std::string name, std::string id, std::string url = "https://overnode.tk") : message(":x: offline"), state("down"), prev_state("up"), name(name), id(id), url(url) {}
        monitor(const monitor& copy) {
            message = copy.message;
            state = copy.state;
            prev_state = copy.prev_state;
        }
        void set_status(std::string status = "down") {
            prev_state = state;
            state = status;
            if (status == "up") message = ":white_check_mark: online";
            else if (status == "maintenance" || status == "paused") message = ":warning: maintenance";
            else if (status == "validating") message = ":hourglass_flowing_sand: validating";
            else message = ":x: offline";
        }
        bool changed() {
            return prev_state != state;
        }
    };
    uint32_t get_color(monitor input[], const int& size) {
        monitor parse;
        uint32_t color = dpp::colors::green;
        for (int i = 0; i < size; ++i) {
            parse = *(input + i);
            if (parse.state == "down") {
                color = dpp::colors::red;
                return color;
            }
        }
        for (int i = 0; i < size; ++i) {
            parse = *(input + i);
            if (parse.state == "maintenance" || parse.state == "paused") {
                color = dpp::colors::yellow;
                return color;
            }
        }
        for (int i = 0; i < size; ++i) {
            parse = *(input + i);
            if (parse.state == "validating") {
                color = dpp::colors::gray;
                return color;
            }
        }
        return color;
    }
    std::string get_image(uint32_t color) {
        if (color == dpp::colors::red) return "https://overnode.tk/wp-content/uploads/2023/02/error-48268.png";
        else if (color == dpp::colors::yellow) return "https://overnode.tk/wp-content/uploads/2023/02/warning.png";
        else if (color == dpp::colors::gray) return "https://overnode.tk/wp-content/uploads/2023/02/sablier.png";
        else if (color == dpp::colors::green) return "https://overnode.tk/wp-content/uploads/2023/02/checkmark.png";
        return "https://overnode.tk/wp-content/uploads/2023/02/error-48268.png";
    }
    bool has_changed(monitor input[], const int& size) {
        monitor parse;
        for (int i = 0; i < size; ++i) {
            parse = *(input + i);
            if (parse.changed()) return true;
        }
        return false;
    }
}