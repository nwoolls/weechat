/*
 * irc-bar-item.c - bar items for IRC plugin
 *
 * Copyright (C) 2003-2014 Sébastien Helleu <flashcode@flashtux.org>
 *
 * This file is part of WeeChat, the extensible chat client.
 *
 * WeeChat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * WeeChat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WeeChat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../weechat-plugin.h"
#include "irc.h"
#include "irc-buffer.h"
#include "irc-color.h"
#include "irc-config.h"
#include "irc-server.h"
#include "irc-channel.h"
#include "irc-nick.h"


/*
 * Returns content of bar item "away": bar item with away indicator.
 */

char *
irc_bar_item_away (void *data, struct t_gui_bar_item *item,
                   struct t_gui_window *window, struct t_gui_buffer *buffer,
                   struct t_hashtable *extra_info)
{
    struct t_irc_server *server;
    char *buf, *message;
    int length;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    buf = NULL;

    irc_buffer_get_server_and_channel (buffer, &server, NULL);

    if (server && server->is_away)
    {
        if (weechat_config_boolean (irc_config_look_item_away_message)
            && server->away_message && server->away_message[0])
        {
            message = strdup (server->away_message);
        }
        else
        {
            message = strdup (_("away"));
        }
        if (message)
        {
            length = strlen (message) + 64 + 1;
            buf = malloc (length);
            if (buf)
            {
                snprintf (buf, length, "%s%s",
                          IRC_COLOR_ITEM_AWAY,
                          message);
            }
            free (message);
        }
    }

    return buf;
}

/*
 * Returns content of bar item "buffer_title": bar item with buffer title.
 */

char *
irc_bar_item_buffer_title (void *data, struct t_gui_bar_item *item,
                           struct t_gui_window *window,
                           struct t_gui_buffer *buffer,
                           struct t_hashtable *extra_info)
{
    const char *title;
    char *title_color;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    title = weechat_buffer_get_string (buffer, "title");
    if (!title)
        return NULL;

    title_color = irc_color_decode (title,
                                    (weechat_config_boolean (irc_config_look_topic_strip_colors)) ?
                                    0 : 1);

    return (title_color) ? title_color : strdup (title);
}

/*
 * Returns content of bar item "buffer_plugin": bar item with buffer plugin.
 */

char *
irc_bar_item_buffer_plugin (void *data, struct t_gui_bar_item *item,
                            struct t_gui_window *window,
                            struct t_gui_buffer *buffer,
                            struct t_hashtable *extra_info)
{
    char buf[512];
    struct t_weechat_plugin *ptr_plugin;
    const char *name;
    struct t_irc_server *server;
    struct t_irc_channel *channel;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    ptr_plugin = weechat_buffer_get_pointer (buffer, "plugin");
    name = weechat_plugin_get_name (ptr_plugin);
    if (ptr_plugin == weechat_irc_plugin)
    {
        irc_buffer_get_server_and_channel (buffer, &server, &channel);
        if (server && channel
            && (weechat_config_integer (irc_config_look_item_display_server) == IRC_CONFIG_LOOK_ITEM_DISPLAY_SERVER_PLUGIN))
        {
            snprintf (buf, sizeof (buf), "%s%s/%s%s",
                      name,
                      IRC_COLOR_BAR_DELIM,
                      IRC_COLOR_BAR_FG,
                      server->name);
        }
        else
        {
            snprintf (buf, sizeof (buf), "%s", name);
        }
    }
    else
    {
        snprintf (buf, sizeof (buf), "%s", name);
    }
    return strdup (buf);
}

/*
 * Returns content of bar item "buffer_name": bar item with buffer name.
 */

char *
irc_bar_item_buffer_name (void *data, struct t_gui_bar_item *item,
                          struct t_gui_window *window,
                          struct t_gui_buffer *buffer,
                          struct t_hashtable *extra_info)
{
    char buf[512], buf_name[256], modes[128];
    const char *name;
    int part_from_channel, display_server;
    struct t_irc_server *server;
    struct t_irc_channel *channel;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    buf_name[0] = '\0';
    modes[0] = '\0';

