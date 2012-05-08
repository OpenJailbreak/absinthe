#include <stdio.h>
#import <Cocoa/Cocoa.h>
#include "AbsintheMainWnd.h"

static AbsintheMainWnd* __this = NULL;

@interface theAppDelegate : NSObject
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
 <NSApplicationDelegate, NSWindowDelegate>
#endif
@end

static void setAttributedTitleColor(id view, NSColor* color)
{
	NSMutableAttributedString *attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[view attributedTitle]];
	int len = [attrTitle length];
	NSRange range = NSMakeRange(0, len);
	[attrTitle addAttribute:NSForegroundColorAttributeName value:color range:range];
	[attrTitle fixAttributesInRange:range];
	[view setAttributedTitle:attrTitle];
	[attrTitle release];
}

@implementation theAppDelegate
- (void)start_clicked:(id)sender
{
	__this->handleStartClicked(NULL);
}

- (void)paypal_clicked:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:PAYPAL_LINK_URL]]];
}

- (void)gp_clicked:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:GP_LINK_URL]]];
}

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification
{
	NSWindow* mainwnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, WND_WIDTH, WND_HEIGHT) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask backing:NSBackingStoreBuffered defer:TRUE];
	[mainwnd setDelegate:self];

	NSTextField* lbTop = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 270, WND_WIDTH-20, 70)];
	[lbTop setEditable:NO];
	[lbTop setSelectable:YES];
	[lbTop setStringValue:[NSString stringWithUTF8String:INFO_LABEL_TEXT]];
	[lbTop setBezeled:NO];
	[lbTop setDrawsBackground:NO];

	NSTextField* lbStatus = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 176, WND_WIDTH-20, 80)];
	[lbStatus setEditable:NO];
	[lbStatus setSelectable:YES];
	[lbStatus setBezeled:NO];
	[lbStatus setDrawsBackground:NO];
	__this->lbStatus = lbStatus;

	NSProgressIndicator* progressBar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(30, 144, 300, 17)];
	[progressBar setIndeterminate:NO];
	[progressBar startAnimation:self];
	[progressBar setMinValue:0];
	[progressBar setMaxValue:100];
	__this->progressBar = progressBar;

	NSButton* btnStart = [[NSButton alloc] initWithFrame:NSMakeRect(350, 140, 100, 24)];
	[btnStart setButtonType:NSMomentaryPushInButton];
	[btnStart setBezelStyle:NSRoundedBezelStyle];
	//[btnStart setEnabled:NO];
	[btnStart setTitle:[NSString stringWithUTF8String:BTN_START_TEXT]];
	[btnStart setTarget:self];
	[btnStart setAction:@selector(start_clicked:)];
	__this->btnStart = btnStart;

	NSTextField* lbCredits = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 40, WND_WIDTH-20, 90)];
	[lbCredits setEditable:NO];
	[lbCredits setSelectable:YES];
	[lbCredits setStringValue:[NSString stringWithUTF8String:CREDITS_LABEL_TEXT]];
	[lbCredits setBezeled:NO];
	[lbCredits setDrawsBackground:NO];
	[lbCredits setAlignment:NSCenterTextAlignment];

	NSButton* lbPaypal = [[NSButton alloc] initWithFrame:NSMakeRect(40, 10, 160, 20)];
	[lbPaypal setTitle:[NSString stringWithUTF8String:PAYPAL_LINK_TEXT]];
	[lbPaypal setButtonType:NSMomentaryPushInButton];
	[lbPaypal setBordered:NO];
	[lbPaypal setTransparent:NO];
	[lbPaypal setBezelStyle:NSRecessedBezelStyle];
	setAttributedTitleColor(lbPaypal, [NSColor blueColor]);
	[lbPaypal setTarget:self];
	[lbPaypal setAction:@selector(paypal_clicked:)];

	NSButton* lbGP = [[NSButton alloc] initWithFrame:NSMakeRect(270, 10, 180, 20)];
	[lbGP setTitle:[NSString stringWithUTF8String:GP_LINK_TEXT]];
	[lbGP setButtonType:NSMomentaryPushInButton];
	[lbGP setBordered:NO];
	[lbGP setTransparent:NO];
	[lbGP setBezelStyle:NSRecessedBezelStyle];
	setAttributedTitleColor(lbGP, [NSColor blueColor]);
	[lbGP setTarget:self];
	[lbGP setAction:@selector(gp_clicked:)];

	[mainwnd setTitle:[NSString stringWithUTF8String:WND_TITLE]];
	[[mainwnd contentView] addSubview:lbTop];
	[[mainwnd contentView] addSubview:lbStatus];
	[[mainwnd contentView] addSubview:progressBar];
	[[mainwnd contentView] addSubview:btnStart];
	[[mainwnd contentView] addSubview:lbCredits];
	[[mainwnd contentView] addSubview:lbPaypal];
	[[mainwnd contentView] addSubview:lbGP];
	__this->mainwnd = mainwnd;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[__this->mainwnd makeKeyAndOrderFront:nil];
	[__this->mainwnd setLevel:NSFloatingWindowLevel];
	__this->worker = new AbsintheWorker(__this);
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *) theApplication
{
	return YES;
}

