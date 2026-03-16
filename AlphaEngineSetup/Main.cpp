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

    // Replace 344 with the actual number from your VS Output window
    // !!!!!!!!!!!!!!!!!!!! Uncommment this line to find memory leaks or code errors !!!!!!!!!!!!!!!!!!!!
    /*_CrtSetBreakAlloc(3583);*/ 

    // Tell the compiler to ignore these variables to avoid unused parameter warnings
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    {
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

        // Manually unload the current level data before the app closes
        GSM_Unload();

        // Safely shut down the engine and clean up system resources
        AESysExit();
    }
    return 0;
}