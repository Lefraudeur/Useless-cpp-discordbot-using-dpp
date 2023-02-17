#include <dpp/dpp.h>
#include <ctime>
#include <iostream>
#include <string>

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
    dpp::cluster bot(BOT_TOKEN);

    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());
    /* Handle slash command */


    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
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
                                set_author("Ban Hammer", "", "https://overnode.tk/wp-content/uploads/2023/02/ban.png").
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
                            event.reply(ban.get_error().message);
                        }
                    });
                }
                else event.reply(member.get_error().message);
            });
        }
    });

    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            dpp::slashcommand ban("ban", "Bans the specified user", bot.me.id);
            ban.set_default_permissions(dpp::p_ban_members);
            ban.add_option(dpp::command_option(dpp::co_user, "user", "The user you want to ban", true));
            ban.add_option(dpp::command_option(dpp::co_string, "reason", "The reason why you ban this user", false));
            bot.global_command_create(ban);
        }
        });
    /* Start the bot */
    bot.start(false);

    return 0;
}