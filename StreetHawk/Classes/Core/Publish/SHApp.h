/*
 * Copyright (c) StreetHawk, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#import <UIKit/UIKit.h>
#import "SHTypes.h" //for enum SHDevelopmentPlatform
#import "PushDataForApplication.h" //for SHResult

@class SHInstall;
@class SHLogger;

/**
 Callback to let customer App to handle deeplinking url.
 */
typedef void (^SHOpenUrlHandler)(NSURL *openUrl);

/**
 Singleton to access SHApp.
 */
#define StreetHawk          [SHApp sharedInstance]

/**
 The SHApp Class is core of whole SDK. It contains almost all the functions.
 
 **Normal usage:**
 
 - SHApp is singleton, access it by `[SHApp sharedInstance]` or `StreetHawk`.
 
 - When your Application starts, usually in `- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions`, call  `registerInstallForApp:withDebugMode:` to initialize all required StreetHawk features.
 
     This is simple, here are a few lines of sample code: 
 
        [StreetHawk registerInstallForApp:appId(registered with the StreetHawk Cloud) withDebugMode:Yes];
 
 - Add tag for user is simple, code like:
 
        [StreetHawk tagString:@"a@a.com" forKey:@"sh_email"];
 
 **Necessary project settings:**
 
 To have your StreetHawk-enabled Application work properly, some project settings are necessary. 
 
 - Add "-ObjC" into "Other Linker Flags". It's necessary for using SDK otherwise link error may occur for category methods.
 - Add resource bundle into project. It's necessary for SDK's xib and image resources.
 - Add "Background fetch" in "Background Modes", since iOS 7.
 - Add "NSLocationAlwaysUsageDescription" or "NSLocationWhenInUseUsageDescription" in Info.plist for enabling location service since iOS 8.
 - Because StreetHawk SDK uses significant location change at background, there is NO need to add "Required background modes" with *location service*.
 
 */
@interface SHApp : NSObject<UIApplicationDelegate>

/** @name Create and initialize */

/**
 Singleton creator of SHApp. Normally use `StreetHawk` to represent `[SHApp sharedInstance]`.
 @return Singleton SHApp instance.
 */
+ (SHApp *)sharedInstance;

/**
 Initialize for an Application, setting up the environment.
 @param appKey The global name of the app. This is registered in StreetHawk server, once registered it cannot change. There are three ways to set `appKey`:
               1. Set `appKey` by this function.
               2. If 1 pass nil or empty, check property `StreetHawk.appKey`.
               3. If 1 and 2 still nill or empty, check InfoPlist `APP_KEY`.
 @param isDebugMode The mode of whether print NSLog in Xcode console.
 */
- (void)registerInstallForApp:(NSString *)appKey withDebugMode:(BOOL)isDebugMode;

/**
 Deprecated, use `- (void)registerInstallForApp:(NSString *)appKey withDebugMode:(BOOL)isDebugMode;` instead. `iTunesId` is setup in web console, or by property `@property (nonatomic, strong) NSString *itunesAppId;`.
 */
- (void)registerInstallForApp:(NSString *)appKey withDebugMode:(BOOL)isDebugMode withiTunesId:(NSString *)iTunesId;

/** @name Global properties */

/**
 The allocated name or code for this app as set in the StreetHawk Cloud, for example "SHSheridan1". It's mandatory for an Application to work. Check `- (void)registerInstallForApp:(NSString *)appKey withDebugMode:(BOOL)isDebugMode` for how it works.
 */
@property (nonatomic, strong) NSString *appKey;

/**
 Only for internal usage. It's "https://api.streethawk.com".
 */
- (void)setDefaultStartingUrl:(NSString *)defaultUrl;

/**
 Decide whether need to show debug log in console.
 */
@property (nonatomic) BOOL isDebugMode;

/**
 The App id after register in iTunes, for example @"337064413". It used for rating and upgrading App, if this id is not setup, rating or upgrading dialog will not promote.
 */
@property (nonatomic, strong) NSString *itunesAppId;

/**
The application version and build version of current Application, formatted as @"[CFBundleShortVersionString] ([CFBundleVersion])", for example @"1.2.7 (10)". This is version for Application project, not for SDK. Use `version` to get StreetHawkCore.framework SDK version.
*/
@property (nonatomic, strong, readonly) NSString *clientVersion;

/**
 Version for StreetHawkCore.framework SDK, formatted as [framework version] (X.Y.Z), for example "1.2.5". In Finder [framework version] is visible by view framework file info.
 */
@property (nonatomic, strong, readonly) NSString *version;

/**
 Before successfully install, it's nil. After install once, it's the install instance.
 */
@property (nonatomic, strong) SHInstall *currentInstall;

/**
 Make sure installs/register or installs/update happen in sequence.
 */
