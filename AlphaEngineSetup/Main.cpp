#include <iostream>
#include <crtdbg.h> 

#include "Utils.h"
#include "GameStateManager.h"

// The entry point for the Windows application
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    // Enable memory leak detection to report any unfreed memory in the Output window on exit
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Tell the compiler to ignore these variables to avoid unused parameter warnings
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize the engine system with the desired resolution and refresh rate
    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);

    // Set the window title for the application
    AESysSetWindowTitle("Dungeon Generator Test");

    // Start the game in the state you want
    GSM_Initialize(GS_MAINMENU);

    // Continue the loop until the game state manager signals a quit
    while (gGameStateCurr != GS_QUIT)
    {
        // Prepare the engine to process the next frame of logic and input
        AESysFrameStart();

        // Update the state manager which handles all loading, logic, and drawing
        GSM_Update();

        // Finish the frame by swapping the buffers and displaying the image
        AESysFrameEnd();
    }

    // Safely shut down the engine and clean up system resources
    AESysExit();

    return 0;
}

// ---------------------------------------------------------------------------
// includes
//#include <iostream>
//#include <crtdbg.h> // To check for memory leaks
//
//#include "Utils.h"
//#include "Level.h"
//#include "RoomGenerator.h"
//#include "Room.h"
//#include "GameStateManager.h"
//
//// ---------------------------------------------------------------------------
//// main
//
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPWSTR    lpCmdLine,
//	_In_ int       nCmdShow)
//{
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//
//	UNREFERENCED_PARAMETER(hPrevInstance);
//	UNREFERENCED_PARAMETER(lpCmdLine);
//
//	// Initialization of your own variables go here
//
//	// Using custom window procedure
//	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);
//
//	// Changing the window title
//	AESysSetWindowTitle("test");
//
//	// Initialize GSM
//	GSM_Initialize(GS_MAINMENU);
//
//	// GAME LOOP
//	while (gGameStateCurr != GS_QUIT)
//	{
//		AESysFrameStart();
//
//		GSM_Update(); // Does all the work
//
//		AESysFrameEnd();
//	}
//
//	// Free Engine
//	AESysExit();
//	return 1;
//}