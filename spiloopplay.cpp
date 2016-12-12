////////////////////////////////////////////////////////////////
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
//
//
//2014july06, creation for loopplaying a stereo sample (wav file) 
//            using portaudio
//
//nakedsoftware.org, spi@oifii.org or stephane.poirier@oifii.org
////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "portaudio.h"
#include "pa_asio.h"
#include "spiwavsetlib.h"

#define STRICT 1 
#include <windows.h>
#include <iostream>

using namespace std;

#include <map>
#include <string>
#include <assert.h>

//Select sample format. 
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

bool global_bwavfoldermode=false;
Instrument* global_pInstrument=NULL;
WavSet* pWavSet = NULL;

UINT global_TimerId=0;

//The event signaled when the app should be terminated.
HANDLE g_hTerminateEvent = NULL;
//Handles events that would normally terminate a console application. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
int Terminate();
PaStream* global_pPaStream;
WavSet* global_pWavSet = NULL;

map<string,int> global_devicemap;

HHOOK   g_kb_hook = 0;
bool global_bmute=false;

LRESULT CALLBACK kb_proc(int code, WPARAM w, LPARAM l)
{
	HWND myHWNDactive=GetForegroundWindow(); //GetActiveWindow();
	HWND myHWNDconsole=GetConsoleWindow();
	//printf("active %d, console %d \n", myHWNDactive, myHWNDconsole);
	if(myHWNDactive==myHWNDconsole)
	{
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)l;
        const char *info = NULL;
        if (w == WM_KEYDOWN)
		{
            info = "key dn";
			if(p->vkCode==77) //m key
			{
				if(global_bmute) global_bmute=false;
				  else global_bmute=true;
			}
		}
		else if (w == WM_KEYUP)
            info = "key up";
        else if (w == WM_SYSKEYDOWN)
            info = "sys key dn";
        else if (w == WM_SYSKEYUP)
            info = "sys key up";
        //printf ("%s - vkCode [%04x], scanCode [%04x]\n", info, p->vkCode, p->scanCode);
	}
    // always call next hook
    return CallNextHookEx(g_kb_hook, code, w, l);
};


VOID CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime) 
{
	cout << "Time: " << dwTime << '\n';
	cout.flush();
}


