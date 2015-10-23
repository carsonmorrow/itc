//
// File:       iTunesPlugIn.cpp
//
// Abstract:   Visual plug-in for iTunes.  Cross-platform code.
//
// Version:    2.0
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ( "Apple" )
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright © 2001-2011 Apple Inc. All Rights Reserved.
//

//-------------------------------------------------------------------------------------------------
//	includes
//-------------------------------------------------------------------------------------------------

#include "iTunesPlugIn.h"

BOOL APIENTRY DllMain( HMODULE, DWORD, LPVOID )
{
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
//	VisualPluginHandler
//-------------------------------------------------------------------------------------------------
//
static OSStatus VisualPluginHandler(OSType message,VisualPluginMessageInfo *,void *)
{
	OSStatus			status;
	
	status = noErr;

	switch ( message )
	{
		case kVisualPluginInitMessage:
		case kVisualPluginCleanupMessage:
		case kVisualPluginEnableMessage:
		case kVisualPluginDisableMessage:
		case kVisualPluginIdleMessage:
		case kVisualPluginActivateMessage:
		case kVisualPluginDeactivateMessage:
		case kVisualPluginWindowChangedMessage:
		case kVisualPluginFrameChangedMessage:
		case kVisualPluginPulseMessage:
		case kVisualPluginDrawMessage:
		case kVisualPluginPlayMessage:
		case kVisualPluginChangeTrackMessage:
		case kVisualPluginCoverArtMessage:
		case kVisualPluginStopMessage:
		case kVisualPluginSetPositionMessage:
		{
			break;
		}
		default:
		{
			status = unimpErr;
			break;
		}
	}

	return status;	
}

//-------------------------------------------------------------------------------------------------
//	RegisterVisualPlugin
//-------------------------------------------------------------------------------------------------
//
OSStatus RegisterVisualPlugin( PluginMessageInfo * messageInfo )
{
	PlayerMessageInfo	playerMessageInfo;
	OSStatus			status;
		
	memset( &playerMessageInfo.u.registerVisualPluginMessage, 0, sizeof(playerMessageInfo.u.registerVisualPluginMessage) );

	// copy in name length byte first
	playerMessageInfo.u.registerVisualPluginMessage.name[0] = (UInt8)strlen("iTunesControl Plugin");
	// now copy in actual name
	memcpy(&playerMessageInfo.u.registerVisualPluginMessage.name[1], "iTunesControl Plugin", strlen("iTunesControl Plugin"));

	SetNumVersion( &playerMessageInfo.u.registerVisualPluginMessage.pluginVersion, MAJORNUMBER, MINORNUMBER, BUILDNUMBER, 0 );

	playerMessageInfo.u.registerVisualPluginMessage.options					= 0;
	playerMessageInfo.u.registerVisualPluginMessage.handler					= (VisualPluginProcPtr)VisualPluginHandler;
	playerMessageInfo.u.registerVisualPluginMessage.registerRefCon			= 0;
	playerMessageInfo.u.registerVisualPluginMessage.creator					= 0;
	
	playerMessageInfo.u.registerVisualPluginMessage.pulseRateInHz			= 1;	// update my state N times a second
	playerMessageInfo.u.registerVisualPluginMessage.numWaveformChannels		= 0;
	playerMessageInfo.u.registerVisualPluginMessage.numSpectrumChannels		= 0;
	
	playerMessageInfo.u.registerVisualPluginMessage.minWidth				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.minHeight				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.maxWidth				= 0;	// no max width limit
	playerMessageInfo.u.registerVisualPluginMessage.maxHeight				= 0;	// no max height limit
	
	status = PlayerRegisterVisualPlugin( messageInfo->u.initMessage.appCookie, messageInfo->u.initMessage.appProc, &playerMessageInfo );

	// itc glue
	HWND hwndITC = FindWindowEx( HWND_MESSAGE, NULL, NULL, L"iTunesControl" );
	if ( NULL != hwndITC )
	{
		// Found it!
		SendMessage( hwndITC, WM_APP + 9, NULL, NULL );
	}
	else
	{
		// iTC isn't running, we'll have to load settings
		SettingsStruct ssSettings;
		memset( &ssSettings, 0, sizeof(SettingsStruct) );
		LoadSettings( NULL, &ssSettings );

		if ( ssSettings.bStartWithiTunes )
		{
			// Start iTC
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			TCHAR tcCmdline[MAX_PATH] = {0};
			TCHAR tcPath[MAX_PATH] = {0};
			TCHAR tcWorkingPath[MAX_PATH] = {0};
			TCHAR tcProductCode[39] = {0};
			DWORD dwPath = MAX_PATH;

			// Get product code based on itc.exe component code
			UINT uiRet = MsiGetProductCode( L"{0C6811A4-7D4F-4B79-BD58-71219A573056}", tcProductCode );
			if ( ERROR_SUCCESS == uiRet )
			{
				uiRet = MsiGetProductInfo( tcProductCode, INSTALLPROPERTY_INSTALLLOCATION, tcPath, &dwPath );

				if ( ERROR_SUCCESS == uiRet )
				{
					_tcscpy_s( tcWorkingPath, MAX_PATH, tcPath );
					PathAppend( tcPath, TEXT("itc.exe") );
					_tcscpy_s( tcCmdline, MAX_PATH, tcPath );
					_tcscat_s( tcCmdline, MAX_PATH, L" -itunesstart" );
					CreateProcess(tcPath, tcCmdline, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, tcWorkingPath, &si, &pi);

					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
			}
		}
	}

	return status;
}

ITC_PLUGIN_API OSStatus iTunesPluginMain( OSType message, PluginMessageInfo *messageInfo, void * )
{
	OSStatus status;

	switch( message )
	{
	case kPluginInitMessage:
		status = RegisterVisualPlugin( messageInfo );
		break;
	case kPluginCleanupMessage:
		status = noErr;
		break;
	default:
		status = unimpErr;
		break;
	}

	return status;
}