    display_server = (weechat_config_integer (irc_config_look_item_display_server) == IRC_CONFIG_LOOK_ITEM_DISPLAY_SERVER_NAME);

    irc_buffer_get_server_and_channel (buffer, &server, &channel);
    if (server || channel)
    {
        if (server && !channel)
        {
            snprintf (buf_name, sizeof (buf_name), "%s%s[%s%s%s]",
                      _("server"),
                      IRC_COLOR_BAR_DELIM,
                      (server && server->ssl_connected) ? IRC_COLOR_STATUS_NAME_SSL : IRC_COLOR_STATUS_NAME,
                      server->name,
                      IRC_COLOR_BAR_DELIM);
        }
        else
        {
            if (channel)
            {
                part_from_channel = ((channel->type == IRC_CHANNEL_TYPE_CHANNEL)
                                     && !channel->nicks);
                snprintf (buf_name, sizeof (buf_name),
                          "%s%s%s%s%s%s%s%s%s%s",
                          (part_from_channel) ? IRC_COLOR_BAR_DELIM : "",
                          (part_from_channel) ? "(" : "",
                          (server && server->ssl_connected) ? IRC_COLOR_STATUS_NAME_SSL : IRC_COLOR_STATUS_NAME,
                          (server && display_server) ? server->name : "",
                          (server && display_server) ? IRC_COLOR_BAR_DELIM : "",
                          (server && display_server) ? "/" : "",
                          (server && server->ssl_connected) ? IRC_COLOR_STATUS_NAME_SSL : IRC_COLOR_STATUS_NAME,
                          channel->name,
                          (part_from_channel) ? IRC_COLOR_BAR_DELIM : "",
                          (part_from_channel) ? ")" : "");
            }
        }
    }
    else
    {
        name = weechat_buffer_get_string (buffer, "name");
        if (name)
            snprintf (buf_name, sizeof (buf_name), "%s", name);
    }

    snprintf (buf, sizeof (buf), "%s%s%s",
              (server && server->ssl_connected) ? IRC_COLOR_STATUS_NAME_SSL : IRC_COLOR_STATUS_NAME,
              buf_name,
              modes);

    return strdup (buf);
}

/*
 * Returns content of bar item "buffer_modes": bar item with buffer modes.
 */

char *
irc_bar_item_buffer_modes (void *data, struct t_gui_bar_item *item,
                           struct t_gui_window *window,
                           struct t_gui_buffer *buffer,
                           struct t_hashtable *extra_info)
{
    char modes[128], *modes_without_args;
    const char *pos_space, *pos_key;
    int part_from_channel;
    struct t_irc_server *server;
    struct t_irc_channel *channel;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    modes[0] = '\0';

    irc_buffer_get_server_and_channel (buffer, &server, &channel);
    if (!channel)
        return NULL;

    part_from_channel = ((channel->type == IRC_CHANNEL_TYPE_CHANNEL)
                         && !channel->nicks);
    if (!part_from_channel
        && channel->modes && channel->modes[0]
        && (strcmp (channel->modes, "+") != 0))
    {
        modes_without_args = NULL;
        if (weechat_config_boolean (irc_config_look_item_channel_modes_hide_key))
        {
            pos_space = strchr(channel->modes, ' ');
            if (pos_space)
            {
                pos_key = strchr(channel->modes, 'k');
                if (pos_key && (pos_key < pos_space))
                {
                    modes_without_args = weechat_strndup (channel->modes,
                                                          pos_space - channel->modes);
                }
            }
        }
        snprintf (modes, sizeof (modes),
                  "%s%s",
                  IRC_COLOR_ITEM_CHANNEL_MODES,
                  (modes_without_args) ? modes_without_args : channel->modes);
        if (modes_without_args)
            free (modes_without_args);
        return strdup (modes);
    }

    return NULL;
}

/*
 * Returns content of bar item "irc_channel": bar item with channel name
 * (without modes).
 */

char *
irc_bar_item_channel (void *data, struct t_gui_bar_item *item,
                      struct t_gui_window *window, struct t_gui_buffer *buffer,
                      struct t_hashtable *extra_info)
{
    char buf[512], buf_name[256], modes[128];
    const char *name;
    int part_from_channel, display_server;
    struct t_irc_server *server;
    struct t_irc_channel *channel;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    buf_name[0] = '\0';
    modes[0] = '\0';

