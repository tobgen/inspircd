/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009-2010 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007, 2009 Dennis Friis <peavey@inspircd.org>
 *   Copyright (C) 2003-2008 Craig Edwards <craigedwards@brainbox.cc>
 *   Copyright (C) 2008 Thomas Stagner <aquanight@inspircd.org>
 *   Copyright (C) 2006-2007 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2006-2007 Oliver Lupton <oliverlupton@gmail.com>
 *   Copyright (C) 2007 Pippijn van Steenhoven <pip88nl@gmail.com>
 *   Copyright (C) 2003 randomdan <???@???>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include "inspircd.h"
#include "xline.h"
#include "socket.h"
#include "socketengine.h"
#include "command_parse.h"
#include "exitcodes.h"

#ifndef _WIN32
	#include <dirent.h>
#endif

static std::vector<dynamic_reference_base*>* dynrefs = NULL;
static bool dynref_init_complete = false;

void dynamic_reference_base::reset_all()
{
	dynref_init_complete = true;
	if (!dynrefs)
		return;
	for(unsigned int i = 0; i < dynrefs->size(); i++)
		(*dynrefs)[i]->resolve();
}

// Version is a simple class for holding a modules version number
Version::Version(const std::string &desc, int flags) : description(desc), Flags(flags)
{
}

Version::Version(const std::string &desc, int flags, const std::string& linkdata)
: description(desc), Flags(flags), link_data(linkdata)
{
}

Event::Event(Module* src, const std::string &eventid) : source(src), id(eventid) { }

void Event::Send()
{
	FOREACH_MOD(OnEvent, (*this));
}

// These declarations define the behavours of the base class Module (which does nothing at all)

Module::Module() { }
CullResult Module::cull()
{
	return classbase::cull();
}
Module::~Module()
{
}

void Module::DetachEvent(Implementation i)
{
	ServerInstance->Modules->Detach(i, this);
}

