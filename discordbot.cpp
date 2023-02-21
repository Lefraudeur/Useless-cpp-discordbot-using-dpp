#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <ctime>
#include <iostream>
#include <string>


class monitor {
public:
    std::string message;
    std::string state;
    std::string prev_state;
    monitor() : message(":x: offline"), state("down"), prev_state("up") {}
    void set_status(std::string status) {
        prev_state = state;
        state = status;
        if (status == "up") message = ":white_check_mark: online";
        else if (status == "maintenance" || status == "paused") message = ":warning: maintenance";
        else if (status == "validating") message = ":hourglass_flowing_sand: validating";
        else message = ":x: offline";
    }
};

/* Be sure to place your token in the line below.
 * Follow steps here to get a token:
 * https://dpp.dev/creating-a-bot-application.html
 * When you invite the bot, be sure to invite it with the
 * scopes 'bot' and 'applications.commands', e.g.
 * https://discord.com/oauth2/authorize?client_id=940762342495518720&scope=bot+applications.commands&permissions=139586816064
 */
const std::string    BOT_TOKEN = "";


int main()
{
    
    /* Create bot cluster */
    dpp::cluster bot(BOT_TOKEN, dpp::i_all_intents);

    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());
    /* Handle slash command */

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        const dpp::snowflake channel_id = 980868841364156476;
        const dpp::snowflake guild_id = 957691694466359296;
        if (event.msg.channel_id == channel_id  && event.msg.guild_id == guild_id && !(event.msg.author.is_bot())) {
            bot.message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
            bot.message_add_reaction(event.msg.id, event.msg.channel_id, "❌");
        }
    });


    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            event.reply(dpp::message("Pong!").set_flags(dpp::m_ephemeral));
        }
        if (event.command.get_command_name() == "ip") {
            event.reply(dpp::message("nightium.ml").set_flags(dpp::m_ephemeral));
        }
        if (event.command.get_command_name() == "ban") {
            dpp::snowflake userid = std::get<dpp::snowflake>(event.get_parameter("user"));
            std::string reason = "none";
            if (std::holds_alternative<std::string>(event.get_parameter("reason"))) reason = std::get<std::string>(event.get_parameter("reason"));
            bot.user_get(userid, [event, &userid, reason, &bot](const dpp::confirmation_callback_t& member) {
                if (!member.is_error()) {
                    dpp::user_identified banned(std::get<dpp::user_identified>(member.value));
                    bot.set_audit_reason(reason).guild_ban_add(event.command.guild_id, userid, 0U, [event, &userid, reason, banned](const dpp::confirmation_callback_t& ban) {
                        if (!ban.is_error()) {
                            std::srand(time(nullptr));
                            int rnd = std::rand() % 5;
                            std::string ban_messages[5] = { "got thrown out of the window", "was launched to the sky", "disappeared", "got kidnapped", "hasn't paid his bill" };
                            std::string rnd_ban_message = *(ban_messages + rnd);
                            dpp::embed embed = dpp::embed().
                                set_color(dpp::colors::red).
                                set_author("Overnode Ban", "", "https://overnode.tk/wp-content/uploads/2023/02/ban.png").
                                set_title("Banned " + banned.format_username()).
                                set_description(banned.get_mention()).
                                set_thumbnail(banned.get_avatar_url()).
                                add_field(
                                    ":hammer: " + banned.format_username() + " " + rnd_ban_message,
                                    ":page_facing_up: Reason : " + reason
                                ).
                                set_footer(dpp::embed_footer().set_text("ID: " + std::to_string(banned.id)).set_icon(banned.get_avatar_url())).
                                set_timestamp(time(0));
                            event.reply(dpp::message(event.command.channel_id, embed));
                        }
                        else {
                            event.reply(dpp::message(ban.get_error().message).set_flags(dpp::m_ephemeral));
                        }
                    });
                }
                else event.reply(dpp::message(member.get_error().message).set_flags(dpp::m_ephemeral));
            });
        }
        if (event.command.get_command_name() == "aregister") {
            std::string email = std::get<std::string>(event.get_parameter("email"));
            std::string username = std::get<std::string>(event.get_parameter("username"));
            nlohmann::json postdata = {
                {"email", email},
                {"username", username},
                {"first_name", username},
                {"last_name", username}
            };
            dpp::http_headers headers = {
                {"Accept", "application/json"},
                {"Content-Type", "application/json"},
                {"Authorization", "Bearer Pterodactyl application api key"}
            };
            bot.request("https://panel.overnode.tk/api/application/users", dpp::m_post, [event, &bot, email, username, headers](const dpp::http_request_completion_t& response) {
                if (response.status == 201) {
                    nlohmann::json content = nlohmann::json::parse(response.body);
                    std::string url = "https://panel.overnode.tk/api/application/users/" + std::to_string(int(content["attributes"]["id"]));
                    std::string password = std::get<std::string>(event.get_parameter("password"));
                    nlohmann::json patchdata = {
                        {"email", email},
                        {"username", username},
                        {"first_name", username},
                        {"last_name", username},
                        {"language", "en"},
                        {"password", password}
                    };
                    bot.request(url, dpp::m_patch, [event, email, password](const dpp::http_request_completion_t& response2){
                        if (response2.status == 200) {
                            event.reply(dpp::message("**User successfully created :**\n> https://panel.overnode.tk\nemail: `" + email + "`\npassword: ||" + password + "||").set_flags(dpp::m_ephemeral));
                        }
                        else {
                            nlohmann::json error = nlohmann::json::parse(response2.body);
                            event.reply(dpp::message("Unexpected Error 2\n```" + std::string(error["errors"][0]["detail"]) + "```").set_flags(dpp::m_ephemeral));
                        }
                    }, nlohmann::to_string(patchdata), "application/json", headers);
                }
                else {
                    nlohmann::json error = nlohmann::json::parse(response.body);
                    event.reply(dpp::message("Unexpected Error 1\n```" + std::string(error["errors"][0]["detail"]) + "```").set_flags(dpp::m_ephemeral));
                }
            }, nlohmann::to_string(postdata), "application/json", headers);
        }
    });
    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            bot.global_command_create(dpp::slashcommand("ip", "Get the IP of Nightium minecraft server", bot.me.id));
            dpp::slashcommand ban("ban", "Bans the specified user", bot.me.id);
            ban.set_default_permissions(dpp::p_ban_members);
            ban.add_option(dpp::command_option(dpp::co_user, "user", "The user you want to ban", true));
            ban.add_option(dpp::command_option(dpp::co_string, "reason", "The reason why you ban this user", false));
            bot.global_command_create(ban);
            bot.global_command_create(dpp::slashcommand("aregister", "register a user on the pterodactyl panel", bot.me.id).
                add_option(dpp::command_option(dpp::co_string, "username", "The username of the account", true)).
                add_option(dpp::command_option(dpp::co_string, "email", "The email of the user", true)).
                add_option(dpp::command_option(dpp::co_string, "password", "The password of the user", true)).
                set_default_permissions(dpp::p_administrator));
        }
    });
    monitor dashboard;
    monitor panel;
    bot.start_timer([&bot, &dashboard, &panel](const dpp::timer& timer) {
        dpp::http_headers headers = {
                {"Authorization", "Bearer BetterstatusToken"}
        };
        bot.request("https://betteruptime.com/api/v2/monitors/1058870", dpp::m_get, [&dashboard](const dpp::http_request_completion_t& status) {
            if (status.status == 200) {
                nlohmann::json response = nlohmann::json::parse(status.body);
                dashboard.set_status(response["data"]["attributes"]["status"]);
            }
        }, "", "application/json", headers);
        bot.request("https://betteruptime.com/api/v2/monitors/1059787", dpp::m_get, [&panel](const dpp::http_request_completion_t& status) {
            if (status.status == 200) {
                nlohmann::json response = nlohmann::json::parse(status.body);
                panel.set_status(response["data"]["attributes"]["status"]);
            }
        }, "", "application/json", headers);
        if ((dashboard.prev_state != dashboard.state) || (panel.prev_state != panel.state)) {
            uint32_t color = dpp::colors::red;
            std::string image = "https://overnode.tk/wp-content/uploads/2023/02/error-48268.png";
            if (dashboard.state == "down" || panel.state == "down") {
                color = dpp::colors::red;
                image = "https://overnode.tk/wp-content/uploads/2023/02/error-48268.png";
            }
            else if (dashboard.state == "maintenance" || dashboard.state == "paused" || panel.state == "maintenance" || panel.state == "paused") {
                color = dpp::colors::moon_yellow;
                image = "https://overnode.tk/wp-content/uploads/2023/02/warning.png";
            }
            else if (dashboard.state == "validating" || panel.state == "validating") {
                color = dpp::colors::gray;
                image = "https://overnode.tk/wp-content/uploads/2023/02/sablier.png";
            }
            else if (dashboard.state == "up" && panel.state == "up") {
                color = dpp::colors::green;
                image = "https://overnode.tk/wp-content/uploads/2023/02/checkmark.png";
            }
            dpp::embed embed = dpp::embed().
                set_color(color).
                set_title(":satellite: **Status**").
                set_thumbnail(image).
                add_field("***__Dashboard:__***    " + dashboard.message, "https://dash.overnode.tk").
                add_field("***__Panel:__***    " + panel.message, "https://panel.overnode.tk").
                set_footer(dpp::embed_footer().set_text("Up to 3min delay | last incident:").set_icon("https://overnode.tk/wp-content/uploads/2021/12/overnode512.png")).
                set_timestamp(time(0));
            dpp::snowflake channel_id = 963419555412185088;
            dpp::snowflake message_id = 1076649475600306256;
            dpp::message toedit(channel_id, embed);
            toedit.id = message_id;
            bot.message_edit(toedit);
        }
    },10);
    /* Start the bot */
    bot.start(false);
    return 0;
}