    display_server = (weechat_config_integer (irc_config_look_item_display_server) == IRC_CONFIG_LOOK_ITEM_DISPLAY_SERVER_NAME);

    irc_buffer_get_server_and_channel (buffer, &server, &channel);
    if (server || channel)
    {
        if (server && !channel)
        {
            snprintf (buf_name, sizeof (buf_name), "%s%s[%s%s%s]",
                      _("server"),
                      IRC_COLOR_BAR_DELIM,
                      IRC_COLOR_STATUS_NAME,
                      server->name,
                      IRC_COLOR_BAR_DELIM);
        }
        else
        {
            if (channel)
            {
                part_from_channel = ((channel->type == IRC_CHANNEL_TYPE_CHANNEL)
                                     && !channel->nicks);
                snprintf (buf_name, sizeof (buf_name),
                          "%s%s%s%s%s%s%s%s%s%s",
                          (part_from_channel) ? IRC_COLOR_BAR_DELIM : "",
                          (part_from_channel) ? "(" : "",
                          IRC_COLOR_STATUS_NAME,
                          (server && display_server) ? server->name : "",
                          (server && display_server) ? IRC_COLOR_BAR_DELIM : "",
                          (server && display_server) ? "/" : "",
                          IRC_COLOR_STATUS_NAME,
                          channel->name,
                          (part_from_channel) ? IRC_COLOR_BAR_DELIM : "",
                          (part_from_channel) ? ")" : "");
            }
        }
    }
    else
    {
        name = weechat_buffer_get_string (buffer, "name");
        if (name)
            snprintf (buf_name, sizeof (buf_name), "%s", name);
    }

    snprintf (buf, sizeof (buf), "%s%s%s",
              IRC_COLOR_STATUS_NAME,
              buf_name,
              modes);

    return strdup (buf);
}

/*
 * Returns content of bar item "lag": bar item with lag value.
 */

char *
irc_bar_item_lag (void *data, struct t_gui_bar_item *item,
                  struct t_gui_window *window, struct t_gui_buffer *buffer,
                  struct t_hashtable *extra_info)
{
    char buf[128];
    struct t_irc_server *server;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    irc_buffer_get_server_and_channel (buffer, &server, NULL);

    if (server
        && (server->lag >= weechat_config_integer (irc_config_network_lag_min_show)))
    {
        snprintf (buf, sizeof (buf),
                  ((server->lag_check_time.tv_sec == 0) || (server->lag < 1000)) ?
                  "%s: %s%.3f" : "%s: %s%.0f",
                  _("Lag"),
                  (server->lag_check_time.tv_sec == 0) ?
                  IRC_COLOR_ITEM_LAG_FINISHED : IRC_COLOR_ITEM_LAG_COUNTING,
                  ((float)(server->lag)) / 1000);
        return strdup (buf);
    }

    return NULL;
}

/*
 * Returns content of bar item "input_prompt": bar item with input prompt.
 */

char *
irc_bar_item_input_prompt (void *data, struct t_gui_bar_item *item,
                           struct t_gui_window *window,
                           struct t_gui_buffer *buffer,
                           struct t_hashtable *extra_info)
{
    struct t_irc_server *server;
    struct t_irc_channel *channel;
    struct t_irc_nick *ptr_nick;
    char *buf, str_prefix[64];
    int length;

    /* make C compiler happy */
    (void) data;
    (void) item;
    (void) window;
    (void) extra_info;

    if (!buffer)
        return NULL;

    irc_buffer_get_server_and_channel (buffer, &server, &channel);
    if (!server || !server->nick)
        return NULL;

    /* build prefix */
    str_prefix[0] = '\0';
    if (weechat_config_boolean (irc_config_look_item_nick_prefix)
        && channel
        && (channel->type == IRC_CHANNEL_TYPE_CHANNEL))
    {
        ptr_nick = irc_nick_search (server, channel, server->nick);
        if (ptr_nick)
        {
            if (ptr_nick->prefix[0] != ' ')
            {
                snprintf (str_prefix, sizeof (str_prefix), "%s%s",
                          weechat_color (irc_nick_get_prefix_color_name (server, ptr_nick->prefix[0])),
                          ptr_nick->prefix);
            }
        }
    }