//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
    PaStreamParameters outputParameters;
    //PaStream* stream;
    PaError err;
	//WavSet* pWavSet = NULL;

	///////////////////
	//read in arguments
	///////////////////
	//char charBuffer[2048] = {"testbeat.wav"}; //usage: spiplay testbeat2.wav 10
	char charBuffer[2048] = {"conga-kit_a1_tum_ot_l14.wav"}; //usage: spiloopplay conga-kit_a1_tum_ot_l14.wav 100 0.5
	//char charBuffer[2048] = {"JAVADRUM 3 (Wave 1).WAV"};
	//float fSecondsPlay = 30; //positive for number of seconds to play/loop
	float fSecondsPlay = -1.0; //negative for playing only once
	double fSecondsPerLoop = 0.5; 
	if(argc>1)
	{
		//first argument is the filename
		sprintf_s(charBuffer,2048-1,argv[1]);
	}
	if(argc>2)
	{
		//second argument is the time it will play
		fSecondsPlay = atof(argv[2]);
	}
	if(argc>3)
	{
		//third argument is the segment length in seconds
		fSecondsPerLoop = atof(argv[3]);
	}
	//use audio_spi\spidevicesselect.exe to find the name of your devices, only exact name will be matched (name as detected by spidevicesselect.exe)  
	//string audiodevicename="E-MU ASIO"; //"Speakers (2- E-MU E-DSP Audio Processor (WDM))"
	string audiodevicename="Speakers (2- E-MU E-DSP Audio P"; //"E-MU ASIO"
	if(argc>4)
	{
		audiodevicename = argv[4]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
	}
    int outputAudioChannelSelectors[2]; //int outputChannelSelectors[1];
	/*
	outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	*/
	/*
	outputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	outputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	*/
	outputAudioChannelSelectors[0] = 6; // on emu patchmix ASIO device channel 15 (left)
	outputAudioChannelSelectors[1] = 7; // on emu patchmix ASIO device channel 16 (right)
	if(argc>5)
	{
		outputAudioChannelSelectors[0]=atoi(argv[5]); //0 for first asio channel (left) or 2, 4, 6 and 8 for spi (maxed out at 10 asio output channel)
	}
	if(argc>6)
	{
		outputAudioChannelSelectors[1]=atoi(argv[6]); //1 for second asio channel (right) or 3, 5, 7 and 9 for spi (maxed out at 10 asio output channel)
	}

    //Auto-reset, initially non-signaled event 
    g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    //Add the break handler
    ::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	string mystring(charBuffer);
	size_t found = mystring.rfind(".wav");
	if(found==string::npos)
	{
		global_bwavfoldermode=true;
	}

	if(global_bwavfoldermode==false)
	{
		/////////////////
		//read a WAV file 
		/////////////////
		global_pWavSet = new WavSet;
		global_pWavSet->ReadWavFile(charBuffer);
		if(global_pWavSet->numChannels!=2)
		{
			printf("exiting because sample is mono\n");
			delete global_pWavSet;
			exit(0);
		}
	}
	else
	{
		//////////////////////////
		//read folder of WAV files
		//////////////////////////
		global_pInstrument = new Instrument;
		//global_pInstrument->CreateFromWavFolder(mystring.c_str(), 128);
		global_pInstrument->CreateFromWavFolder(mystring.c_str(), 20);
	}
	
	///////////////////////
	// Initialize portaudio 
	///////////////////////
    err = Pa_Initialize();
    if( err != paNoError ) 
	{
		//goto error;
		Pa_Terminate();
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		return Terminate();
	}

	////////////////////////
	//audio device selection
	////////////////////////
	const PaDeviceInfo* deviceInfo;
    int numDevices = Pa_GetDeviceCount();
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pa_GetDeviceInfo( i );
		string devicenamestring = deviceInfo->name;
		global_devicemap.insert(pair<string,int>(devicenamestring,i));
	}

	int deviceid = Pa_GetDefaultOutputDevice(); // default output device 
	map<string,int>::iterator it;
	it = global_devicemap.find(audiodevicename);
	if(it!=global_devicemap.end())
	{
		deviceid = (*it).second;
		printf("%s maps to %d\n", audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//deviceInfo->maxInputChannels
		assert(outputAudioChannelSelectors[0]<deviceInfo->maxOutputChannels);
		assert(outputAudioChannelSelectors[1]<deviceInfo->maxOutputChannels);
	}
	else
	{
		for(it=global_devicemap.begin(); it!=global_devicemap.end(); it++)
		{
			printf("%s maps to %d\n", (*it).first.c_str(), (*it).second);
		}
		//Pa_Terminate();
		//return -1;
		printf("error, audio device not found, will use default\n");
		deviceid = Pa_GetDefaultOutputDevice();
	}


	//outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device 
	outputParameters.device = deviceid; 
	if (outputParameters.device == paNoDevice) 
	{
		fprintf(stderr,"Error: No default output device.\n");
		//goto error;
		Pa_Terminate();
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		return Terminate();
	}
	//outputParameters.channelCount = global_pWavSet->numChannels;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	//outputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	PaAsioStreamInfo asioOutputInfo;
	asioOutputInfo.size = sizeof(PaAsioStreamInfo);
	asioOutputInfo.hostApiType = paASIO;
	asioOutputInfo.version = 1;
	asioOutputInfo.flags = paAsioUseChannelSelectors;
	asioOutputInfo.channelSelectors = outputAudioChannelSelectors;
	//outputChannelSelectors[0] = 0; // ASIO device channel 1 (left)
	//outputChannelSelectors[1] = 1; // ASIO device channel 2 (right)
	if(deviceid==Pa_GetDefaultOutputDevice())
	{
		outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO) 
	{
		outputParameters.hostApiSpecificStreamInfo = &asioOutputInfo;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS) 
	{
		/*
		//Use an WDMKS specific structure. WARNING - this is not portable. 
		PaWDMKSStreamInfo wdmksOutputInfo;
		asioOutputInfo.size = sizeof(PaWDMKSStreamInfo);
		asioOutputInfo.hostApiType = paWDMKS;
		asioOutputInfo.version = 1;
		asioOutputInfo.flags = paAsioUseChannelSelectors;
		//outputChannelSelectors[0] = 0; // ASIO device channel 1 (left)
		//outputChannelSelectors[1] = 1; // ASIO device channel 2 (right)
		outputParameters.hostApiSpecificStreamInfo = &wdmksOutputInfo;
		*/
		outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		outputParameters.hostApiSpecificStreamInfo = NULL;
	}


	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));


	int Counter=0;
	MSG Msg;
	global_TimerId = SetTimer(NULL, 0, fSecondsPerLoop*1000, &TimerProc);
	cout << "TimerId: " << global_TimerId << '\n';
	if (!global_TimerId)
		return 16;

    g_kb_hook = SetWindowsHookEx(WH_KEYBOARD_LL, &kb_proc,
                                GetModuleHandle (NULL), // cannot be NULL, otherwise it will fail
                                0);
    if (g_kb_hook == NULL)
    {
        fprintf (stderr, "SetWindowsHookEx failed with error %d\n", ::GetLastError ());
        return 0;
    };

	while (GetMessage(&Msg, NULL, 0, 0)) 
	{
		++Counter;
		if (Msg.message == WM_TIMER)
		{
			cout << charBuffer << '\n';
			cout << "Counter: " << Counter << "; timer message\n";
			if(!global_bmute)
			{
				if(global_bwavfoldermode==false)
				{
					global_pWavSet->Play(&outputParameters,fSecondsPerLoop);
				}
				else
				{
					pWavSet = global_pInstrument->GetWavSetRandomly();
					if(pWavSet) pWavSet->Play(&outputParameters,fSecondsPerLoop);
				}
			}
		}
		else
		{
			cout << "Counter: " << Counter << "; message: " << Msg.message << '\n';
		}
		DispatchMessage(&Msg);
	}
	//KillTimer(NULL, global_TimerId);
	Terminate();
	return 0;
}

