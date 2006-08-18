/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd is copyright (C) 2002-2006 ChatSpike-Dev.
 *                       E-mail:
 *                <brain@chatspike.net>
 *                <Craig@chatspike.net>
 *
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

using namespace std;

#include "inspircd_config.h"
#include "inspircd.h"
#include "inspircd_io.h"
#include <string>
#include <ext/hash_map>
#include "users.h"
#include "ctables.h"
#include "globals.h"
#include "modules.h"
#include "dynamic.h"
#include "message.h"
#include "commands.h"
#include "inspstring.h"
#include "helperfuncs.h"
#include "hashcomp.h"
#include "typedefs.h"
#include "cmd_list.h"
#include "wildcard.h"

extern chan_hash chanlist;

void cmd_list::Handle (char **parameters, int pcnt, userrec *user)
{
	WriteServ(user->fd,"321 %s Channel :Users Name",user->nick);
	for (chan_hash::const_iterator i = chanlist.begin(); i != chanlist.end(); i++)
	{
		// If the user gave us a glob pattern, attempt to match it
		if (pcnt && !match(i->second->name, parameters[0]))
				continue;
		// if the channel is not private/secret, OR the user is on the channel anyway
		bool n = i->second->HasUser(user);
		if (((!(i->second->modes[CM_PRIVATE])) && (!(i->second->modes[CM_SECRET]))) || (n))
		{
			long users = usercount(i->second);
			if (users)
				WriteServ(user->fd,"322 %s %s %d :[+%s] %s",user->nick,i->second->name,users,chanmodes(i->second,n),i->second->topic);
		}
	}
	WriteServ(user->fd,"323 %s :End of channel list.",user->nick);
}



