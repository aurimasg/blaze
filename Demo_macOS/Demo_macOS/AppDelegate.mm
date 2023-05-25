
#import "AppDelegate.h"
#import "AppView.h"


@interface AppDelegate ()
@property (strong) IBOutlet NSWindow *window;
@end


@implementation AppDelegate {
    AppView *mView;
}


- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    mView = [[AppView alloc] initWithFrame:self.window.contentView.bounds];

    self.window.contentView = mView;
}


- (IBAction)openBinaryFile:(id)sender {
    NSOpenPanel *op = NSOpenPanel.openPanel;

    op.allowsMultipleSelection = NO;
    op.canChooseDirectories = NO;
    op.canCreateDirectories = NO;
    op.allowedFileTypes = @[ @"vectorimage" ];

    [op runModal];

    NSURL *url = op.URL;

    if (url != nil) {
        NSString *path = url.path;

        [mView loadImageAtPath:path];
    }
}


@end
