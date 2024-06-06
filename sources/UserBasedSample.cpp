#include "UserBasedSample.h"
#include <iostream>

using namespace LicenseSpring;

void UserBasedSample::runOnline( bool deactivateAndRemove )
{
    auto license = m_licenseManager->getCurrentLicense();

    if( license == nullptr )
    {
        const std::string userId = "";
        const std::string userPassword = "";
        auto licenseId = LicenseID::fromUser( userId, userPassword );
        if( userPassword.empty() )
        {
            // After next call you will get invitation email where you can set up password for new license user
            licenseId = m_licenseManager->getTrialLicense( userId );
            std::cout << "Trial license created for: " << licenseId.id() << std::endl;
        }

        license = m_licenseManager->activateLicense( licenseId );
        std::cout << "License activated successfully" << std::endl;
    }
    else
        checkLicenseLocal( license );

    if( license->isFloating() )
        setupAutomaticLicenseUpdates( license );
    else
        updateAndCheckLicense( license );

    PrintLicense( license );

    printUpdateInfo();

    if( deactivateAndRemove )
        cleanUp( license );
}

void UserBasedSample::runOffline( bool deactivateAndRemove )
{
    auto license = m_licenseManager->getCurrentLicense();

    if( license == nullptr )
    {
        // You can provide here path to offline activation response file,
        // or use default desktop loaction and file name ls_activation.lic
        license = m_licenseManager->activateLicenseOffline();
        if( license == nullptr )
        {
            std::cout << "Offline activation error\n";

            const std::string userId = "someone@test.com";
            const std::string userPassword = "";

            createOfflineActivationRequest( LicenseID::fromUser( userId, userPassword ) );
            return;
        }
        else
            std::cout << "Offline activation succeeded\n";
    }

    checkLicenseLocal( license );

    updateOfflineLicense( license );

    PrintLicense( license );

    if( license->type() == LicenseTypeConsumption )
        license->updateConsumption();

    if( deactivateAndRemove )
        cleanUpLocal( license );
}

