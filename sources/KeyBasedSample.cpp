#include "KeyBasedSample.h"
#include <cassert>
#include <iostream>

using namespace LicenseSpring;

//First time online license installation 
void KeyBasedSample::runOnline( bool deactivateAndRemove )
{
    auto license = m_licenseManager->getCurrentLicense();
    if(license){
        std::cout <<"\nError - License is already installed.";
        //return;
    }

    auto licenseId = LicenseID::fromKey("HAGJ-ET4H-8CJJ-RKBS" );
    if(licenseId.isEmpty())
    {
        std::cout <<"\nError - Invalid License Key supplied.";
        return;        
    }

    license = m_licenseManager->activateLicense( licenseId );
    
    //license->addDeviceVariable("DV0", "value0", false);
    //license->addDeviceVariable("DV1", "value1", false);
    std::cout << "SUCCESS - License activated successfully.." << std::endl;
    //AY - update license is disabled for current poc installation
    updateAndCheckLicense( license );

    PrintLicense( license );
    printUpdateInfo();

    // if( deactivateAndRemove )
    //     cleanUp( license );
}

//AY - Below method is redundant for Presien case
// Keeping the code to refer in future to SDK source.
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

            auto licenseId = LicenseID::fromKey( "HAGJ-ET4H-8CJJ-RKBS" ); // Assign some license key here
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
