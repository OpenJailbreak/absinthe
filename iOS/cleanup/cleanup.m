#import <Foundation/Foundation.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#define CONNECTION_NAME "jailbreak"
#define WEBCLIP "/private/var/mobile/Library/WebClips/corona.webclip"

#define PREFS @"/private/var/preferences/SystemConfiguration/preferences.plist"

NSMutableDictionary* dict_access_by_path(NSMutableDictionary* dict, int length, ...)
{
	NSMutableDictionary* current = dict;
	va_list args;
	va_start(args, length);
	int i;
	for (i = 0; i < length && current; i++) {
		const char* key = va_arg(args, const char*);
        	current = [current objectForKey:[NSString stringWithUTF8String:key]];
	}
	va_end(args);
	return current;
}

void cleanup_preferences() /*{{{*/
{
	NSMutableDictionary* prefs = [NSMutableDictionary dictionaryWithContentsOfFile:PREFS];
	if (prefs) {
		NSLog(@"Cleaning up preferences.plist...");
		NSMutableDictionary* netsvc = [prefs objectForKey:@"NetworkServices"];
		NSString* guid = nil;
		NSArray* nskeys = (netsvc) ? [netsvc allKeys] : nil;
		if (nskeys) {
			for (NSString* nsk in nskeys) {
				NSMutableDictionary* nsc = [netsvc objectForKey:nsk];
				NSString* name = [nsc objectForKey:@"UserDefinedName"];
				if (name && [name isEqualToString:@""CONNECTION_NAME]) {
					NSLog(@"Removing /NetworkServices/%@ (UserDefinedName: %@)", nsk, name);
					[netsvc removeObjectForKey:nsk];
					guid = [NSString stringWithString:nsk];	
				}
			}
		} else {
			NSLog(@"no NetworkServices node?!");
		}
		if (guid) {
			NSMutableDictionary* sets = [prefs objectForKey:@"Sets"];
			NSArray* setsk = (sets) ? [sets allKeys] : nil;
			if (setsk) {
				for (NSString* setk in setsk) {
					NSMutableDictionary* set = [sets objectForKey:setk];
					NSMutableDictionary* netsvc = dict_access_by_path(set, 2, "Network", "Service");
					if (netsvc) {
						NSMutableDictionary* link = [netsvc objectForKey:guid];
						if (link) {
							NSLog(@"Removing /Sets/%@/Network/Services/%@", setk, guid);
							[netsvc removeObjectForKey:guid];
						}
					} else {
						NSLog(@"No /Sets/%@/Network/Services node?!", setk);
					}
					NSMutableDictionary* srvord = dict_access_by_path(set, 3, "Network", "Global", "IPv4");
					if (srvord) {
						NSMutableArray* srvorda = [srvord objectForKey:@"ServiceOrder"];
						int i = 0;
						for (NSString* ae in srvorda) {
							if ([ae isEqualToString:guid]) {
								NSLog(@"Removing /Sets/%@/Network/Global/IPv4/ServiceOrder/%@", setk, guid);
								[srvorda removeObjectAtIndex:i];
							}
							i++;
						}
					} else {
						NSLog(@"No /Sets/%@/Network/Global/IPv4/ServicesOrder node?!", setk);
					}
				}
			} else {
				NSLog(@"no Sets node?!");
			}
		}
		[prefs writeToFile:PREFS atomically:YES];
	} else {
		NSLog(@"Huh? Could not parse '%@'?!", PREFS);
	}
} /*}}}*/

int main(int argc, char** argv)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// remove VPN entry from preferences.plist
	cleanup_preferences();

	// remove WebClip
	NSLog(@"Removing WebClip");
	remove(WEBCLIP "/Info.plist");
	remove(WEBCLIP "/icon.png");
	remove(WEBCLIP);

	[pool release];
	return 0;
}
