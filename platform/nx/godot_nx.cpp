#include "main/main.h"
#include "os_nx.h"

#include <cstdlib>

#include <nn/nn_Log.h>
#include <nn/mem.h>
#include <nn/os.h>
#include <nn/fs.h>
#include <nn/nn_Result.h>

#include <nn/account/account_Api.h>
#include <nn/account/account_ApiForApplications.h>
#include <nn/account/account_Result.h>
#include <nn/account/account_Selector.h>
#include <nn/util/util_ScopeExit.h>


extern "C" void nnMain()
{
    NN_LOG("nnMain\n");
	int argc = nn::os::GetHostArgc();
	char **argv = nn::os::GetHostArgv();

	nn::Result result;
    // Initialize the account library.
    nn::account::Initialize();

    // First try and find a preselected user account
    // Open the preselected user instead of selecting one after launching the game.
    nn::account::UserHandle userHandle;
    nn::account::Uid user;
    if ( nn::account::TryOpenPreselectedUser(&userHandle) ) {
        result = nn::account::GetUserId(&user, userHandle);
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
    } else {
        // Display the user account selection screen and make the user select a user account.
        // nn::account::ResultCancelledByUser is returned if the process is canceled by user operation.
        result = nn::account::ShowUserSelector(&user);
        if( nn::account::ResultCancelledByUser::Includes(result) ) {
            NN_LOG("The user selection was canceled.\n");
            return;
        }
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
        // Open the user selected at application startup.
        // Keep it open for the duration of user operations.
        result = nn::account::OpenUser(&userHandle, user);
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
    }

    NN_UTIL_SCOPE_EXIT
    {
        nn::account::CloseUser(userHandle);
    };

	// Mount save data.
    {
        NN_LOG("Mount save data as \"user\".\n");
        // Create the selected user save data.
        // If data already exists, does nothing and returns nn::ResultSuccess.
        result = nn::fs::EnsureSaveData(user);
        if ( nn::fs::ResultUsableSpaceNotEnough::Includes(result) )
        {
            // Error handling when the application does not have enough memory is required for the nn::fs::EnsureSaveData() function.
            // The system automatically displays a message indicating that there was not enough capacity to create save data in the error viewer.
            // The application must offer options to cancel account selection and to return to the prior scene.
            NN_ABORT("Usable space not enough.\n");
        }
        // No further error handling is necessary because
        // the process is aborted in the library if it fails for any other reason.
        // Mount the save data as "save."
        result = nn::fs::MountSaveData("user", user);
        // Always abort when a failure occurs.
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
    }

    OS_NX os;
    Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK) {
        NN_LOG("nnMain failed to setup Main\n");
		return;
    }
	
	if (Main::start() == EXIT_SUCCESS)
		os.run(); // it is actually the OS that decides how to run
	Main::cleanup();

	// Unmount.
    NN_LOG("Unmount \"user\".\n\n");
    nn::fs::Unmount("user");

    NN_LOG("nnMain end\n");
}