void		Module::ReadConfig(ConfigStatus& status) { }
ModResult	Module::OnSendSnotice(char &snomask, std::string &type, const std::string &message) { DetachEvent(I_OnSendSnotice); return MOD_RES_PASSTHRU; }
void		Module::OnUserConnect(LocalUser*) { DetachEvent(I_OnUserConnect); }
void		Module::OnUserQuit(User*, const std::string&, const std::string&) { DetachEvent(I_OnUserQuit); }
void		Module::OnUserDisconnect(LocalUser*) { DetachEvent(I_OnUserDisconnect); }
void		Module::OnUserJoin(Membership*, bool, bool, CUList&) { DetachEvent(I_OnUserJoin); }
void		Module::OnPostJoin(Membership*) { DetachEvent(I_OnPostJoin); }
void		Module::OnUserPart(Membership*, std::string&, CUList&) { DetachEvent(I_OnUserPart); }
void		Module::OnPreRehash(User*, const std::string&) { DetachEvent(I_OnPreRehash); }
void		Module::OnModuleRehash(User*, const std::string&) { DetachEvent(I_OnModuleRehash); }
ModResult	Module::OnUserPreJoin(LocalUser*, Channel*, const std::string&, std::string&, const std::string&) { DetachEvent(I_OnUserPreJoin); return MOD_RES_PASSTHRU; }
void		Module::OnMode(User*, User*, Channel*, const std::vector<std::string>&, const std::vector<TranslateType>&) { DetachEvent(I_OnMode); }
void		Module::OnOper(User*, const std::string&) { DetachEvent(I_OnOper); }
void		Module::OnPostOper(User*, const std::string&, const std::string &) { DetachEvent(I_OnPostOper); }
void		Module::OnInfo(User*) { DetachEvent(I_OnInfo); }
void		Module::OnWhois(User*, User*) { DetachEvent(I_OnWhois); }
ModResult	Module::OnUserPreInvite(User*, User*, Channel*, time_t) { DetachEvent(I_OnUserPreInvite); return MOD_RES_PASSTHRU; }
ModResult	Module::OnUserPreMessage(User*, void*, int, std::string&, char, CUList&, MessageType) { DetachEvent(I_OnUserPreMessage); return MOD_RES_PASSTHRU; }
ModResult	Module::OnUserPreNick(User*, const std::string&) { DetachEvent(I_OnUserPreNick); return MOD_RES_PASSTHRU; }
void		Module::OnUserPostNick(User*, const std::string&) { DetachEvent(I_OnUserPostNick); }
ModResult	Module::OnPreMode(User*, User*, Channel*, const std::vector<std::string>&) { DetachEvent(I_OnPreMode); return MOD_RES_PASSTHRU; }
void		Module::On005Numeric(std::map<std::string, std::string>&) { DetachEvent(I_On005Numeric); }
ModResult	Module::OnKill(User*, User*, const std::string&) { DetachEvent(I_OnKill); return MOD_RES_PASSTHRU; }
void		Module::OnLoadModule(Module*) { DetachEvent(I_OnLoadModule); }
void		Module::OnUnloadModule(Module*) { DetachEvent(I_OnUnloadModule); }
void		Module::OnBackgroundTimer(time_t) { DetachEvent(I_OnBackgroundTimer); }
ModResult	Module::OnPreCommand(std::string&, std::vector<std::string>&, LocalUser*, bool, const std::string&) { DetachEvent(I_OnPreCommand); return MOD_RES_PASSTHRU; }
void		Module::OnPostCommand(Command*, const std::vector<std::string>&, LocalUser*, CmdResult, const std::string&) { DetachEvent(I_OnPostCommand); }
void		Module::OnUserInit(LocalUser*) { DetachEvent(I_OnUserInit); }
ModResult	Module::OnCheckReady(LocalUser*) { DetachEvent(I_OnCheckReady); return MOD_RES_PASSTHRU; }
ModResult	Module::OnUserRegister(LocalUser*) { DetachEvent(I_OnUserRegister); return MOD_RES_PASSTHRU; }
ModResult	Module::OnUserPreKick(User*, Membership*, const std::string&) { DetachEvent(I_OnUserPreKick); return MOD_RES_PASSTHRU; }
void		Module::OnUserKick(User*, Membership*, const std::string&, CUList&) { DetachEvent(I_OnUserKick); }
ModResult	Module::OnRawMode(User*, Channel*, const char, const std::string &, bool, int) { DetachEvent(I_OnRawMode); return MOD_RES_PASSTHRU; }
ModResult	Module::OnCheckInvite(User*, Channel*) { DetachEvent(I_OnCheckInvite); return MOD_RES_PASSTHRU; }
ModResult	Module::OnCheckKey(User*, Channel*, const std::string&) { DetachEvent(I_OnCheckKey); return MOD_RES_PASSTHRU; }
ModResult	Module::OnCheckLimit(User*, Channel*) { DetachEvent(I_OnCheckLimit); return MOD_RES_PASSTHRU; }
ModResult	Module::OnCheckChannelBan(User*, Channel*) { DetachEvent(I_OnCheckChannelBan); return MOD_RES_PASSTHRU; }
ModResult	Module::OnCheckBan(User*, Channel*, const std::string&) { DetachEvent(I_OnCheckBan); return MOD_RES_PASSTHRU; }
ModResult	Module::OnExtBanCheck(User*, Channel*, char) { DetachEvent(I_OnExtBanCheck); return MOD_RES_PASSTHRU; }
ModResult	Module::OnStats(char, User*, string_list&) { DetachEvent(I_OnStats); return MOD_RES_PASSTHRU; }
ModResult	Module::OnChangeLocalUserHost(LocalUser*, const std::string&) { DetachEvent(I_OnChangeLocalUserHost); return MOD_RES_PASSTHRU; }
ModResult	Module::OnChangeLocalUserGECOS(LocalUser*, const std::string&) { DetachEvent(I_OnChangeLocalUserGECOS); return MOD_RES_PASSTHRU; }
ModResult	Module::OnPreTopicChange(User*, Channel*, const std::string&) { DetachEvent(I_OnPreTopicChange); return MOD_RES_PASSTHRU; }
void		Module::OnEvent(Event&) { DetachEvent(I_OnEvent); }
ModResult	Module::OnPassCompare(Extensible* ex, const std::string &password, const std::string &input, const std::string& hashtype) { DetachEvent(I_OnPassCompare); return MOD_RES_PASSTHRU; }
void		Module::OnGlobalOper(User*) { DetachEvent(I_OnGlobalOper); }
void		Module::OnPostConnect(User*) { DetachEvent(I_OnPostConnect); }
void		Module::OnUserMessage(User*, void*, int, const std::string&, char, const CUList&, MessageType) { DetachEvent(I_OnUserMessage); }
void		Module::OnUserInvite(User*, User*, Channel*, time_t) { DetachEvent(I_OnUserInvite); }
void		Module::OnPostTopicChange(User*, Channel*, const std::string&) { DetachEvent(I_OnPostTopicChange); }
void		Module::OnSyncUser(User*, ProtocolInterface::Server&) { DetachEvent(I_OnSyncUser); }
void		Module::OnSyncChannel(Channel*, ProtocolInterface::Server&) { DetachEvent(I_OnSyncChannel); }
void		Module::OnSyncNetwork(ProtocolInterface::Server&) { DetachEvent(I_OnSyncNetwork); }
void		Module::OnDecodeMetaData(Extensible*, const std::string&, const std::string&) { DetachEvent(I_OnDecodeMetaData); }
void		Module::OnChangeHost(User*, const std::string&) { DetachEvent(I_OnChangeHost); }
void		Module::OnChangeName(User*, const std::string&) { DetachEvent(I_OnChangeName); }
void		Module::OnChangeIdent(User*, const std::string&) { DetachEvent(I_OnChangeIdent); }
void		Module::OnAddLine(User*, XLine*) { DetachEvent(I_OnAddLine); }
void		Module::OnDelLine(User*, XLine*) { DetachEvent(I_OnDelLine); }
void		Module::OnExpireLine(XLine*) { DetachEvent(I_OnExpireLine); }
void 		Module::OnCleanup(int, void*) { }
ModResult	Module::OnChannelPreDelete(Channel*) { DetachEvent(I_OnChannelPreDelete); return MOD_RES_PASSTHRU; }
void		Module::OnChannelDelete(Channel*) { DetachEvent(I_OnChannelDelete); }
ModResult	Module::OnSetAway(User*, const std::string &) { DetachEvent(I_OnSetAway); return MOD_RES_PASSTHRU; }
ModResult	Module::OnWhoisLine(User*, User*, int&, std::string&) { DetachEvent(I_OnWhoisLine); return MOD_RES_PASSTHRU; }
void		Module::OnBuildNeighborList(User*, IncludeChanList&, std::map<User*,bool>&) { DetachEvent(I_OnBuildNeighborList); }
void		Module::OnGarbageCollect() { DetachEvent(I_OnGarbageCollect); }
ModResult	Module::OnSetConnectClass(LocalUser* user, ConnectClass* myclass) { DetachEvent(I_OnSetConnectClass); return MOD_RES_PASSTHRU; }
void 		Module::OnText(User*, void*, int, const std::string&, char, CUList&) { DetachEvent(I_OnText); }
void		Module::OnRunTestSuite() { DetachEvent(I_OnRunTestSuite); }
void		Module::OnNamesListItem(User*, Membership*, std::string&, std::string&) { DetachEvent(I_OnNamesListItem); }
ModResult	Module::OnNumeric(User*, unsigned int, const std::string&) { DetachEvent(I_OnNumeric); return MOD_RES_PASSTHRU; }
ModResult   Module::OnAcceptConnection(int, ListenSocket*, irc::sockets::sockaddrs*, irc::sockets::sockaddrs*) { DetachEvent(I_OnAcceptConnection); return MOD_RES_PASSTHRU; }
void		Module::OnSendWhoLine(User*, const std::vector<std::string>&, User*, Channel*, std::string&) { DetachEvent(I_OnSendWhoLine); }
void		Module::OnSetUserIP(LocalUser*) { DetachEvent(I_OnSetUserIP); }

