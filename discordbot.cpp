#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <ctime>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sqlite3.h>


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
    //create sqlite database
    sqlite3* db = nullptr;
    int rc = sqlite3_open("sqlite.db", &db);
    if (rc) {
        std::cerr << "Can't open database" << std::endl << sqlite3_errmsg(db) << std::endl;
        return 0;
    }
    else {
        std::clog << "Database openned successfuly" << std::endl;
    }
    //sql create table instruction
    std::string sql = "CREATE TABLE IF NOT EXISTS USERS (DISCORDID INT NOT NULL, PTERODACTYLID INT NOT NULL, SERVERS INT DEFAULT 0, MAXSERVERS INT DEFAULT 1, RAM INT DEFAULT 2048, STORAGE INT DEFAULT 8192);";
    char* err_msg = nullptr;
    rc = sqlite3_exec(db,  sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Could not create the table" << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    else {
        std::clog << "Table created successfuly" << std::endl;
    }

    /* Create bot cluster */
    dpp::cluster bot(BOT_TOKEN, dpp::i_all_intents);

    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        if (event.msg.channel_id == dpp::snowflake(980868841364156476) && !(event.msg.author.is_bot())) {
            bot.message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
            bot.message_add_reaction(event.msg.id, event.msg.channel_id, "❌");
        }
    });

    /* Handle slash command */
    bot.on_slashcommand([&bot, db](const dpp::slashcommand_t& event) {
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
            bot.user_get(userid, [event, userid, reason, &bot](const dpp::confirmation_callback_t& member) {
                if (!member.is_error()) {
                    dpp::user_identified banned(std::get<dpp::user_identified>(member.value));
                    bot.set_audit_reason(reason).guild_ban_add(event.command.guild_id, userid, 0U, [event, reason, banned](const dpp::confirmation_callback_t& ban) {
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
        if (event.command.get_command_name() == "panel") {
            event.reply(dpp::message("https://panel.overnode.tk").set_flags(dpp::m_ephemeral));
        }
        if (event.command.get_command_name() == "register") {
            std::string sql = "SELECT 1 FROM USERS WHERE DISCORDID=" + std::to_string(event.command.member.user_id) + ";";
            //where sql instruction result stored
            struct sqlite3_stmt* selectstmt = nullptr;
            int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &selectstmt, nullptr);
            if (rc == SQLITE_OK) {
                if (sqlite3_step(selectstmt) == SQLITE_ROW) {
                    //USER FOUND
                    event.reply(dpp::message("You are already registered").set_flags(dpp::m_ephemeral));
                }
                else {
                    //USER NOT FOUND
                    std::string email = std::get<std::string>(event.get_parameter("email"));
                    std::string username = std::get<std::string>(event.get_parameter("username"));
                    nlohmann::json postdata = {
                        {"email", email},
                        {"username", username},
                        {"first_name", username},
                        {"last_name", std::to_string(event.command.member.user_id)}
                    };
                    dpp::http_headers headers = {
                        {"Accept", "application/json"},
                        {"Content-Type", "application/json"},
                        {"Authorization", "Bearer pterodactyltoken"}
                    };
                    bot.request("https://panel.overnode.tk/api/application/users", dpp::m_post, [event, &bot, email, username, headers, db](const dpp::http_request_completion_t& response) {
                        if (response.status == 201) {
                            nlohmann::json content = nlohmann::json::parse(response.body);
                            std::string url = "https://panel.overnode.tk/api/application/users/" + std::to_string(int(content["attributes"]["id"]));
                            std::string password = std::get<std::string>(event.get_parameter("password"));
                            nlohmann::json patchdata = {
                                {"email", email},
                                {"username", username},
                                {"first_name", username},
                                {"last_name", std::to_string(event.command.member.user_id)},
                                {"language", "en"},
                                {"password", password}
                            };
                            bot.request(url, dpp::m_patch, [event, email, password, db, content](const dpp::http_request_completion_t& response2) {
                                if (response2.status == 200) {
                                    std::string sql = "INSERT INTO USERS (DISCORDID, PTERODACTYLID) VALUES (" + std::to_string(event.command.member.user_id) + ", " + std::to_string(int(content["attributes"]["id"])) + ");";
                                    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
                                    event.reply(dpp::message("**Successfully registered :**\n> https://panel.overnode.tk\nemail: `" + email + "`\npassword: ||" + password + "||").set_flags(dpp::m_ephemeral));
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
            }
            else {
                event.reply(dpp::message("Error while accessing the database\n please  report this issue to the developper").set_flags(dpp::m_ephemeral));
                std::cerr << "Error while accessing the database" << std::endl;
            }
            sqlite3_finalize(selectstmt);
        }
        if (event.command.get_command_name() == "delete") {
            if (std::get<int>(event.get_parameter("number")) >= 2 && std::get<int>(event.get_parameter("number")) <= 100) {
                bot.channel_get(event.command.channel_id, [&bot, event](const dpp::confirmation_callback_t& callback) {
                    dpp::channel channel(std::get<dpp::channel>(callback.value));
                    dpp::message_map messages = bot.messages_get_sync(event.command.channel_id, 0, channel.last_message_id, 0, std::get<int>(event.get_parameter("number")));
                });
            }
            else {
                event.reply("You can delete only from 2 to 100 messages at a time");
            }
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
            bot.global_command_create(dpp::slashcommand("panel", "Get the link of the pterodactyl panel", bot.me.id));
            bot.global_command_create(dpp::slashcommand("register", "Register yourself on the pterodactyl panel", bot.me.id).
                set_default_permissions(dpp::p_administrator).
                add_option(dpp::command_option(dpp::co_string, "email", "Your email to login, please use a valid one", true)).
                add_option(dpp::command_option(dpp::co_string, "username", "The username you want on the panel", true)).
                add_option(dpp::command_option(dpp::co_string, "password", "Unique sercret password you will use to login to the panel", true)));
            bot.global_command_create(dpp::slashcommand("delete", "delete a certain number of messages", bot.me.id).
                set_default_permissions(dpp::p_manage_messages).
                add_option(dpp::command_option(dpp::co_integer, "number", "The number of messages you want to delete", true)));
        }
    });
    monitor dashboard;
    monitor panel;
    bot.start_timer([&bot, &dashboard, &panel](const dpp::timer& timer) {
        dpp::http_headers headers = {
                {"Authorization", "Bearer Betteruptimetoken"}
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
    sqlite3_close(db);
    return 0;
}