    /* build bar item */
    length = 64 + strlen (server->nick) + 64 +
        ((server->nick_modes) ? strlen (server->nick_modes) : 0) + 64 + 1;

    buf = malloc (length);
    if (buf)
    {
        if (weechat_config_boolean (irc_config_look_item_nick_modes)
            && server->nick_modes && server->nick_modes[0])
        {
            snprintf (buf, length, "%s%s%s%s(%s%s%s)",
                      str_prefix,
                      IRC_COLOR_INPUT_NICK,
                      server->nick,
                      IRC_COLOR_BAR_DELIM,
                      IRC_COLOR_ITEM_NICK_MODES,
                      server->nick_modes,
                      IRC_COLOR_BAR_DELIM);
        }
        else
        {
            snprintf (buf, length, "%s%s%s",
                      str_prefix,
                      IRC_COLOR_INPUT_NICK,
                      server->nick);
        }
    }

    return buf;
}

/*
 * Focus on nicklist.
 */

struct t_hashtable *
irc_bar_item_focus_buffer_nicklist (void *data,
                                    struct t_hashtable *info)
{
    long unsigned int value;
    int rc;
    struct t_gui_buffer *buffer;
    struct t_irc_nick *ptr_nick;
    const char *str_buffer, *nick;

    str_buffer = weechat_hashtable_get (info, "_buffer");
    if (!str_buffer || !str_buffer[0])
        return NULL;

    rc = sscanf (str_buffer, "%lx", &value);
    if ((rc == EOF) || (rc == 0))
        return NULL;

    buffer = (struct t_gui_buffer *)value;

    IRC_BUFFER_GET_SERVER_CHANNEL(buffer);

    /* make C compiler happy */
    (void) data;

    if (ptr_server && ptr_channel)
    {
        nick = weechat_hashtable_get (info, "nick");
        if (nick)
        {
            ptr_nick = irc_nick_search (ptr_server, ptr_channel, nick);
            if (ptr_nick && ptr_nick->host)
            {
                weechat_hashtable_set (info, "irc_host", ptr_nick->host);
                return info;
            }
        }
    }

    return NULL;
}

/*
 * Callback for signal "buffer_switch": refreshes irc bar items (for root bars).
 */

int
irc_bar_item_buffer_switch (void *data, const char *signal,
                            const char *type_data, void *signal_data)
{
    /* make C compiler happy */
    (void) data;
    (void) signal;
    (void) type_data;
    (void) signal_data;

    weechat_bar_item_update ("away");
    weechat_bar_item_update ("buffer_title");
    weechat_bar_item_update ("buffer_name");
    weechat_bar_item_update ("buffer_modes");
    weechat_bar_item_update ("irc_channel");
    weechat_bar_item_update ("lag");
    weechat_bar_item_update ("input_prompt");

    return WEECHAT_RC_OK;
}

/*
 * Initializes IRC bar items.
 */

void
irc_bar_item_init ()
{
    weechat_bar_item_new ("away", &irc_bar_item_away, NULL);
    weechat_bar_item_new ("buffer_title", &irc_bar_item_buffer_title, NULL);
    weechat_bar_item_new ("buffer_plugin", &irc_bar_item_buffer_plugin, NULL);
    weechat_bar_item_new ("buffer_name", &irc_bar_item_buffer_name, NULL);
    weechat_bar_item_new ("buffer_modes", &irc_bar_item_buffer_modes, NULL);
    weechat_bar_item_new ("irc_channel", &irc_bar_item_channel, NULL);
    weechat_bar_item_new ("lag", &irc_bar_item_lag, NULL);
    weechat_bar_item_new ("input_prompt", &irc_bar_item_input_prompt, NULL);
    weechat_hook_focus ("buffer_nicklist",
                        &irc_bar_item_focus_buffer_nicklist, NULL);
    weechat_hook_signal ("buffer_switch",
                         &irc_bar_item_buffer_switch, NULL);
}