ServiceProvider::ServiceProvider(Module* Creator, const std::string& Name, ServiceType Type)
	: creator(Creator), name(Name), service(Type)
{
	if ((ServerInstance) && (ServerInstance->Modules->NewServices))
		ServerInstance->Modules->NewServices->push_back(this);
}

void ServiceProvider::DisableAutoRegister()
{
	if ((ServerInstance) && (ServerInstance->Modules->NewServices))
	{
		ModuleManager::ServiceList& list = *ServerInstance->Modules->NewServices;
		ModuleManager::ServiceList::iterator it = std::find(list.begin(), list.end(), this);
		if (it != list.end())
			list.erase(it);
	}
}

ModuleManager::ModuleManager()
{
}

ModuleManager::~ModuleManager()
{
}

bool ModuleManager::Attach(Implementation i, Module* mod)
{
	if (std::find(EventHandlers[i].begin(), EventHandlers[i].end(), mod) != EventHandlers[i].end())
		return false;

	EventHandlers[i].push_back(mod);
	return true;
}

bool ModuleManager::Detach(Implementation i, Module* mod)
{
	EventHandlerIter x = std::find(EventHandlers[i].begin(), EventHandlers[i].end(), mod);

	if (x == EventHandlers[i].end())
		return false;

	EventHandlers[i].erase(x);
	return true;
}

