/**
 * GreenPois0n Absinthe - vpn_config.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <plist/plist.h>

#include "vpn_config.h"

static int get_rand(int min, int max)
{
	return (rand() % (max - min)) + min;
}

static char *generate_guid(void)
{
	char *guid = (char *) malloc(sizeof(char) * 37);
	const char *chars = "ABCDEF0123456789";
	srand(time(NULL));
	int i = 0;

	for (i = 0; i < 36; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			guid[i] = '-';
			continue;
		} else {
			guid[i] = chars[get_rand(0, 16)];
		}
	}
	guid[36] = '\0';
	return guid;
}

void vpn_prefs_remove_entry(plist_t *pl)
{
	char *guid = NULL;
	plist_dict_iter iter = NULL;
	plist_t ns = plist_dict_get_item(*pl, "NetworkServices");
	if (!ns || (plist_get_node_type(ns) != PLIST_DICT)) {
		fprintf(stderr, "no NetworkServices node?!\n");
		return;
	}
	plist_dict_new_iter(ns, &iter);
	if (iter) {
		int found = 0;
		plist_t n = NULL;
		char *key = NULL;
		do {
			plist_dict_next_item(ns, iter, &key, &n);
			if (key && (plist_get_node_type(n) == PLIST_DICT)) {
				plist_t uname = plist_dict_get_item(n, "UserDefinedName");
				if (uname && (plist_get_node_type(uname) == PLIST_STRING)) {
					char *uname_str = NULL;
					plist_get_string_val(uname, &uname_str);
					if (uname_str) {
						if (strcmp(uname_str, VPN_CONNECTION_NAME) == 0) {
							/* entry found */
							found++;
							guid = strdup(key);
							printf(
								"removing /NetworkServices/%s (UserDefinedName: %s)\n",
								key, uname_str);
							plist_dict_remove_item(ns, guid);
						}
						free(uname_str);
					}
				}
			}
			if (key) {
				free(key);
				key = NULL;
			}
		} while (n && !found);
		free(iter);
	}

	if (!guid) {
		/* not found — nothing to do */
		fprintf(stderr, "no entry found\n");
		return;
	}

	plist_t sets = plist_dict_get_item(*pl, "Sets");
	if (!sets || (plist_get_node_type(sets) != PLIST_DICT)) {
		fprintf(stderr, "no Sets node?!\n");
		free(guid);
		return;
	}

	iter = NULL;
	plist_dict_new_iter(sets, &iter);
	if (iter) {
		plist_t n = NULL;
		char *key = NULL;
		do {
			plist_dict_next_item(sets, iter, &key, &n);
			if (key && (plist_get_node_type(n) == PLIST_DICT)) {
				plist_t nn = plist_access_path(n, 3, "Network", "Service",
						guid);
				if (nn) {
					nn = plist_access_path(n, 2, "Network", "Service");
					if (nn) {
						fprintf(stderr,
								"removing /Sets/%s/Network/Service/%s\n", key,
								guid);
						plist_dict_remove_item(nn, guid);
					}
				}
				nn = plist_access_path(n, 4, "Network", "Global", "IPv4",
						"ServiceOrder");
				if (nn && (plist_get_node_type(nn) == PLIST_ARRAY)) {
					int32_t num = (int32_t) plist_array_get_size(nn);
					int32_t x;
					for (x = num - 1; x >= 0; x--) {
						plist_t nnn = plist_array_get_item(nn, x);
						char *val = NULL;
						if (plist_get_node_type(nnn) == PLIST_KEY) {
							plist_get_key_val(nnn, &val);
						} else if (plist_get_node_type(nnn) == PLIST_STRING) {
							plist_get_string_val(nnn, &val);
						}
						if (val) {
							if (strcmp(val, guid) == 0) {
								free(val);
								fprintf(stderr,
										"removing /Sets/%s/Network/Global/IPv4/ServiceOrder/%s\n",
										key, guid);
								plist_array_remove_item(nn, x);
								break;
							}
							free(val);
						}
					}
				}
			}
			if (key) {
				free(key);
				key = NULL;
			}
		} while (n);
		free(iter);
	}
	free(guid);
}