@property (nonatomic, strong) dispatch_semaphore_t install_semaphore;

/**
 An enum for current App's development platform, refer to `SHDevelopmentPlatform` for supporting platforms. This is only used internally, and setup by Phonegap plugin, Titanium module, Xamarin binding etc. Normal customer does not need to change it.
 */
@property (nonatomic, readonly) SHDevelopmentPlatform developmentPlatform;

/**
 StreetHawk requires AppDelegate has some common functions, if `autoIntegrateAppDelegate` is YES (by default), customer App does not need to manually implement any of the push-related UIApplicationDelegate protocol methods or pass notifications to the library. The library is able to do this by setting itself as the app delegate, intercepting messages and forwarding them to your original app delegate. This must be setup before register install. It's YES by default but if custome App set it to NO, customer App must implement these functions manually:
 
 `- (void)application:(UIApplication *)application didRegisterUserNotificationSettings:(UIUserNotificationSettings *)notificationSettings  //since iOS 8.0
 {
    [StreetHawk handleUserNotificationSettings:notificationSettings];
 }
 
 - (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
 {
    [StreetHawk setApnsDeviceToken:deviceToken];
 }
 
 - (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
 {
    [StreetHawk handleRemoteNotification:userInfo];
 }
 
 - (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo fetchCompletionHandler:(void (^)(UIBackgroundFetchResult))completionHandler
 {
    [StreetHawk handleRemoteNotification:userInfo needComplete:YES fetchCompletionHandler:completionHandler];
 }
 
 - (void)application:(UIApplication *)application handleActionWithIdentifier:(NSString *)identifier forRemoteNotification:(NSDictionary *)userInfo completionHandler:(void (^)())completionHandler
 {
    [StreetHawk handleRemoteNotification:userInfo withActionId:identifier needComplete:YES completionHandler:completionHandler];
 }

 - (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
 {
    [StreetHawk handleLocalNotification:notification];
 }
 
 - (void)application:(UIApplication *)application handleActionWithIdentifier:(NSString *)identifier forLocalNotification:(UILocalNotification *)notification completionHandler:(void (^)())completionHandler
 {
    [StreetHawk handleLocalNotification:notification withActionId:identifier needComplete:YES completionHandler:completionHandler];
 }

 - (void)application:(UIApplication *)application performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult))completionHandler
 {
    [StreetHawk shRegularTask:completionHandler];
 }
 
 - (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
 {
    return [StreetHawk openURL:url sourceApplication:sourceApplication annotation:annotation];
 }`
 */
@property (nonatomic) BOOL autoIntegrateAppDelegate;

/** @name Handlers */

/**
 An instance to deal with log.
 */
@property (nonatomic, readonly, weak) SHLogger *logger;

/** @name Global properties and methods */

/**
 Push notification 8004/8006/8007 is to launch a certain view controller, however it's difficult for server to know "how to launch the view controller". In iOS platform it requires the following elements to initialize a view controller:
 
 * view controller class name -- mandatory, must be a class inherit from UIViewController.
 * xib for iPhone -- optional, used for intialize the view controller on iPhone device, and xib name is different from class name; if nil use class name as the xib name.
 * xib for iPad -- optional, used for intialize the view controller on iPad device, and xib name is different from class name; if nil use class name as the xib name.
 
 For Phonegap App, it can be load an html page. Refer to `shPGHtmlReceiver` and `shPGLoadHtml`.
 
 Server may have problem to send push notification, because:
 
 * iOS and Android possibly have different class name, different mechanism to launch view controller. Need to consider the platform details.
 * Applications even running on the same platform can have different ac/vc names in different versions.
 
 To make server simpler in this aspect, client side provide a way to register "friendly name", while locally store the map for vc and xib. For example, client side can register: friendly name = "login", vc = "MyLoginViewController", xib_iphone = "MyLoginViewController_iphone", xib_ipad = "MyLoginViewController_ipad", so server display "login" in web console when sending push notification. After client side receive this notification, find vc and xib locally to initialize view controller.
 
 **Note 1: Friendly names will be visible in StreetHawk web interface and they should be the same across different platforms (ios, android etc.).**
 **Note 2: Use friendly name = "register" (FRIENDLYNAME_REGISTER) for register page, which can be specifically handled by 8006 push notification.
 **Note 3: Use friendly name = "login" (FRIENDLYNAME_LOGIN) for login page, which can be specifically handled by 8007 push notification.
 **Note 4: The friendly name will be submitted when next `app_status` with "submit_views" = true.
 
 @param arrayFriendlyNameObj The array with each object defines a friendly name object.
 @return If the format is correct for register friendly name, return YES; otherwise return NO, and error information can refer to console log output (withDebugMode:YES).
 */