- (BOOL) windowShouldClose:(id)sender
{
	if (__this->mainwnd == sender) {
		return !__this->onClose(NULL);
	}
	return YES;
}
@end

int AbsintheMainWnd::msgBox(const char* message, const char* caption, int style)
{
	int res = 0;

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	NSAlert* alert = [[NSAlert alloc] init];

	// get message type
	if (style & mb_ICON_INFO) {
		[alert setAlertStyle:NSInformationalAlertStyle];
	} else if (style & mb_ICON_WARNING) {
		[alert setAlertStyle:NSWarningAlertStyle];
	} else if (style & mb_ICON_QUESTION) {
		[alert setAlertStyle:NSInformationalAlertStyle];
	} else if (style & mb_ICON_ERROR) {
		[alert setAlertStyle:NSCriticalAlertStyle];
	}

	// add buttons
	NSButton* btn;
	if (style & mb_OK) {
		btn = [alert addButtonWithTitle:@"OK"];
		[btn setTag:mb_OK];
	} else if (style & mb_CANCEL) {
		btn = [alert addButtonWithTitle:@"Cancel"];
		[btn setTag:mb_CANCEL];
	} else if (style & mb_OK_CANCEL) {
		btn = [alert addButtonWithTitle:@"OK"];
		[btn setTag:mb_OK];
		btn = [alert addButtonWithTitle:@"Cancel"];
		[btn setTag:mb_CANCEL];
	} else if (style & mb_YES_NO) {
		btn = [alert addButtonWithTitle:@"Yes"];
		[btn setTag:mb_YES];
		btn = [alert addButtonWithTitle:@"No"];
		[btn setTag:mb_NO];
	}

	[alert setMessageText:[NSString stringWithUTF8String:caption]];
	[alert setInformativeText:[NSString stringWithUTF8String:message]];

	res = [alert runModal];
	[alert release];

	[pool release];

	return res;
}

void AbsintheMainWnd::setButtonEnabled(int enabled)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[btnStart setEnabled:enabled];
	[pool release];
}

void AbsintheMainWnd::setStatusText(const char* text)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[this->lbStatus setStringValue:[NSString stringWithUTF8String:text]];
	[pool release];
}

void AbsintheMainWnd::setProgress(int percentage)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	if ((percentage == 0) || (percentage > 99)) {
		[this->progressBar stopAnimation:nil];
	}
	[this->progressBar setDoubleValue:percentage];
	[pool release];
}

void AbsintheMainWnd::handleStartClicked(void* data)
{
	this->setButtonEnabled(0);
	this->setProgress(0);
	[this->progressBar startAnimation:nil];
	this->worker->processStart();
}

bool AbsintheMainWnd::onClose(void* data)
{
	if (this->closeBlocked) {
		return TRUE;
	}
	return FALSE;
}

AbsintheMainWnd::AbsintheMainWnd(int* pargc, char*** pargv)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];
	[NSApp setDelegate:[theAppDelegate new]];

	this->closeBlocked = 0;
	__this = this;
}

void AbsintheMainWnd::run(void)
{
	[NSApp run];
}
