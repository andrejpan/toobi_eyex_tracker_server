# toobi_eyex_tracker_server
Getting minimal fixation data from the Tobii EyeX Controller and sending through sockets, windows side

## Windows setup
1. You need to have [Tobii EyeX Controller](http://www.tobii.com/en/eye-experience/buy/).
2. Download the [Tobii EyeX Engine](http://developer.tobii.com/eyex-setup/) and install the software.
3. Fallow the instalation guide, connect the controler and make initial calibration.
4. Install Free Visual Studio: [Express for Desktop](https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx). Professional version should also work.
5. Download [Tobii EyeX SDK for C/C++](http://developer.tobii.com/downloads/) together with samples and extract it. You should register an account and accept their License Agreement.
6. The easiest way to build the project is to copy [this code](https://github.com/andrejpan/toobi_eyex_tracker_server/blob/master/MinimalFixationDataStreamServer.c) and paste it to /samples/MinimalFixationDataStream/MinimalFixationDataStream.c. Open solution, build the project and run it. 

Make sure that you running **MinimalFixationDataStream** project and not some other project inside Visual Studio Solution. Run the project and fixation data is going to be printed in console. Server will start sending data when the first client will connect to it.

## TODO 
- fix prints at console
- fix while true loops in thread

## VirtualBox 5.X.Y tips

With fresh VirtualBox instalation you could have problems with usb permissions and not seeing Controler inside Windows OS. Do the [Step 5](http://www.dedoimedo.com/computers/virtualbox-usb.html): Fix the group permissions, log out, log in and  Tobii EyeX Controller should be now seen inside Windows. 

If you are using VirtualBox and NAT settings for network you can not use this source for a server, because you will not see server behind NAT. You must change network configuration to Bridge adapter.

VirtualBox and Tobii EyeX Engine has the same **primary key**(Right Ctrl) on Linux OS as host. EyeX Button inside Tobii EyeX Engine is not going to work with default settings. 