- (BOOL)shCustomActivityList:(NSArray *)arrayFriendlyNameObj;

/**
 API to trigger feedback UI and send feedback. It behaves in this way:
 
 * If define option choice list by `arrayChoice`, a choice list will show first, after user select one of the option: a) If `needInput` is Yes, an input UI with the selected choice is displayed for user to input free text; b) If `needInput` is No user's selected choice is posted to server directly.
 * If `arrayChoice` = nil or empty, input free text UI is displayed for user to type.
 * Choice title is mandatory, free text detail content is optional.
 @param arrayChoice The option choice list. For example, @[@"Product not Available", @"Wrong Address", @"Description mismatch"]. It can be nil.
 @param needInput Whether need to show free text input dialog. If `arrayChoice` is nil or empty always show input dialog regardless of this settings.
 @param needConfirm Whether need to show confirm alert dialog of Cancel/Yes Please!. When App in FG and notification arrive needs to show confirm dialog.
 @param infoTitle The title display on choice list. If nil shows "<App Name> loves Feedback!".
 @param infoMessage The message display on choice list. It can be nil or empty.
 @param pushData When used in notification, pass in payload from server. If not used in notification, pass nil.
 */
- (void)shFeedback:(NSArray *)arrayChoice needInputDialog:(BOOL)needInput needConfirmDialog:(BOOL)needConfirm withTitle:(NSString *)infoTitle withMessage:(NSString *)infoMessage withPushData:(PushDataForApplication *)pushData;

/**
 Submit feedback request to server without UI.
 @param title Feedback title submit to server. 
 @param content Feedback content submit to server. 
 @param handler Request callback handler.
 */
- (void)shSendFeedbackWithTitle:(NSString *)title withContent:(NSString *)content withHandler:(SHCallbackHandler)handler;

/**
 Send enter log (8108) for `page`. For trace view it's recommended to inherit from `StreetHawkViewController` or `StreetHawkBaseViewController`, which automatically call `shNotifyPageEnter` on `viewDidAppear` and `shNotifyPageExit` on `viewDidDisappear`. But for App which cannot do inheritance (for example Phonegap, Titanium and Xamarin), call `shNotifyPageEnter` and `shNotifyPageExit` explictly.
 Note: if history has a page record, it sends exit log (8108) for the history. This is a workaround fix for "forget" add `shNotifyPageExit` on `viewDidDisappear`, and more importantly, some App such as Phonegap cannot call `shNotifyPageExit`.
 @param page Enter page name. It cannot be nil. For UIViewController it's class name such as `self.class.description`; for Phonegap it's html page name such as `index.html`.
 */
- (void)shNotifyPageEnter:(NSString *)page;

/**
 Send exit log (8109) for `page`. For trace view it's recommended to inherit from `StreetHawkViewController` or `StreetHawkBaseViewController`, which automatically call `shNotifyPageEnter` on `viewDidAppear` and `shNotifyPageExit` on `viewDidDisappear`. But for App which cannot do inheritance (for example Phonegap, Titanium and Xamarin), call `shNotifyPageEnter` and `shNotifyPageExit` explictly. 
 @param page Exit page name. It cannot be nil. For UIViewController it's class name such as `self.class.description`; for Phonegap it's html page name such as `index.html`.
 */
- (void)shNotifyPageExit:(NSString *)page;

/**
 Callback to let customer App to handle deeplinking url.
 Launch view controller in such scenarios:
 1. Click a link "<app_scheme://host/path?param=value>" in Email or social App, launch this App by `openURL`.
 2. Notification "Launch page activity" sends from StreetHawk campaign with deeplinking url "<app_scheme://host/path?param=value>", and host not equal "launchvc".
 3. Growth recommend friend to install a new App, after first launch Growth automatically match a deeplinking url "<app_scheme://host/path?param=value>" and launch view controller.
 */
@property (nonatomic, copy) SHOpenUrlHandler openUrlHandler;

/**
 StreetHawk can use `AdvertisingIdentifier` to help trace end-user, however it requires customer's App is capable to use advertising function according to Apple's agreement. If customer's App can get this, pass into StreetHawk.
 */
@property (nonatomic, strong) NSString *advertisingIdentifier;

/** @name Background Regular Task */

/**
 Perform regular task at certain time interval. It leverages `UIApplicationDelegate` function `- (void)application:(UIApplication *)application performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler` to do some tasks at background, and when App in foreground, it calls each time when App become active. Note:
 
 1. Customer App must have Background mode -> fetch enabled to have this work. 
 2. This function is available since iOS 7.0. Previous iOS system cannot support it. 
 3. User App implement this function by calling it in AppDelegate.m if NOT auto-integrate. If `StreetHawk.autoIntegrateAppDelegate = YES;` make sure NOT call this otherwise cause dead loop. Code snippet:
    `- (void)application:(UIApplication *)application performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler`
    `{`
        `[StreetHawk shRegularTask:completionHandler needComplete:YES];`
    `}`
 
 This function perform following tasks:
 1. If user's location service is enabled, time interval one hour, send non-priority log for current user location (code=19).
 2. Sends priority heartbeat log in 6 hours(code=8051).
 */