void ModuleManager::Attach(Implementation* i, Module* mod, size_t sz)
{
	for (size_t n = 0; n < sz; ++n)
		Attach(i[n], mod);
}

void ModuleManager::AttachAll(Module* mod)
{
	for (size_t i = I_BEGIN + 1; i != I_END; ++i)
		Attach((Implementation)i, mod);
}

void ModuleManager::DetachAll(Module* mod)
{
	for (size_t n = I_BEGIN + 1; n != I_END; ++n)
		Detach((Implementation)n, mod);
}

bool ModuleManager::SetPriority(Module* mod, Priority s)
{
	for (size_t n = I_BEGIN + 1; n != I_END; ++n)
		SetPriority(mod, (Implementation)n, s);

	return true;
}

bool ModuleManager::SetPriority(Module* mod, Implementation i, Priority s, Module* which)
{
	/** To change the priority of a module, we first find its position in the vector,
	 * then we find the position of the other modules in the vector that this module
	 * wants to be before/after. We pick off either the first or last of these depending
	 * on which they want, and we make sure our module is *at least* before or after
	 * the first or last of this subset, depending again on the type of priority.
	 */
	size_t my_pos = 0;

	/* Locate our module. This is O(n) but it only occurs on module load so we're
	 * not too bothered about it
	 */
	for (size_t x = 0; x != EventHandlers[i].size(); ++x)
	{
		if (EventHandlers[i][x] == mod)
		{
			my_pos = x;
			goto found_src;
		}
	}

	/* Eh? this module doesnt exist, probably trying to set priority on an event
	 * theyre not attached to.
	 */
	return false;

found_src:
	// The modules registered for a hook are called in reverse order (to allow for easier removal
	// of list entries while looping), meaning that the Priority given to us has the exact opposite effect
	// on the list, e.g.: PRIORITY_BEFORE will actually put 'mod' after 'which', etc.
	size_t swap_pos = my_pos;
	switch (s)
	{
		case PRIORITY_LAST:
			if (prioritizationState != PRIO_STATE_FIRST)
				return true;
			else
				swap_pos = 0;
			break;
		case PRIORITY_FIRST:
			if (prioritizationState != PRIO_STATE_FIRST)
				return true;
			else
				swap_pos = EventHandlers[i].size() - 1;
			break;
		case PRIORITY_BEFORE:
		{
			/* Find the latest possible position, only searching AFTER our position */
			for (size_t x = EventHandlers[i].size() - 1; x > my_pos; --x)
			{
				if (EventHandlers[i][x] == which)
				{
					swap_pos = x;
					goto swap_now;
				}
			}
			// didn't find it - either not loaded or we're already after
			return true;
		}
		/* Place this module before a set of other modules */
		case PRIORITY_AFTER:
		{
			for (size_t x = 0; x < my_pos; ++x)
			{
				if (EventHandlers[i][x] == which)
				{
					swap_pos = x;
					goto swap_now;
				}
			}
			// didn't find it - either not loaded or we're already before
			return true;
		}
	}

swap_now:
	/* Do we need to swap? */
	if (swap_pos != my_pos)
	{
		// We are going to change positions; we'll need to run again to verify all requirements
		if (prioritizationState == PRIO_STATE_LAST)
			prioritizationState = PRIO_STATE_AGAIN;
		/* Suggestion from Phoenix, "shuffle" the modules to better retain call order */
		int incrmnt = 1;

		if (my_pos > swap_pos)
			incrmnt = -1;

		for (unsigned int j = my_pos; j != swap_pos; j += incrmnt)
		{
			if ((j + incrmnt > EventHandlers[i].size() - 1) || ((incrmnt == -1) && (j == 0)))
				continue;

			std::swap(EventHandlers[i][j], EventHandlers[i][j+incrmnt]);
		}
	}

	return true;
}

