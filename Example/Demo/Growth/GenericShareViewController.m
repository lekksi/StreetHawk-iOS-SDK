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

#import "GenericShareViewController.h"
#import <MessageUI/MessageUI.h>

@interface GenericShareViewController () <MFMailComposeViewControllerDelegate>

@property (strong, nonatomic) IBOutlet UITableViewCell *cellID;
@property (strong, nonatomic) IBOutlet UITextField *textboxID;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellSource;
@property (retain, nonatomic) IBOutlet UITextField *textboxSource;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellMedium;
@property (retain, nonatomic) IBOutlet UITextField *textboxMedium;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellContent;
@property (retain, nonatomic) IBOutlet UITextField *textboxContent;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellTerm;
@property (retain, nonatomic) IBOutlet UITextField *textboxTerm;
@property (strong, nonatomic) IBOutlet UITableViewCell *cellUrl;
@property (strong, nonatomic) IBOutlet UITextField *textboxUrl;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellDestinationUrl;
@property (retain, nonatomic) IBOutlet UITextField *textboxDestinationUrl;
@property (strong, nonatomic) IBOutlet UITableViewCell *cellEmailSubject;
@property (strong, nonatomic) IBOutlet UITextField *textboxEmailSubject;
@property (strong, nonatomic) IBOutlet UITableViewCell *cellEmailBody;
@property (strong, nonatomic) IBOutlet UITextField *textboxEmailBody;
@property (retain, nonatomic) IBOutlet UITableViewCell *cellShare;

- (IBAction)buttonShareClicked:(id)sender;

@property (nonatomic, strong) NSArray *arrayCells;

@end

@implementation GenericShareViewController

#pragma mark - life cycle

- (void)viewDidLoad
{
    self.arrayCells = @[self.cellID, self.cellSource, self.cellMedium, self.cellContent, self.cellTerm, self.cellUrl, self.cellDestinationUrl, self.cellEmailSubject, self.cellEmailBody, self.cellShare];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return self.arrayCells.count;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = self.arrayCells[indexPath.row];
    return cell.bounds.size.height;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return self.arrayCells[indexPath.row];;
}

#pragma mark - UITextFieldDelegate handler

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return YES;
}

#pragma mark - event handler

- (IBAction)buttonShareClicked:(id)sender
{
    NSURL *deeplinkingUrl = nil;
    if (self.textboxUrl.text.length > 0)
    {
        deeplinkingUrl = [NSURL URLWithString:self.textboxUrl.text];
        if (deeplinkingUrl == nil)
        {
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Deeplinking url format is invalid. Correct it or delete it." message:nil delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
            [alertView show];
            return;
        }
    }
    NSURL *destinationUrl = nil;
    if (self.textboxDestinationUrl.text.length > 0)
    {
        destinationUrl = [NSURL URLWithString:self.textboxDestinationUrl.text];
        if (destinationUrl == nil)
        {
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Destination url format is invalid. Correct it or delete it." message:nil delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
            [alertView show];
            return;
        }
    }
    [StreetHawk originateShareWithCampaign:self.textboxID.text withSource:self.textboxSource.text withMedium:self.textboxMedium.text withContent:self.textboxContent.text withTerm:self.textboxTerm.text shareUrl:deeplinkingUrl withDefaultUrl:destinationUrl streetHawkGrowth_object:^(NSObject *result, NSError *error)
    {
        dispatch_async(dispatch_get_main_queue(), ^
           {
               if (error == nil)
               {
                   NSString *shareUrl = (NSString *)result;
                   if ([MFMailComposeViewController canSendMail])
                   {
                       MFMailComposeViewController *mc = [[MFMailComposeViewController alloc] init];
                       mc.mailComposeDelegate = self;
                       [mc setMessageBody:[NSString stringWithFormat:@"%@\n\n%@", self.textboxEmailBody.text, shareUrl] isHTML:NO];
                       [mc setSubject:self.textboxEmailSubject.text];
                       [self presentViewController:mc animated:YES completion:nil];
                   }
                   else
                   {
                       UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"share_guid_url" message:shareUrl delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
                       [alert show];
                   }
               }
               else
               {
                   UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Error" message:error.localizedDescription delegate:nil cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
                   [alertView show];
               }
           });
    }];
}

- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error
{
    if (!error)
    {
        [controller dismissViewControllerAnimated:YES completion:nil];
    }
}

@end