- (void)shRegularTask:(void (^)(UIBackgroundFetchResult result))completionHandler needComplete:(BOOL)needComplete NS_AVAILABLE_IOS(7_0);

/** @name Open Url Scheme */

/**
 Handle open URL, customer's App must register "URL Types" in Info.plist with its own scheme. User App implement this function by calling it in AppDelegate.m if NOT auto-integrate. If `StreetHawk.autoIntegrateAppDelegate = YES;` make sure NOT call this otherwise cause dead loop. Code snippet:
    `- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation`
    `{`
        `return [StreetHawk openURL:url sourceApplication:sourceApplication annotation:annotation];`
    `}`
 
 This function performs following tasks:
 1. Deeplinking: open a view and call function of the view. The url must be formatted as: <url scheme>://launchvc?vc=<friendly name or vc>&xib_iphone=<xib_iphone>&xib_ipad=<xib_ipad>&<additional params>. "<url scheme>" must same as Info.plist register URL type; "launchvc" is pre-defined command, case insensitive; "vc" is friendly name or UIViewController's class name, mandatory; others are optional.
 */
- (BOOL)openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation;

/** @name Permission */

/**
 Show this App's preference settings page. Only available since iOS 8. In previous iOS nothing happen.
 @return YES if can show preference page since iOS 8; NO if called in previous iOS and nothing happen.
 */
- (BOOL)launchSystemPreferenceSettings NS_AVAILABLE_IOS(8_0);

@end

/**
 **Extension for logs:**
 
 * The Profile for this install will be tagged with the given values. You can use these tags to run campaigns.
 
 [StreetHawk tagString:@"a@a.com" forKey:@"sh_email"];
 
 * Remove a tagged profile if not need it anymore.
 
 [StreetHawk removeTag:@"sh_email"];
 */
@interface SHApp (LoggerExt)

/**
 Send log with code=8999. It's used for tagging a string value for user. For example, you can tag user's email as by:
 
 `[StreetHawk tagString:@"a@a.com" forKey:@"sh_email"];`
 
 This will send log comment as {"key": "sh_email", "string": @"a@a.com"}.
 
 @param value The value for tag to the user profile. Cannot be empty. It can be NSString, or NSDictionary, or NSArray. 
 @param key The key for tag to the user profile. Cannot be empty.
 @return If tag to server return YES; if fail to send to server return NO.
 */
- (BOOL)tagString:(NSObject *)value forKey:(NSString *)key;

/**
 Send log with code=8999. It's used for tagging a number value for user. For example, you can tag user's favourite product count by:
 
 `[StreetHawk tagNumeric:8 forKey:@"fave_product"];`
 
 This will send log comment as {"key": "fave_product", "numeric": [NSNumber numberWithDouble:8]}.
 
 @param value The number value for tag to the user profile.
 @param key The key for tag to the user profile. Cannot be empty.
 @return If tag to server return YES; if fail to send to server return NO.
 */
- (BOOL)tagNumeric:(double)value forKey:(NSString *)key;

/**
 Send log with code=8999. It's used for tagging a date value for user. For example, you can tag user's visit time by:
 
 `[StreetHawk tagDatetime:[NSDate date] forKey:@"visit_time"];`
 
 This will send log comment as {"key": "visit_time", "datetime": [NSDate date]}.
 
 @param value The date value for tag to the user profile. Cannot be empty.
 @param key The key for tag to the user profile. Cannot be empty.
 @return If tag to server return YES; if fail to send to server return NO.
 */
- (BOOL)tagDatetime:(NSDate *)value forKey:(NSString *)key;

/**
 This is opposite function of `tagString` or `tagNumeric` or `tagDatetime`. It's to remove a user tag by the key, for example `tagDatetime` adds {"key": "sh_date_of_birth", "datetime": "2012-12-12 11:11:11"}, so this `removeUserTag` can remove the tag by key = "sh_date_of_birth". It send log with code=8998, comment = "{key : "sh_date_of_birth"}".
 @param key Key for existing tag. Cannot be empty.
 @return If tag to server return YES; if fail to send to server return NO.
 */
- (BOOL)removeTag:(NSString *)key;

/**
 Send log with code=8997, comment={"key": "<key>", "numeric": 1}.
 @param key Key for existing tag. Cannot be empty.
 @return If tag to server return YES; if fail to send to server return NO.
 */
- (BOOL)incrementTag:(NSString *)key;

@end