bool ModuleManager::PrioritizeHooks()
{
	/* We give every module a chance to re-prioritize when we introduce a new one,
	 * not just the one thats loading, as the new module could affect the preference
	 * of others
	 */
	for (int tries = 0; tries < 20; tries++)
	{
		prioritizationState = tries > 0 ? PRIO_STATE_LAST : PRIO_STATE_FIRST;
		for (std::map<std::string, Module*>::iterator n = Modules.begin(); n != Modules.end(); ++n)
			n->second->Prioritize();

		if (prioritizationState == PRIO_STATE_LAST)
			break;
		if (tries == 19)
		{
			ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, "Hook priority dependency loop detected");
			return false;
		}
	}
	return true;
}

bool ModuleManager::CanUnload(Module* mod)
{
	std::map<std::string, Module*>::iterator modfind = Modules.find(mod->ModuleSourceFile);

	if ((modfind == Modules.end()) || (modfind->second != mod) || (mod->dying))
	{
		LastModuleError = "Module " + mod->ModuleSourceFile + " is not loaded, cannot unload it!";
		ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, LastModuleError);
		return false;
	}
	if (mod->GetVersion().Flags & VF_STATIC)
	{
		LastModuleError = "Module " + mod->ModuleSourceFile + " not unloadable (marked static)";
		ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, LastModuleError);
		return false;
	}

	mod->dying = true;
	return true;
}

void ModuleManager::DoSafeUnload(Module* mod)
{
	// First, notify all modules that a module is about to be unloaded, so in case
	// they pass execution to the soon to be unloaded module, it will happen now,
	// i.e. before we unregister the services of the module being unloaded
	FOREACH_MOD(OnUnloadModule, (mod));

	std::map<std::string, Module*>::iterator modfind = Modules.find(mod->ModuleSourceFile);

	std::vector<reference<ExtensionItem> > items;
	ServerInstance->Extensions.BeginUnregister(modfind->second, items);
	/* Give the module a chance to tidy out all its metadata */
	for (chan_hash::iterator c = ServerInstance->chanlist->begin(); c != ServerInstance->chanlist->end(); )
	{
		Channel* chan = c->second;
		++c;
		mod->OnCleanup(TYPE_CHANNEL, chan);
		chan->doUnhookExtensions(items);
		const UserMembList* users = chan->GetUsers();
		for(UserMembCIter mi = users->begin(); mi != users->end(); mi++)
			mi->second->doUnhookExtensions(items);
	}
	for (user_hash::iterator u = ServerInstance->Users->clientlist->begin(); u != ServerInstance->Users->clientlist->end(); )
	{
		User* user = u->second;
		// The module may quit the user (e.g. SSL mod unloading) and that will remove it from the container
		++u;
		mod->OnCleanup(TYPE_USER, user);
		user->doUnhookExtensions(items);
	}
	for(char m='A'; m <= 'z'; m++)
	{
		ModeHandler* mh;
		mh = ServerInstance->Modes->FindMode(m, MODETYPE_USER);
		if (mh && mh->creator == mod)
			this->DelService(*mh);
		mh = ServerInstance->Modes->FindMode(m, MODETYPE_CHANNEL);
		if (mh && mh->creator == mod)
			this->DelService(*mh);
	}
	for(std::multimap<std::string, ServiceProvider*>::iterator i = DataProviders.begin(); i != DataProviders.end(); )
	{
		std::multimap<std::string, ServiceProvider*>::iterator curr = i++;
		if (curr->second->creator == mod)
			DataProviders.erase(curr);
	}

	dynamic_reference_base::reset_all();

	DetachAll(mod);

	Modules.erase(modfind);
	ServerInstance->GlobalCulls.AddItem(mod);

	ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, "Module %s unloaded",mod->ModuleSourceFile.c_str());
	ServerInstance->ISupport.Build();
}

void ModuleManager::UnloadAll()
{
	/* We do this more than once, so that any service providers get a
	 * chance to be unhooked by the modules using them, but then get
	 * a chance to be removed themsleves.
	 *
	 * Note: this deliberately does NOT delete the DLLManager objects
	 */
	for (int tries = 0; tries < 4; tries++)
	{
		std::map<std::string, Module*>::iterator i = Modules.begin();
		while (i != Modules.end())
		{
			std::map<std::string, Module*>::iterator me = i++;
			if (CanUnload(me->second))
			{
				DoSafeUnload(me->second);
			}
		}
		ServerInstance->GlobalCulls.Apply();
	}
}

