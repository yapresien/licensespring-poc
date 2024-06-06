#include "KeyBasedSample.h"
#include <iostream>

using namespace LicenseSpring;

void KeyBasedSample::runOnline( bool deactivateAndRemove )
{
    auto license = m_licenseManager->getCurrentLicense();

    if( license == nullptr )
    {
        // Assign some license key here or leave it empty to test trial key
        auto licenseId = LicenseID::fromKey("HA74-LJ7D-NT6J-RUBS" );
        if( licenseId.isEmpty() )
        {
            licenseId = m_licenseManager->getTrialLicense();
            std::cout << "Got trial license: " << licenseId.id() << std::endl;
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

void KeyBasedSample::runOffline( bool deactivateAndRemove )
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

            auto licenseId = LicenseID::fromKey( "" ); // Assign some license key here
            if( licenseId.isEmpty() )
            {
                licenseId = m_licenseManager->getTrialLicense();
                std::cout << "Got trial license: " << licenseId.id() << std::endl;
            }

            createOfflineActivationRequest( licenseId );
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