int Terminate()
{
	/*
	//terminate all playing streams
	global_stopallstreams=true;
	Sleep(1000); //would have to wait at least fSecondsPerSegment
	*/

	//kill timer
	KillTimer(NULL, global_TimerId);

	//terminate the only playing stream
    PaError err = Pa_StopStream( global_pPaStream );
    if( err != paNoError ) goto error;


	if( global_pPaStream )
	{
		err = Pa_CloseStream( global_pPaStream );
		if( err != paNoError ) goto error;
		printf("Done.\n"); fflush(stdout);
	}
	Pa_Terminate();
	if(global_pWavSet) delete global_pWavSet;
	if(global_pInstrument) delete global_pInstrument;
	printf("Exiting!\n"); fflush(stdout);

	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
	return 0;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return -1;
}

//Called by the operating system in a separate thread to handle an app-terminating event. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        // CTRL_C_EVENT - Ctrl+C was pressed 
        // CTRL_BREAK_EVENT - Ctrl+Break was pressed 
        // CTRL_CLOSE_EVENT - Console window was closed 
		Terminate();
        // Tell the main thread to exit the app 
        ::SetEvent(g_hTerminateEvent);
        return TRUE;
    }

    //Not an event handled by this function.
    //The only events that should be able to
	//reach this line of code are events that
    //should only be sent to services. 
    return FALSE;
}