namespace
{
	struct UnloadAction : public HandlerBase0<void>
	{
		Module* const mod;
		UnloadAction(Module* m) : mod(m) {}
		void Call()
		{
			DLLManager* dll = mod->ModuleDLLManager;
			ServerInstance->Modules->DoSafeUnload(mod);
			ServerInstance->GlobalCulls.Apply();
			// In pure static mode this is always NULL
			delete dll;
			ServerInstance->GlobalCulls.AddItem(this);
		}
	};

	struct ReloadAction : public HandlerBase0<void>
	{
		Module* const mod;
		HandlerBase1<void, bool>* const callback;
		ReloadAction(Module* m, HandlerBase1<void, bool>* c)
			: mod(m), callback(c) {}
		void Call()
		{
			DLLManager* dll = mod->ModuleDLLManager;
			std::string name = mod->ModuleSourceFile;
			ServerInstance->Modules->DoSafeUnload(mod);
			ServerInstance->GlobalCulls.Apply();
			delete dll;
			bool rv = ServerInstance->Modules->Load(name);
			if (callback)
				callback->Call(rv);
			ServerInstance->GlobalCulls.AddItem(this);
		}
	};
}

bool ModuleManager::Unload(Module* mod)
{
	if (!CanUnload(mod))
		return false;
	ServerInstance->AtomicActions.AddAction(new UnloadAction(mod));
	return true;
}

void ModuleManager::Reload(Module* mod, HandlerBase1<void, bool>* callback)
{
	if (CanUnload(mod))
		ServerInstance->AtomicActions.AddAction(new ReloadAction(mod, callback));
	else
		callback->Call(false);
}

void ModuleManager::LoadAll()
{
	std::map<std::string, ServiceList> servicemap;
	LoadCoreModules(servicemap);

	ConfigTagList tags = ServerInstance->Config->ConfTags("module");
	for (ConfigIter i = tags.first; i != tags.second; ++i)
	{
		ConfigTag* tag = i->second;
		std::string name = tag->getString("name");
		this->NewServices = &servicemap[name];
		std::cout << "[" << con_green << "*" << con_reset << "] Loading module:\t" << con_green << name << con_reset << std::endl;

		if (!this->Load(name, true))
		{
			ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, this->LastError());
			std::cout << std::endl << "[" << con_red << "*" << con_reset << "] " << this->LastError() << std::endl << std::endl;
			ServerInstance->Exit(EXIT_STATUS_MODULE);
		}
	}

	ConfigStatus confstatus;

	for (ModuleMap::const_iterator i = Modules.begin(); i != Modules.end(); ++i)
	{
		Module* mod = i->second;
		try
		{
			ServerInstance->Logs->Log("MODULE", LOG_DEBUG, "Initializing %s", i->first.c_str());
			AttachAll(mod);
			AddServices(servicemap[i->first]);
			mod->init();
			mod->ReadConfig(confstatus);
		}
		catch (CoreException& modexcept)
		{
			LastModuleError = "Unable to initialize " + mod->ModuleSourceFile + ": " + modexcept.GetReason();
			ServerInstance->Logs->Log("MODULE", LOG_DEFAULT, LastModuleError);
			std::cout << std::endl << "[" << con_red << "*" << con_reset << "] " << LastModuleError << std::endl << std::endl;
			ServerInstance->Exit(EXIT_STATUS_MODULE);
		}
	}

	this->NewServices = NULL;

	if (!PrioritizeHooks())
		ServerInstance->Exit(EXIT_STATUS_MODULE);
}

std::string& ModuleManager::LastError()
{
	return LastModuleError;
}

void ModuleManager::AddServices(const ServiceList& list)
{
	for (ServiceList::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		ServiceProvider& s = **i;
		AddService(s);
	}
}