void vpn_prefs_add_entry(plist_t *pl)
{
	plist_t dict = NULL;
	plist_t arr = NULL;

	/* construct connection data */
	plist_t conn = plist_new_dict();

	/* DNS */
	plist_dict_insert_item(conn, "DNS", plist_new_dict());

	/* UserDefinedName */
	plist_dict_insert_item(conn, "UserDefinedName",
			plist_new_string(VPN_CONNECTION_NAME));

	/* IPv4 */
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "ConfigMethod", plist_new_string("Automatic"));
	plist_dict_insert_item(dict, "OverridePrimary", plist_new_uint(1));
	plist_dict_insert_item(conn, "IPv4", dict);

	/* payload */
	plist_dict_insert_item(conn, "com.apple.payload", plist_new_dict());

	/* Interface */
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "Type", plist_new_string("IPSec"));
	plist_dict_insert_item(conn, "Interface", dict);

	/* Proxies */
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "FTPPassive", plist_new_uint(1));
	arr = plist_new_array();
	plist_array_append_item(arr, plist_new_string("*.local"));
	plist_array_append_item(arr, plist_new_string("169.254/16"));
	plist_dict_insert_item(dict, "ExceptionList", arr);
	plist_dict_insert_item(conn, "Proxies", dict);

	/* IPSec */
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "PromptForVPNPIN", plist_new_bool(0));
	plist_dict_insert_item(dict, "AuthenticationMethod",
			plist_new_string("SharedSecret"));
	plist_dict_insert_item(dict, "OnDemandSupported", plist_new_bool(1));
	plist_dict_insert_item(dict, "OnDemandEnabled", plist_new_uint(1));
	arr = plist_new_array();
	plist_array_append_item(arr, plist_new_string(VPN_TRIGGER_ADDRESS));
	plist_dict_insert_item(dict, "OnDemandMatchDomainsAlways", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsNever", arr);
	arr = plist_new_array();
	plist_dict_insert_item(dict, "OnDemandMatchDomainsOnRetry", arr);
	plist_dict_insert_item(dict, "RemoteAddress",
			plist_new_string("127.0.0.1"));
	plist_dict_insert_item(dict, "LocalIdentifier", plist_new_string("2"));
	plist_dict_insert_item(dict, "XAuthName",
			plist_new_string(
				"pod2g\"; proposal {authentication_method xauth_psk_client;"
				" hash_algorithm sha1; encryption_algorithm aes 256;"
				" lifetime time 3600 sec; dh_group 2;} } include"
				" \"/private/var/preferences/SystemConfiguration/"
				"com.apple.ipsec.plist"));
	plist_dict_insert_item(dict, "LocalIdentifierType",
			plist_new_string("KeyID"));
	plist_dict_insert_item(dict, "XAuthEnabled", plist_new_uint(1));
	plist_dict_insert_item(dict, "SharedSecretEncryption",
			plist_new_string("Key"));
	plist_dict_insert_item(dict, "SharedSecret", plist_new_string("pod2g"));
	plist_dict_insert_item(conn, "IPSec", dict);

	/* IPv6 */
	plist_dict_insert_item(conn, "IPv6", plist_new_dict());

	/* get NetworkServices node */
	plist_t ns = plist_dict_get_item(*pl, "NetworkServices");
	if (!ns || (plist_get_node_type(ns) != PLIST_DICT)) {
		fprintf(stderr, "ERROR: no NetworkServices node?!\n");
		plist_free(conn);
		return;
	}

	/* get current set */
	plist_t cset = plist_dict_get_item(*pl, "CurrentSet");
	if (!cset || (plist_get_node_type(cset) != PLIST_STRING)) {
		fprintf(stderr, "ERROR: no CurrentSet node found\n");
		plist_free(conn);
		return;
	}

	char *curset = NULL;
	plist_get_string_val(cset, &curset);
	if (!curset || (memcmp(curset, "/Sets/", 6) != 0)) {
		fprintf(stderr, "ERROR: CurrentSet has unexpected string value '%s'\n",
				(curset) ? curset : "(null)");
		if (curset) {
			free(curset);
		}
		plist_free(conn);
		return;
	}

	/* locate /Sets/{SetGUID}/Network/Service node */
	plist_t netsvc = plist_access_path(*pl, 4, "Sets", curset + 6, "Network",
			"Service");
	if (!netsvc) {
		fprintf(stderr,
				"ERROR: Could not access /Sets/%s/Network/Service node\n",
				curset + 6);
		free(curset);
		plist_free(conn);
		return;
	}

	/* locate /Sets/{SetGUID}/Network/Global/IPv4/ServiceOrder node */
	plist_t order = plist_access_path(*pl, 6, "Sets", curset + 6, "Network",
			"Global", "IPv4", "ServiceOrder");
	if (!order) {
		fprintf(stderr,
				"ERROR: Could not access /Sets/%s/Network/Global/IPv4/ServiceOrder node\n",
				curset + 6);
		free(curset);
		plist_free(conn);
		return;
	}
	free(curset);

	/* make sure we don't have a GUID collision */
	char *guid = generate_guid();
	while (plist_dict_get_item(ns, guid)) {
		free(guid);
		guid = generate_guid();
	}

	/* add connection to /NetworkServices/{GUID} */
	plist_dict_insert_item(ns, guid, conn);

	/* add {GUID} to /Sets/{SetGUID}/Network/Global/IPv4/ServiceOrder/ */
	plist_array_append_item(order, plist_new_string(guid));

	/* add {GUID} link dict to /Sets/{SetGUID}/Network/Service/ */
	char *linkstr = malloc(17 + strlen(guid) + 1);
	strcpy(linkstr, "/NetworkServices/");
	strcat(linkstr, guid);
	dict = plist_new_dict();
	plist_dict_insert_item(dict, "__LINK__", plist_new_string(linkstr));
	free(linkstr);
	plist_dict_insert_item(netsvc, guid, dict);

	free(guid);
}
