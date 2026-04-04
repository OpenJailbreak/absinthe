/**
 * GreenPois0n Absinthe - vpn_config.h
 * Copyright (C) 2012 Chronic-Dev Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef VPN_CONFIG_H
#define VPN_CONFIG_H

#include <plist/plist.h>

#define VPN_CONNECTION_NAME   "jailbreak"
#define VPN_TRIGGER_ADDRESS   "127.0.0.1"

/* Remove the jailbreak VPN on-demand entry from a preferences.plist, if present. */
void vpn_prefs_remove_entry(plist_t *pl);

/* Add a jailbreak VPN on-demand entry to a preferences.plist. */
void vpn_prefs_add_entry(plist_t *pl);

#endif /* VPN_CONFIG_H */