void ModuleManager::AddService(ServiceProvider& item)
{
	switch (item.service)
	{
		case SERVICE_COMMAND:
			if (!ServerInstance->Parser->AddCommand(static_cast<Command*>(&item)))
				throw ModuleException("Command "+std::string(item.name)+" already exists.");
			return;
		case SERVICE_MODE:
		{
			ModeHandler* mh = static_cast<ModeHandler*>(&item);
			if (!ServerInstance->Modes->AddMode(mh))
				throw ModuleException("Mode "+std::string(item.name)+" already exists.");
			DataProviders.insert(std::make_pair((mh->GetModeType() == MODETYPE_CHANNEL ? "mode/" : "umode/") + item.name, &item));
			dynamic_reference_base::reset_all();
			return;
		}
		case SERVICE_METADATA:
			if (!ServerInstance->Extensions.Register(static_cast<ExtensionItem*>(&item)))
				throw ModuleException("Extension " + std::string(item.name) + " already exists.");
			return;
		case SERVICE_DATA:
		case SERVICE_IOHOOK:
		{
			if ((item.name.substr(0, 5) == "mode/") || (item.name.substr(0, 6) == "umode/"))
				throw ModuleException("The \"mode/\" and the \"umode\" service name prefixes are reserved.");

			DataProviders.insert(std::make_pair(item.name, &item));
			std::string::size_type slash = item.name.find('/');
			if (slash != std::string::npos)
			{
				DataProviders.insert(std::make_pair(item.name.substr(0, slash), &item));
				DataProviders.insert(std::make_pair(item.name.substr(slash + 1), &item));
			}
			dynamic_reference_base::reset_all();
			return;
		}
		default:
			throw ModuleException("Cannot add unknown service type");
	}
}

void ModuleManager::DelService(ServiceProvider& item)
{
	switch (item.service)
	{
		case SERVICE_MODE:
			if (!ServerInstance->Modes->DelMode(static_cast<ModeHandler*>(&item)))
				throw ModuleException("Mode "+std::string(item.name)+" does not exist.");
			// Fall through
		case SERVICE_DATA:
		case SERVICE_IOHOOK:
		{
			for(std::multimap<std::string, ServiceProvider*>::iterator i = DataProviders.begin(); i != DataProviders.end(); )
			{
				std::multimap<std::string, ServiceProvider*>::iterator curr = i++;
				if (curr->second == &item)
					DataProviders.erase(curr);
			}
			dynamic_reference_base::reset_all();
			return;
		}
		default:
			throw ModuleException("Cannot delete unknown service type");
	}
}

ServiceProvider* ModuleManager::FindService(ServiceType type, const std::string& name)
{
	switch (type)
	{
		case SERVICE_DATA:
		case SERVICE_IOHOOK:
		{
			std::multimap<std::string, ServiceProvider*>::iterator i = DataProviders.find(name);
			if (i != DataProviders.end() && i->second->service == type)
				return i->second;
			return NULL;
		}
		// TODO implement finding of the other types
		default:
			throw ModuleException("Cannot find unknown service type");
	}
}

dynamic_reference_base::dynamic_reference_base(Module* Creator, const std::string& Name)
	: name(Name), value(NULL), creator(Creator)
{
	if (!dynrefs)
		dynrefs = new std::vector<dynamic_reference_base*>;
	dynrefs->push_back(this);
	if (dynref_init_complete)
		resolve();
}

dynamic_reference_base::~dynamic_reference_base()
{
	for(unsigned int i = 0; i < dynrefs->size(); i++)
	{
		if (dynrefs->at(i) == this)
		{
			unsigned int last = dynrefs->size() - 1;
			if (i != last)
				dynrefs->at(i) = dynrefs->at(last);
			dynrefs->erase(dynrefs->begin() + last);
			if (dynrefs->empty())
			{
				delete dynrefs;
				dynrefs = NULL;
			}
			return;
		}
	}
}

void dynamic_reference_base::SetProvider(const std::string& newname)
{
	name = newname;
	resolve();
}

void dynamic_reference_base::resolve()
{
	std::multimap<std::string, ServiceProvider*>::iterator i = ServerInstance->Modules->DataProviders.find(name);
	if (i != ServerInstance->Modules->DataProviders.end())
		value = static_cast<DataProvider*>(i->second);
	else
		value = NULL;
}

Module* ModuleManager::Find(const std::string &name)
{
	std::map<std::string, Module*>::iterator modfind = Modules.find(name);

	if (modfind == Modules.end())
		return NULL;
	else
		return modfind->second;
}
